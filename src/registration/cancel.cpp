/**
  @file    cancel.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/registration/cancel.cgi
  Cancels a registration
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

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

        if (jlwe.getPermissionValue("perm_registrations")) { //if logged in

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            std::string userKey = jsonDocument.at("key");

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setRegistrationStatus(?,?,?,?);");
            prep_stmt->setString(1, userKey);
            prep_stmt->setString(2, "C");
            prep_stmt->setString(3, jlwe.getCurrentUserIP());
            prep_stmt->setString(4, jlwe.getCurrentUsername());
            res = prep_stmt->executeQuery();
            res->next();
            delete res;
            delete prep_stmt;

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT log_user_event(?,?,?);");
            prep_stmt->setString(1, jlwe.getCurrentUserIP());
            prep_stmt->setString(2, jlwe.getCurrentUsername());
            prep_stmt->setString(3, "Registration/Merch order cancelled for order id: " + userKey);
            res = prep_stmt->executeQuery();
            res->next();
            delete res;
            delete prep_stmt;

            std::cout << JsonUtils::makeJsonSuccess("Order cancelled");
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
