/**
  @file    gd_list_files.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/public_upload/gd_list_files.cgi
  Gets a list of folders/files in Google Drive
  GET requests only, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/JsonUtils.h"
#include "../core/KeyValueParser.h"
#include "../core/HttpRequest.h"
#include "../core/Encoder.h"

#include "GoogleAuthToken.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        if (jlwe.getPermissionValue("perm_file")) { //if logged in

            KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);
            bool folders_only = JlweUtils::compareStringsNoCase(urlQueries.getValue("type"), "folder");
            bool files_only = JlweUtils::compareStringsNoCase(urlQueries.getValue("type"), "file");
            std::string parent = urlQueries.getValue("parent");
            bool sharedWithMe = JlweUtils::compareStringsNoCase(urlQueries.getValue("sharedWithMe"), "true");

            // Query examples: https://developers.google.com/workspace/drive/api/guides/search-files#examples
            std::string query;
            if (folders_only) {
                query = "mimeType = 'application/vnd.google-apps.folder'";
            } else if (files_only) {
                query = "mimeType != 'application/vnd.google-apps.folder'";
            }
            if (parent.size()) {
                if (query.size())
                    query += " and ";
                query += "'" + parent + "' in parents";
            }
            if (sharedWithMe) {
                if (query.size())
                    query += " and ";
                query += "sharedWithMe";
            }

            std::string auth_header = GoogleAuthToken::getAuthorizationHeader(&jlwe);
            HttpRequest request("https://www.googleapis.com/drive/v3/files?q=" + Encoder::urlEncode(query));
            request.setHeader("Authorization: " + auth_header);
            if (request.get()) {
                nlohmann::json jsonResponse = nlohmann::json::parse(request.responseAsString());
                std::cout << JsonUtils::makeJsonHeader() << jsonResponse.dump();
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
