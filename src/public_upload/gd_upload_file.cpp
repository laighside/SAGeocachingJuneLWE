/**
  @file    gd_upload_file.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/public_upload/gd_upload_file.cgi
  Sends one or more files to Google Drive
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/JsonUtils.h"
#include "../core/PostDataParser.h"
#include "../core/HttpRequest.h"
#include "../core/Encoder.h"

#include "GoogleAuthToken.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << JsonUtils::makeJsonError(postData.errorText());
            return 0;
        }

        if (jlwe.getPermissionValue("perm_file")) { //if logged in

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            std::string filename = jsonDocument.at("filename");
            std::string parent_id = jsonDocument.at("parent");

            if (!parent_id.size())
                throw std::invalid_argument("Parent ID must be set");

            std::string auth_header = GoogleAuthToken::getAuthorizationHeader(&jlwe);

            // check if file already exists
            std::string existing_id = "";
            HttpRequest check_exists_request("https://www.googleapis.com/drive/v3/files?q=" + Encoder::urlEncode("name = '" + filename + "' and '" + parent_id + "' in parents"));
            check_exists_request.setHeader("Authorization: " + auth_header);
            if (check_exists_request.get()) {
                nlohmann::json jsonResponse = nlohmann::json::parse(check_exists_request.responseAsString());
                if (jsonResponse.contains("files") && jsonResponse.at("files").is_array() && jsonResponse.at("files").size()) {
                    existing_id = jsonResponse.at("files").at(0).at("id");
                }
            }

            std::string public_upload_dir = jlwe.config.at("publicFileUpload").at("directory");
            std::string mime_type = JlweUtils::getMIMEType((public_upload_dir + "/" + filename).c_str());

            nlohmann::json metadata = {{"name", filename}, {"mimeType", mime_type}};
            if (existing_id.size() == 0)
                metadata["parents"] = {parent_id};

            std::string boundary = JlweUtils::makeRandomToken(20);
            std::string post_data = "--" + boundary + "\r\n";
            post_data += "Content-Type: application/json\r\n\r\n";
            post_data += metadata.dump() + "\r\n\r\n";
            post_data += "--" + boundary + "\r\n";
            post_data += "Content-Type: " + mime_type + "\r\n";
            post_data += "Content-Transfer-Encoding: base64\r\n\r\n";
            post_data += Encoder::base64encode(JlweUtils::readFileToString((public_upload_dir + "/" + filename).c_str())) + "\r\n";
            post_data += "--" + boundary + "--";

            HttpRequest request("https://www.googleapis.com/upload/drive/v3/files" + (existing_id.size() > 0 ? "/" + existing_id : "") + "?uploadType=multipart");
            request.setHeader("Authorization: " + auth_header);
            if (request.post(post_data, "multipart/mixed; boundary=" + boundary + ";", false, existing_id.size() > 0)) {
                try {
                    nlohmann::json jsonResponse = nlohmann::json::parse(request.responseAsString());
                    std::cout << JsonUtils::makeJsonHeader() << jsonResponse.dump();
                } catch (nlohmann::json::parse_error& e) {
                    nlohmann::json jsonResponse {{"error", e.what()}, {"response", request.responseAsString()}};
                    std::cout << JsonUtils::makeJsonHeader() << jsonResponse.dump();
                }
            } else {
                std::cout << JsonUtils::makeJsonError(request.errorMessage());
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
