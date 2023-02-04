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

int main () {
    try{
        JlweCore jlwe;
        std::string page_request = CgiEnvironment::getRequestUri();

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

        std::string file_dir = jlwe.config.at("files").at("directory");

        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT filename,public FROM files WHERE CONCAT(directory,filename) = ?;");
        prep_stmt->setString(1, filename);
        res = prep_stmt->executeQuery();

        bool validFile = false;

        if (res->next()){ // if the file exists in MySQL
            if (res->getInt(2) != 0 || jlwe.isLoggedIn()){ // if public file or user is logged in
                std::string mysql_filename = res->getString(1);

                // check if it is a file not a directory
                if (mysql_filename.at(mysql_filename.size() - 1) != '/') {

                    // this should be safe from injection attacks since filename is confirmed to exist in the database
                    std::string full_filename = file_dir + filename;
                    std::string mime_type = JlweUtils::getMIMEType(full_filename);

                    FILE *file = fopen(full_filename.c_str(), "rb");
                    if (file) { // if file exists in filesystem
                        // output header
                        std::cout << "Access-Control-Allow-Origin: *\r\n";
                        std::cout << "Content-type:" << mime_type << "\r\n";
                        std::cout << "Content-Disposition: attachment; filename=" << mysql_filename << "\r\n\r\n";

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
            }
        }
        delete res;
        delete prep_stmt;

        // out an error message if something goes wrong
        if (validFile == false){
            std::cout << "Status:404 Not Found\r\n";
            std::cout << "Content-type:text/plain\r\n\r\n";
            std::cout << "The file " + filename + " could not be found on the server\n";
        }

    } catch (const sql::SQLException &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what() << " (MySQL error code: " << std::to_string(e.getErrorCode()) << ")\n";
    } catch (const std::exception &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what();
    }
    return 0;
}
