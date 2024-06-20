/**
  @file    thumbnail.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/files/thumbnail.cgi
  This gets the thumbnail for a given file. Imagemagick is used to generate the thumbnails.
  GET requests only, return type is a JPEG image if there is a valid thumbnail for the file.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/KeyValueParser.h"

int main () {
    try {
        JlweCore jlwe;

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);
        std::string type = urlQueries.getValue("type");
        std::string filename = urlQueries.getValue("file");

        // remove the url prefix
        size_t prefix_length = std::string(jlwe.config.at("files").at("urlPrefix")).size();
        if (filename.substr(0, prefix_length) == std::string(jlwe.config.at("files").at("urlPrefix")))
            filename = filename.substr(prefix_length);

        size_t fileIndex = filename.find_last_of('/');
        std::string directory = "/";
        if (fileIndex != std::string::npos) {
            directory = filename.substr(0, fileIndex + 1);
            filename = filename.substr(fileIndex + 1);
        }

        int width = 100;
        try {
            width = std::stoi(urlQueries.getValue("w"));
        } catch (...) {}
        int height = 100;
        try {
            height = std::stoi(urlQueries.getValue("h"));
        } catch (...) {}

        std::string public_upload_dir = jlwe.config.at("publicFileUpload").at("directory");
        std::string base_file_dir = jlwe.config.at("files").at("directory");
        bool includePublicUploads = (base_file_dir.size() < public_upload_dir.size()) && (public_upload_dir.substr(0, base_file_dir.size()) == base_file_dir);

        std::string mysql_filename = "";
        bool validFilename = false;
        bool isPublicFile = false;

        // check that it's a valid filename to prevent directory traversal attacks
        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT CONCAT(directory,filename),public FROM files WHERE filename = ? AND directory = ?;");
        prep_stmt->setString(1, filename);
        prep_stmt->setString(2, directory);
        res = prep_stmt->executeQuery();
        if (res->next()) {
            mysql_filename = res->getString(1);
            isPublicFile = res->getInt(2) > 0;
            validFilename = true;
        }
        delete res;
        delete prep_stmt;

        // Check if it's in public upload folder
        if ((!validFilename) && includePublicUploads) {
            if (base_file_dir + directory == public_upload_dir + "/") {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT server_filename FROM public_file_upload WHERE server_filename = ?;");
                prep_stmt->setString(1, filename);
                res = prep_stmt->executeQuery();
                if (res->next()) { // if the file exists in MySQL
                    mysql_filename = public_upload_dir.substr(base_file_dir.size()) + "/" + res->getString(1);
                    validFilename = true;
                }
                delete res;
                delete prep_stmt;
            }
        }

        if (isPublicFile || jlwe.getPermissionValue("perm_file")) { // only get thumbnail if it's a public file or the user is logged in

            if (validFilename && mysql_filename.size() > 0) {

                // Thumbnails should be cached, not dynamically generated
                std::string thumbFile = base_file_dir + "/.thumb" + mysql_filename + ".thumb." + std::to_string(width) + "x" + std::to_string(height) + ".jpg";

                // check if thumbnail aready exists
                bool thumbnailCached = false;
                FILE *file = fopen(thumbFile.c_str(), "rb");
                if (file) { //if file exists in filesystem
                    thumbnailCached = true;
                    fclose(file);
                }

                if (!thumbnailCached) {
                    if (type == "img") {
                        std::string command = "convert -thumbnail " + std::to_string(width) + "x" + std::to_string(height) + "^ -gravity Center -extent " + std::to_string(width) + "x" + std::to_string(height);
                        command += " " + base_file_dir + mysql_filename;
                        command += " " + thumbFile;
                        system(command.c_str());
                    } else if (type == "doc") {
                        std::string command = "convert -density 100 -colorspace rgb";
                        command += " " + base_file_dir + mysql_filename;
                        command += " -scale " + std::to_string(width) + "x" + std::to_string(height) + "^ -gravity Center -extent " + std::to_string(width) + "x" + std::to_string(height);
                        command += " " + thumbFile;
                        system(command.c_str());
                    }
                }


                file = fopen(thumbFile.c_str(), "rb");
                if (file) { // if file exists in filesystem

                    std::cout << "Content-type:image/jpeg\r\n\r\n";

                    char buffer[1024];
                    size_t size = 1024;
                    while (size == 1024){
                        size = fread(buffer, 1, 1024, file);
                        std::cout.write(buffer, size);
                    }
                    fclose(file);

                } else {
                    std::cout << "Content-type:text/plain\r\n\r\n";
                    std::cout << "Invalid file.\n";
                }
            } else {
                std::cout << "Content-type:text/plain\r\n\r\n";
                std::cout << "Invalid file.\n";
            }

        } else {
            std::cout << "Content-type:text/plain\r\n\r\n";
            std::cout << "Invalid file.\n";
        }
    } catch (sql::SQLException &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")\n";
    } catch (const std::exception &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what() << "\n";
    }

    return 0;
}
