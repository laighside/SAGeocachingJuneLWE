/**
  @file    set_user_preferences.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/users/set_user_preferences.cgi
  This sets the preferences for a current user.
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

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

        if (jlwe.isLoggedIn()) { //if logged in

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            bool daily_reg = jsonDocument.value("daily_reg", false);
            std::string reg_type = jsonDocument.value("reg_type", "N");
            bool every_reg = jsonDocument.value("every_reg", false);
            bool daily_merch = jsonDocument.value("daily_merch", false);
            std::string merch_type = jsonDocument.value("merch_type", "N");
            bool every_merch = jsonDocument.value("every_merch", false);

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setUserPreferences(?,?,?,?,?,?,?);");
            prep_stmt->setInt(1, jlwe.getCurrentUserId());
            prep_stmt->setString(2, daily_reg ? reg_type : "N");
            prep_stmt->setInt(3, every_reg);
            prep_stmt->setString(4, daily_merch ? merch_type : "N");
            prep_stmt->setInt(5, every_merch);
            prep_stmt->setString(6, jlwe.getCurrentUserIP());
            prep_stmt->setString(7, jlwe.getCurrentUsername());
            res = prep_stmt->executeQuery();
            if (res->next()) {
                if (res->getInt(1) == 0) {
                    std::cout << JsonUtils::makeJsonSuccess("Preferences updated");
                } else {
                    std::cout << JsonUtils::makeJsonError("Invalid user ID");
                }
            } else {
                std::cout << JsonUtils::makeJsonError("Unable to save changes");
            }
            delete res;
            delete prep_stmt;
        } else {
            std::cout << JsonUtils::makeJsonError("you need to be logged in to view this area");
        }
    } catch (sql::SQLException &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")");
    } catch (const std::exception &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()));
    }

    return 0;
}
