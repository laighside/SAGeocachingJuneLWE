/**
  @file    set_variable.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/settings/set_variable.cgi
  This sets the value of a website settings variable.
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/PostDataParser.h"
#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"

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

            std::string var_name = jsonDocument.at("var_name");
            std::string var_value = jsonDocument.at("var_value");


            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setVariable(?,?,?,?,?);");
            prep_stmt->setString(1, var_name);
            prep_stmt->setString(2, var_value);
            prep_stmt->setString(3, jlwe.getCurrentUserIP());
            prep_stmt->setString(4, jlwe.getCurrentUsername());
            prep_stmt->setInt(5, 0); // ignore editable flag = false
            res = prep_stmt->executeQuery();
            if (res->next()) {
                if (res->getInt(1) == 0) {
                    std::cout << JsonUtils::makeJsonSuccess("Variable updated");
                } else {
                    std::cout << JsonUtils::makeJsonError("Unable to write to variable");
                }
            } else {
                std::cout << JsonUtils::makeJsonError("Variable not found");
            }
            delete res;
            delete prep_stmt;
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
