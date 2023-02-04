/**
  @file    get_webpage_json.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/website_edit/get_webpage_json.cgi
  Gets the HTML and title for a given page on the public website.
  GET requests, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"
#include "../core/KeyValueParser.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_website_edit")) { //if logged in

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT html,page_name FROM webpages WHERE path = ?;");
            prep_stmt->setString(1, urlQueries.getValue("page"));
            res = prep_stmt->executeQuery();
            if (res->next()) {
                nlohmann::json jsonDocument;

                jsonDocument["html"] = res->getString(1);
                jsonDocument["title"] = res->getString(2);

                std::cout << JsonUtils::makeJsonHeader() + jsonDocument.dump();
            } else {
                std::cout << JsonUtils::makeJsonError("Page not found");
            }
            delete res;
            delete prep_stmt;
        } else {
            std::cout << JsonUtils::makeJsonError("You do not have permission to view this area");
        }
    } catch (sql::SQLException &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")");
    } catch( const std::exception &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()));
    }

    return 0;
}
