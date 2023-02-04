/**
  @file    upload_image.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/website_edit/upload_image.cgi
  Uploads an image for use on the public website.
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

        if (jlwe.getPermissionValue("perm_website_edit")) { //if logged in
            if (postData.getFiles()->size() == 1) {
                PostDataParser::FormFile inputFile = postData.getFiles()->at(0);

                std::string filename = Encoder::filterSafeCharsOnly(inputFile.filename);

                if (filename.size()) {

                    if (inputFile.dataType.find("image/") == std::string::npos) {
                        std::cout << JsonUtils::makeJsonError("The uploaded file is not an image file");
                    } else {
                        bool imageExists = false;
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT filename FROM webpage_images WHERE filename = ?;");
                        prep_stmt->setString(1, filename);
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                            imageExists = true;
                        }
                        delete res;
                        delete prep_stmt;

                        if (imageExists) {
                            std::cout << JsonUtils::makeJsonError("The file \"" + filename + "\" already exists");
                        } else {
                            std::string file_dir = CgiEnvironment::getDocumentRoot() + std::string(jlwe.config.at("files").at("imagePrefix")) + "/";
                            FILE *file = fopen(std::string(file_dir + filename).c_str(), "wb");
                            if (file){
                                fwrite(inputFile.data.data(), 1, inputFile.data.size(), file);
                                fclose(file);

                                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT createWebpageImage(?,?,?,?);");
                                prep_stmt->setString(1, filename);
                                prep_stmt->setUInt(2, inputFile.data.size());
                                prep_stmt->setString(3, jlwe.getCurrentUsername());
                                prep_stmt->setString(4, jlwe.getCurrentUserIP());
                                res = prep_stmt->executeQuery();
                                if (res->next()) {

                                    nlohmann::json jsonDocument;

                                    jsonDocument["success"] = true;
                                    jsonDocument["message"] = "Image: \"" + filename + "\" successfully uploaded";
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
                    }
                } else {
                    std::cout << JsonUtils::makeJsonError("Invalid file name");
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
