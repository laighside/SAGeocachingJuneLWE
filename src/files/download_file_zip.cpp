/**
  @file    download_file_zip.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/files/download_file_zip.cgi
  Downloads a list of files compressed into a ZIP file
  POST requests only, with JSON data, return type is a ZIP file (unless an error occurs).

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/JlweUtils.h"
#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"
#include "../core/PostDataParser.h"

#include "../ext/nlohmann/json.hpp"

std::string removeFileUrlPrefix(std::string href, std::string prefix) {
    size_t prefix_length = prefix.size();
    if (href.substr(0, prefix_length) == prefix) {
        return href.substr(prefix_length);
    } else {
        throw std::invalid_argument("Invalid href file path");
    }
}

int main () {
    try {
        JlweCore jlwe;

        // Allow CORS if file manager is running on a different domain
        /*
        std::cout << "Access-Control-Allow-Origin: *\r\n";
        std::cout << "Access-Control-Allow-Credentials: true\r\n";
        std::cout << "Access-Control-Allow-Methods: GET,HEAD,OPTIONS,POST,PUT\r\n";
        std::cout << "Access-Control-Allow-Headers: Access-Control-Allow-Headers, Origin,Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers\r\n";
        */

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << JsonUtils::makeJsonError(postData.errorText());
            return 0;
        }
        postData.parseUrlEncodedForm();

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_file")) { //if logged in

            std::string public_upload_dir = jlwe.config.at("publicFileUpload").at("directory");
            std::string base_file_dir = jlwe.config.at("files").at("directory");
            bool includePublicUploads = (base_file_dir.size() < public_upload_dir.size()) && (public_upload_dir.substr(0, base_file_dir.size()) == base_file_dir);
            std::string public_upload_sub_dir = "";
            if (includePublicUploads)
                public_upload_sub_dir = public_upload_dir.substr(base_file_dir.size() + 1) + "/";

            //nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());
            nlohmann::json jsonDocument = nlohmann::json::parse(postData.getValue("json"));

            std::string action = jsonDocument.at("action");
            std::string download_filename = jsonDocument.value("as", "files.zip");
            if (download_filename.size() == 0)
                download_filename = "files.zip";
            download_filename = JlweUtils::replaceString(download_filename, "\n", "");
            download_filename = JlweUtils::replaceString(download_filename, "\r", "");
            download_filename = JlweUtils::replaceString(download_filename, " ", "_");
            if (!JlweUtils::compareStringsNoCase(download_filename.substr(download_filename.size() - 4), ".zip"))
                download_filename += ".zip";

            if (JlweUtils::compareStringsNoCase(action, "download") && jsonDocument["hrefs"].is_array()) {

                // make temp folder and filenames
                char dir_template[] = "/tmp/tmpdir.XXXXXX";
                char *dir_name = mkdtemp(dir_template);
                if (dir_name == nullptr)
                    throw std::runtime_error("Unable to create temporary directory");
                std::string tmp_dir = std::string(dir_name);
                std::string zip_dir = tmp_dir + "/files/";
                system(("mkdir " + zip_dir).c_str());

                // Copy each file requested into the zip folder
                for (nlohmann::json::iterator it = jsonDocument["hrefs"].begin(); it != jsonDocument["hrefs"].end(); ++it) {
                    std::string href = *it;
                    href = removeFileUrlPrefix(href, jlwe.config.at("files").at("urlPrefix"));

                    std::string mysql_directory = "";
                    std::string mysql_filename = "";

                    // Check if the requested filename is in the database
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT directory,filename FROM files WHERE CONCAT(directory,filename) = ?;");
                    prep_stmt->setString(1, href);
                    res = prep_stmt->executeQuery();
                    if (res->next()) { // if the file exists in MySQL
                        mysql_directory = res->getString(1);
                        mysql_filename = res->getString(2);
                    }
                    delete res;
                    delete prep_stmt;

                    // If it's not in the file database, check if it's a public upload file
                    if (includePublicUploads && mysql_filename.size() == 0 && mysql_directory.size() == 0) {
                        if (base_file_dir + href == public_upload_dir + "/") { // download entire folder
                            mysql_directory = "/";
                            mysql_filename = public_upload_sub_dir;
                        } else {
                            href = removeFileUrlPrefix(href, "/" + public_upload_sub_dir);
                            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT server_filename FROM public_file_upload WHERE server_filename = ?;");
                            prep_stmt->setString(1, href);
                            res = prep_stmt->executeQuery();
                            if (res->next()) { // if the file exists in MySQL
                                mysql_filename = res->getString(1);
                                mysql_directory = "/" + public_upload_sub_dir;
                            }
                            delete res;
                            delete prep_stmt;
                        }
                    }

                    // If file isn't in the database then ignore it (should stop injection attacks)
                    if (mysql_filename.size() == 0 || mysql_directory.size() == 0)
                        continue;

                    std::string full_filename = base_file_dir + mysql_directory + mysql_filename;
                    bool is_directory = (mysql_filename.at(mysql_filename.size() - 1) == '/');

                    if (is_directory) {
                        system(("cp -R " + full_filename + " " + zip_dir + mysql_filename).c_str());
                    } else {
                        system(("cp " + full_filename + " " + zip_dir + mysql_filename).c_str());
                    }
                }

                // zip the folder to make single file
                std::string zip_filename = tmp_dir + "/zip_file.zip";
                system(("cd " + zip_dir + " ; zip -q -r " + zip_filename + " . -i \\*").c_str());

                FILE *file = fopen(zip_filename.c_str(), "rb");
                if (file) { //if file exists in filesystem
                    //output header
                    std::cout << "Content-type: application/zip\r\n";
                    std::cout << "Content-Disposition: attachment; filename=" << download_filename << "\r\n\r\n";

                    JlweUtils::readFileToOStream(file, std::cout);
                    fclose(file);
                } else {
                    std::cout << JsonUtils::makeJsonError("Unable to read zip file");
                }

                system(("rm -r " + tmp_dir + "/").c_str());

            } else {
                std::cout << JsonUtils::makeJsonError("Invalid action");
            }
        } else {
            std::cout << JsonUtils::makeJsonError("You need to be logged in to view this area");
        }
    } catch (sql::SQLException &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")");
    } catch (const std::exception &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()));
    }

    return 0;
}
