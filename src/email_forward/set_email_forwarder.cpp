/**
  @file    set_email_forwarder.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/email_forward/set_email_forwarder.cgi
  Creates, updates and deletes email forwarder settings.
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
#include "../email/Email.h"

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

        if (jlwe.getPermissionValue("perm_email_forward")) { //if logged in

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            std::string source = jsonDocument.at("source");
            std::string destination = jsonDocument.at("destination");
            bool deleteEmail = jsonDocument.at("deleteEmail");

            if (Email::isValidEmail(source + "@" + std::string(jlwe.config.at("emailForwarderDomain")))) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setEmailForwarder(?,?,?,?,?);");
                prep_stmt->setString(1, source);
                prep_stmt->setString(2, destination);
                prep_stmt->setInt(3, deleteEmail);
                prep_stmt->setString(4, jlwe.getCurrentUserIP());
                prep_stmt->setString(5, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    if (res->getInt(1) == 1) {
                        std::cout << JsonUtils::makeJsonSuccess("Email forwarder created");
                    } else if (res->getInt(1) == 2) {
                        std::cout << JsonUtils::makeJsonSuccess("Email forwarder updated");
                    } else if (res->getInt(1) == 3) {
                        std::cout << JsonUtils::makeJsonSuccess("Email forwarder deleted");
                    } else {
                        std::cout << JsonUtils::makeJsonError("Unable to save changes");
                    }
                } else {
                    std::cout << JsonUtils::makeJsonError("SQL Error");
                }
                delete res;
                delete prep_stmt;
            } else {
                std::cout << JsonUtils::makeJsonError("Invalid source address");
            }
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
