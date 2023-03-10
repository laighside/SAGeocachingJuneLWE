/**
  @file    set_message_status.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/contact_form/set_message_status.cgi
  Sets the status of a message
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

        if (jlwe.isLoggedIn()) { //if logged in

            sql::PreparedStatement *prep_stmt;
            sql::ResultSet *res;

            PostDataParser postData(jlwe.config.at("maxPostSize"));
            if (postData.hasError()) {
                std::cout << JsonUtils::makeJsonError(postData.errorText());
                return 0;
            }

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            int message_id = jsonDocument.at("message_id");
            std::string new_status = jsonDocument.at("status");
            if (new_status.size() > 1)
                new_status = new_status.substr(0, 1);

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setContactFormStatus(?,?,?,?);");
            prep_stmt->setInt(1, message_id);
            prep_stmt->setString(2, new_status);
            prep_stmt->setString(3, jlwe.getCurrentUserIP());
            prep_stmt->setString(4, jlwe.getCurrentUsername());
            res = prep_stmt->executeQuery();
            if (res->next()) {
                std::cout << JsonUtils::makeJsonSuccess("Status successfully changed");
            } else {
                std::cout << JsonUtils::makeJsonError("Error writing to database");
            }
            delete res;
            delete prep_stmt;
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
