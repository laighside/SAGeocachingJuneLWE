/**
  @file    set_webpage_json.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/website_edit/set_webpage_json.cgi
  Sets the HTML and title for a given page on the public website.
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"
#include "../core/PostDataParser.h"

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

            std::string page_request = jsonDocument.value("path", "");
            int draft_page = 0;
            if (page_request.substr((page_request.size() - 2)) == "_0") {
                page_request = page_request.substr(0, page_request.size() - 2);
            } else if (page_request.substr((page_request.size() - 2)) == "_1") {
                draft_page = 1;
                page_request = page_request.substr(0, page_request.size() - 2);
            }

            std::string title = jsonDocument.value("title", "");
            std::string html = jsonDocument.value("html", "");

            if (page_request.size()) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setWebpageHTML(?,?,?,?,?,?);");
                prep_stmt->setString(1, page_request);
                prep_stmt->setInt(2, draft_page);
                prep_stmt->setString(3, title);
                prep_stmt->setString(4, html);
                prep_stmt->setString(5, jlwe.getCurrentUserIP());
                prep_stmt->setString(6, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    if (res->getInt(1) == 1) {
                        std::cout << JsonUtils::makeJsonSuccess("Webpage updated");
                    } else if (res->getInt(1) == 0) {
                        std::cout << JsonUtils::makeJsonSuccess("New webpage created");
                    } else {
                        std::cout << JsonUtils::makeJsonError("Unable to save changes");
                    }
                } else {
                    std::cout << JsonUtils::makeJsonError("Error communicating to database");
                }
                delete res;
                delete prep_stmt;
            } else {
                std::cout << JsonUtils::makeJsonError("Invalid Page");
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
