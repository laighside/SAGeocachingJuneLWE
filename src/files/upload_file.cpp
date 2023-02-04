/**
  @file    upload_file.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/files/upload_file.cgi
  Uploads a file to the file manager.
  POST requests only, with multipart/form-data data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/Encoder.h"
#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"
#include "../core/PostDataParser.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        PostDataParser postData(jlwe.config.at("files").at("maxUploadSize"));
        if (postData.hasError()) {
            std::cout << JsonUtils::makeJsonError(postData.errorText());
            return 0;
        }

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_file")) { // if logged in
            if (postData.getFiles()->size() == 1) {
                PostDataParser::FormFile inputFile = postData.getFiles()->at(0);

                std::string filename = Encoder::filterSafeCharsOnly(inputFile.filename);
                std::string baseHref = postData.getValue("baseHref", "");

                size_t prefix_length = std::string(jlwe.config.at("files").at("urlPrefix")).size();
                if (baseHref.substr(0, prefix_length) == std::string(jlwe.config.at("files").at("urlPrefix"))) {
                    baseHref = baseHref.substr(prefix_length);

                    bool baseDirExists = (baseHref == "/");
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT filename FROM files WHERE CONCAT(directory,filename) = ?;");
                    prep_stmt->setString(1, baseHref);
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        baseDirExists = true;
                    }
                    delete res;
                    delete prep_stmt;

                    if (filename.size() && baseDirExists) {

                        bool fileExists = false;
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT filename FROM files WHERE directory = ? AND filename = ?;");
                        prep_stmt->setString(1, baseHref);
                        prep_stmt->setString(2, filename);
                        res = prep_stmt->executeQuery();
                        if (res->next()){
                            fileExists = true;
                        }
                        delete res;
                        delete prep_stmt;

                        if (fileExists) {
                            std::cout << JsonUtils::makeJsonError("The file \"" + filename + "\" already exists");
                        } else {
                            // this should be ok since baseHref is confimed to exist in the database
                            std::string file_dir = jlwe.config.at("files").at("directory");
                            FILE *file = fopen(std::string(file_dir + baseHref + filename).c_str(), "wb");
                            if (file) {
                                fwrite(inputFile.data.data(), 1, inputFile.data.size(), file);
                                fclose(file);

                                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT createFile(?,?,?,?,?,?,?);");
                                prep_stmt->setString(1, filename);
                                prep_stmt->setString(2, baseHref);
                                prep_stmt->setInt(3, 0);
                                prep_stmt->setInt(4, 0);
                                prep_stmt->setString(5, jlwe.getCurrentUsername());
                                prep_stmt->setUInt(6, inputFile.data.size());
                                prep_stmt->setString(7, jlwe.getCurrentUserIP());
                                res = prep_stmt->executeQuery();
                                if (res->next()) {

                                    nlohmann::json jsonDocument;

                                    jsonDocument["success"] = true;
                                    jsonDocument["message"] = "File: \"" + filename + "\" successfully uploaded";
                                    jsonDocument["filename"] = filename;

                                    std::cout << JsonUtils::makeJsonHeader() + jsonDocument.dump();
                                } else {
                                    std::cout << JsonUtils::makeJsonError("Error saving file in database");
                                }
                                delete res;
                                delete prep_stmt;
                            } else {
                                std::cout << JsonUtils::makeJsonError("Error saving file on server");
                            }
                        }
                    } else {
                        std::cout << JsonUtils::makeJsonError("Invalid file/folder name");
                    }
                } else {
                    std::cout << JsonUtils::makeJsonError("Invalid folder name");
                }
            } else {
                std::cout << JsonUtils::makeJsonError("There must be only one file in the upload, this request has " + std::to_string(postData.getFiles()->size()) + " files");
            }
        } else {
            std::cout << JsonUtils::makeJsonError("You do not have permission to view this area");
        }
    } catch (sql::SQLException &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")");
    } catch (const std::exception &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()));
    }

   return 0;
}
