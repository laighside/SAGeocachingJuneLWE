/**
  @file    set_user_perm.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/users/set_user_perm.cgi
  This sets a given permission for a given user.
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

        if (jlwe.getPermissionValue("perm_admin")) { //if logged in

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            std::string target_username = jsonDocument.at("user");
            std::string perm_name_in = jsonDocument.at("perm_name");
            int perm_setting = jsonDocument.at("setting");

            if (target_username == "admin" || target_username == jlwe.getCurrentUsername()) {
                std::cout << JsonUtils::makeJsonError("You are not allowed to change this permission");
            } else {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setUserPerm(?,?,?,?,?);");
                prep_stmt->setString(1, target_username);
                prep_stmt->setString(2, perm_name_in);
                prep_stmt->setInt(3, perm_setting);
                prep_stmt->setString(4, jlwe.getCurrentUserIP());
                prep_stmt->setString(5, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    if (res->getInt(1) == 0) {
                        std::cout << JsonUtils::makeJsonSuccess("Permission updated");
                    } else {
                        std::cout << JsonUtils::makeJsonError("Invalid permission or username");
                    }
                } else {
                    std::cout << JsonUtils::makeJsonError("SQL Error");
                }
                delete res;
                delete prep_stmt;
            }
        } else {
            std::cout << JsonUtils::makeJsonError("you need to be logged in to view this area");
        }
    } catch (sql::SQLException &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")");
    } catch( const std::exception &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()));
    }

    return 0;
}
