/**
  @file    edit_image.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/public_upload/edit_image.cgi
  Rotates or deletes a image from the public uploads
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <filesystem>

#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/JsonUtils.h"
#include "../core/PostDataParser.h"
#include "../core/Encoder.h"
#include "ImageUtils.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_file")) { //if logged in

            PostDataParser postData(jlwe.config.at("maxPostSize"));
            if (postData.hasError()) {
                std::cout << JsonUtils::makeJsonError(postData.errorText());
                return 0;
            }
            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            if (!jsonDocument.contains("filename") || !jsonDocument.at("filename").is_string())
                throw std::invalid_argument("A string must be provided for \"filename\"");
            std::string filename = jsonDocument.at("filename");

            std::string public_upload_dir = jlwe.config.at("publicFileUpload").at("directory");

            if (jsonDocument.contains("delete") && (jsonDocument.at("delete") == true)) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT deletePublicFile(?,?,?);");
                prep_stmt->setString(1, filename);
                prep_stmt->setString(2, jlwe.getCurrentUsername());
                prep_stmt->setString(3, jlwe.getCurrentUserIP());
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    std::cout << JsonUtils::makeJsonSuccess("Image deleted");
                } else {
                    std::cout << JsonUtils::makeJsonError("Image not found");
                }
                delete res;
                delete prep_stmt;
            } else if (jsonDocument.contains("rotate")) {
                int rotate_deg = jsonDocument.at("rotate");
                std::string full_filename = public_upload_dir + "/" + filename;
                if (!std::filesystem::is_regular_file(full_filename))
                    throw std::invalid_argument("Invalid filename: " + filename);

                std::string command = "jpegtran -rotate " + std::to_string(rotate_deg) + " -outfile " + full_filename + " " + full_filename;
                if (system(command.c_str())) {
                    std::cout << JsonUtils::makeJsonError("Error running jpegtran");
                } else {
                    // Update the resized version of the image as well
                    ImageUtils::getResizedImage(filename, public_upload_dir, true);
                    std::cout << JsonUtils::makeJsonSuccess("Image rotated");
                }
            } else {
                std::cout << JsonUtils::makeJsonError("Invalid request");
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
