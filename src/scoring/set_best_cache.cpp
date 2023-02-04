/**
  @file    set_best_cache.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/scoring/set_best_cache.cgi
  Sets the value for the best cache award for a given category.
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

            int award_id = jsonDocument.at("award_id");
            std::string cache_name = jsonDocument.at("cache_name");

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setBestCache(?,?,?,?);");
            prep_stmt->setInt(1, award_id);
            prep_stmt->setString(2, cache_name);
            prep_stmt->setString(3, jlwe.getCurrentUserIP());
            prep_stmt->setString(4, jlwe.getCurrentUsername());
            res = prep_stmt->executeQuery();
            if (res->next()) {
                if (res->getInt(1) == 0) {
                    std::cout << JsonUtils::makeJsonSuccess("Winning cache updated");
                } else {
                    std::cout << JsonUtils::makeJsonError("Award title not found");
                }
            } else {
                std::cout << JsonUtils::makeJsonError("Award title not found");
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
