/**
  @file    set_cache.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/gpx_builder/set_cache.cgi
  Sets the details for a given cache
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"
#include "../core/KeyValueParser.h"
#include "../core/PostDataParser.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << JsonUtils::makeJsonError(postData.errorText());
            return 0;
        }

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_gpxbuilder")) { //if logged in

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            std::string cache_name = jsonDocument.at("cache_name");
            std::string team_name = jsonDocument.at("team_name");
            double lat = jsonDocument.at("latitude");
            double lon = jsonDocument.at("longitude");
            std::string public_hint = jsonDocument.at("public_hint");
            std::string detailed_hint = jsonDocument.at("detailed_hint");
            bool camo = jsonDocument.at("camo");
            bool permanent = jsonDocument.at("permanent");
            bool private_property = jsonDocument.at("private_property");
            int zone_bonus = jsonDocument.at("zone_bonus");
            int osm_distance = jsonDocument.at("osm_distance");
            int actual_distance = jsonDocument.value("actual_distance", -1);

            int id_number = jsonDocument.value("id_number", 0);

            int cache_number = 0;
            try {
                cache_number = std::stoi(urlQueries.getValue("cache_number"));
            } catch (...) {}
            if (cache_number) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setCacheDetails(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
                prep_stmt->setInt(1, cache_number);
                prep_stmt->setString(2, cache_name);
                prep_stmt->setString(3, team_name);
                prep_stmt->setDouble(4, lat);
                prep_stmt->setDouble(5, lon);
                prep_stmt->setString(6, public_hint);
                prep_stmt->setString(7, detailed_hint);
                prep_stmt->setInt(8, camo);
                prep_stmt->setInt(9, permanent);
                prep_stmt->setInt(10, private_property);
                prep_stmt->setInt(11, zone_bonus);
                prep_stmt->setInt(12, osm_distance);
                prep_stmt->setInt(13, actual_distance);
                prep_stmt->setString(14, jlwe.getCurrentUserIP());
                prep_stmt->setString(15, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    if (res->getInt(1)) {
                        std::cout << JsonUtils::makeJsonSuccess("Cache updated");
                    } else {
                        std::cout << JsonUtils::makeJsonSuccess("New cache created");
                    }
                } else {
                    std::cout << JsonUtils::makeJsonError("Cache not found");
                }
                delete res;
                delete prep_stmt;

                if (id_number) {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setCacheStatus(?,?,?,?);");
                    prep_stmt->setInt(1, id_number);
                    prep_stmt->setString(2, "S");
                    prep_stmt->setString(3, jlwe.getCurrentUserIP());
                    prep_stmt->setString(4, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    res->next();
                    delete res;
                    delete prep_stmt;
                }
            } else {
                std::cout << JsonUtils::makeJsonError("Invalid cache number");
            }
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
