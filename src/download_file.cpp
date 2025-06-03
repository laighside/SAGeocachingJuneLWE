/**
  @file    download_file.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This is used for downloading files from the file manager on the website
  All file download requests are rewritten to this script by a ModRewrite rule in the htaccess file
  eg. any request starting with /files/ is ends up here

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "core/CgiEnvironment.h"
#include "core/JlweCore.h"
#include "core/JlweUtils.h"
#include "core/KeyValueParser.h"
#include "public_upload/ImageUtils.h"

int main () {
    try {
        JlweCore jlwe;
        std::string page_request = CgiEnvironment::getRequestUri();

        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);
        bool download_request = urlQueries.getValue("dl") == "true";

        // remove any url arguments (like fbclid)
        if (page_request.find("?") != std::string::npos)
            page_request = page_request.substr(0, page_request.find("?"));

        // check the prefix of the path, the htaccess file should only allow requests with the right prefix to call this script
        size_t prefix_length = std::string(jlwe.config.at("files").at("urlPrefix")).size();
        if (page_request.substr(0, prefix_length) != std::string(jlwe.config.at("files").at("urlPrefix"))) {
            std::cout << "Status:400 Bad Request\r\n";
            std::cout << "Content-type:text/plain\r\n\r\n";
            std::cout << "Bad request.\n";
            return 0;
        }

        std::string filename = page_request.substr(prefix_length);

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        std::string public_upload_dir = jlwe.config.at("publicFileUpload").at("directory");
        std::string base_file_dir = jlwe.config.at("files").at("directory");
        bool includePublicUploads = (base_file_dir.size() < public_upload_dir.size()) && (public_upload_dir.substr(0, base_file_dir.size()) == base_file_dir);

        std::string full_filename = "";
        std::string mysql_filename = "";

        bool validFile = false;
        bool validFilename = false;

        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT filename,public FROM files WHERE CONCAT(directory,filename) = ?;");
        prep_stmt->setString(1, filename);
        res = prep_stmt->executeQuery();
        if (res->next()) { // if the file exists in MySQL
            if (res->getInt(2) != 0 || jlwe.isLoggedIn()) { // if public file or user is logged in
                mysql_filename = res->getString(1);

                // check if it is a file not a directory
                if (mysql_filename.at(mysql_filename.size() - 1) != '/') {

                    // this should be safe from injection attacks since filename is confirmed to exist in the database
                    full_filename = base_file_dir + filename;
                    validFilename = true;

                }
            }
        }
        delete res;
        delete prep_stmt;

        // Check if it's in public upload folder
        if ((!validFilename) && includePublicUploads && jlwe.isLoggedIn()) {
            size_t idx = filename.find_last_of('/');
            std::string sub_dir = (idx != std::string::npos) ? filename.substr(0, idx) : "";
            bool is_resized_file = (base_file_dir + sub_dir == public_upload_dir + "/.resize");
            std::string filename_only = (idx != std::string::npos) ? filename.substr(idx + 1) : "";
            if (base_file_dir + sub_dir == public_upload_dir + (is_resized_file ? "/.resize" : "") && filename_only.size() > 0) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT server_filename FROM public_file_upload WHERE server_filename = ?;");
                prep_stmt->setString(1, filename_only);
                res = prep_stmt->executeQuery();
                if (res->next()) { // if the file exists in MySQL
                    mysql_filename = res->getString(1);
                    if (is_resized_file) {
                        // This will create the resized image if it doesn't exist
                        full_filename = ImageUtils::getResizedImage(mysql_filename, public_upload_dir); //public_upload_dir + "/.resize/" + mysql_filename;
                    } else {
                        full_filename = public_upload_dir + "/" + mysql_filename;
                    }
                    validFilename = true;
                }
                delete res;
                delete prep_stmt;
            }
        }

        // Output file data if filename is valid
        if (validFilename && full_filename.size() > 0 && mysql_filename.size() > 0) {
            std::string mime_type = JlweUtils::getMIMEType(full_filename);

            FILE *file = fopen(full_filename.c_str(), "rb");
            if (file) { // if file exists in filesystem
                // output header
                std::cout << "Access-Control-Allow-Origin: *\r\n";
                std::cout << "Content-type:" << mime_type << "\r\n";
                if (download_request)
                    std::cout << "Content-Disposition: attachment; filename=" << mysql_filename << "\r\n";
                std::cout << "\r\n";

                char buffer[1024];
                size_t size = 1024;
                while (size == 1024){
                    size = fread(buffer, 1, 1024, file);
                    std::cout.write(buffer, size);
                }
                validFile = true;
                fclose(file);
            }
        }

        int response_code = 200;

        // out an error message if something goes wrong
        if (validFile == false){
            response_code = 404;
            std::cout << "Status:404 Not Found\r\n";
            std::cout << "Content-type:text/plain\r\n\r\n";
            std::cout << "The file " + filename + " could not be found on the server\n";
        } else {
            response_code = 200;
        }

        // log download request
        // ignore any errors, the users don't care about the logging
        try {
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT insertFileDownloadLog(?,?,?,?);");
            prep_stmt->setString(1, filename);
            prep_stmt->setString(2, CgiEnvironment::getUserAgent());
            prep_stmt->setString(3, jlwe.getCurrentUserIP());
            prep_stmt->setInt(4, response_code);
            res = prep_stmt->executeQuery();
            delete res;
            delete prep_stmt;
        } catch (...) {}

    } catch (const sql::SQLException &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what() << " (MySQL error code: " << std::to_string(e.getErrorCode()) << ")\n";
    } catch (const std::exception &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what();
    }
    return 0;
}
