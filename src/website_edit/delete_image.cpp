/**
  @file    delete_image.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/website_edit/delete_image.cgi
  Deletes an image from the user uploaded images.
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/PostDataParser.h"
#include "../core/JsonUtils.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << JsonUtils::makeJsonError(postData.errorText());
            return 0;
        }

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_website_edit")) { //if logged in

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            std::string delete_image = jsonDocument.at("delete_image");

            if (delete_image.size()) {
                bool fileExists = false;
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT * FROM webpage_images WHERE filename = ?;");
                prep_stmt->setString(1, delete_image);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    fileExists = true;
                }
                delete res;
                delete prep_stmt;

                if (fileExists) {
                    std::string file_dir = CgiEnvironment::getDocumentRoot() + std::string(jlwe.config.at("files").at("imagePrefix")) + "/";
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT deleteImage(?, ?, ?);");
                    prep_stmt->setString(1, delete_image);
                    prep_stmt->setString(2, jlwe.getCurrentUsername());
                    prep_stmt->setString(3, jlwe.getCurrentUserIP());
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        remove(std::string(file_dir + delete_image).c_str());
                        std::cout << JsonUtils::makeJsonSuccess("Image \"" + delete_image + "\" successfully deleted");
                    } else {
                        std::cout << JsonUtils::makeJsonError("Error deleting file \"" + delete_image + "\"");
                    }
                    delete res;
                    delete prep_stmt;
                } else {
                    std::cout << JsonUtils::makeJsonError("File \"" + delete_image + "\" not found");
                }
            } else {
                std::cout << JsonUtils::makeJsonError("Invalid request (empty file name)");
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
