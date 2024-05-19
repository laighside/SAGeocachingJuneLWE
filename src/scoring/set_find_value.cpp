/**
  @file    set_find_value.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/scoring/set_find_value.cgi
  Sets caches as found or not.
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

        if (jlwe.getPermissionValue("perm_pptbuilder")) { //if logged in

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            std::string result = JsonUtils::makeJsonError("Something has gone wrong");

            int team_id = jsonDocument.at("team_id");
            int extras_id = jsonDocument.value("extras_id", 0);
            int cache_number = jsonDocument.value("cache_number", 0);
            int value = jsonDocument.at("value");

            if (cache_number > 0) {

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setTeamFindTrad(?,?,?,?,?);");
                prep_stmt->setInt(1, team_id);
                prep_stmt->setInt(2, cache_number);
                prep_stmt->setInt(3, value);
                prep_stmt->setString(4, jlwe.getCurrentUserIP());
                prep_stmt->setString(5, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next() && res->getInt(1) == 0) {
                    result = JsonUtils::makeJsonSuccess("Find value set");
                } else {
                    result = JsonUtils::makeJsonError("Unable to execute query");
                }
                delete res;
                delete prep_stmt;

            } else if (extras_id != 0) {

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setTeamFindExtra(?,?,?,?,?);");
                prep_stmt->setInt(1, team_id);
                prep_stmt->setInt(2, extras_id);
                prep_stmt->setInt(3, value);
                prep_stmt->setString(4, jlwe.getCurrentUserIP());
                prep_stmt->setString(5, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next() && res->getInt(1) == 0) {
                    result = JsonUtils::makeJsonSuccess("Find value set");
                } else {
                    result = JsonUtils::makeJsonError("Unable to execute query");
                }
                delete res;
                delete prep_stmt;

            }

            std::cout << result;
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
