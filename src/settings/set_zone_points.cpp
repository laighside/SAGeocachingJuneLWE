/**
  @file    set_zone_points.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/settings/set_zone_points.cgi
  This sets the points for a given zone, or creates a new zone.
  Also allows the deletion of a zone.
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

            std::string kml_name = jsonDocument.at("kml_name");
            std::string zone_name = jsonDocument.value("zone_name", "");
            int points = jsonDocument.value("points", 0);
            bool del = jsonDocument.value("delete", false);


            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setZonePoints(?,?,?,?,?,?);");
            prep_stmt->setString(1, kml_name);
            prep_stmt->setString(2, zone_name);
            prep_stmt->setInt(3, points);
	    prep_stmt->setInt(4, del);
            prep_stmt->setString(5, jlwe.getCurrentUserIP());
            prep_stmt->setString(6, jlwe.getCurrentUsername());
            res = prep_stmt->executeQuery();
            if (res->next()) {
                if (res->getInt(1) == 1) {
                    std::cout << JsonUtils::makeJsonSuccess("Zone updated");
                } else if (res->getInt(1) == 2) {
                    std::cout << JsonUtils::makeJsonSuccess("Zone deleted");
                } else if (res->getInt(1) == 3) {
                    std::cout << JsonUtils::makeJsonSuccess("Zone created");
                } else {
                    std::cout << JsonUtils::makeJsonError("Zone not found");
                }
            } else {
                std::cout << JsonUtils::makeJsonError("Zone not found");
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
