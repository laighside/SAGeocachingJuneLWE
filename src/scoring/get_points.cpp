/**
  @file    get_points.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/scoring/get_points.cgi
  Gets a list of points for each cache.
  GET requests, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/KeyValueParser.h"
#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_pptbuilder")) { //if logged in

            // Root element of the JSON to return
            nlohmann::json jsonDocument;
            jsonDocument["find_points_extras"] = nlohmann::json::array();
            jsonDocument["find_points_trads"] = nlohmann::json::array();
            jsonDocument["zones"] = nlohmann::json::array();

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT id, name, point_value, enabled FROM game_find_points_extras ORDER BY id;");
            while (res->next()) {
                nlohmann::json jsonObject;
                jsonObject["id"] = res->getInt(1);
                jsonObject["name"] = res->getString(2);
                jsonObject["point_value"] = res->getInt(3);
                jsonObject["enabled"] = (res->getInt(4) != 0);

                jsonDocument["find_points_extras"].push_back(jsonObject);
            }
            delete res;
            delete stmt;

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT id, name, enabled FROM game_find_points_trads ORDER BY id;");
            while (res->next()) {
                nlohmann::json jsonObject;
                jsonObject["id"] = res->getInt(1);
                jsonObject["name"] = res->getString(2);
                jsonObject["enabled"] = (res->getInt(3) != 0);

                jsonDocument["find_points_trads"].push_back(jsonObject);
            }
            delete res;
            delete stmt;

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT id, kml_file, name, points, enabled FROM zones;");
            while (res->next()) {
                nlohmann::json jsonObject;
                jsonObject["id"] = res->getInt(1);
                jsonObject["kml_file"] = res->getString(2);
                jsonObject["name"] = res->getString(3);
                jsonObject["points"] = res->getInt(4);
                jsonObject["enabled"] = (res->getInt(5) != 0);

                jsonDocument["zones"].push_back(jsonObject);
            }
            delete res;
            delete stmt;

            // Make a list of all the KML files (these are the options for creating new zones)
            jsonDocument["kml_file_list"] = nlohmann::json::array();
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT CONCAT(directory,filename) FROM files WHERE RIGHT(filename, 4) = '.kml';");
            while (res->next()){
                jsonDocument["kml_file_list"].push_back(res->getString(1));
            }
            delete res;
            delete stmt;

            std::cout << JsonUtils::makeJsonHeader() << jsonDocument.dump();

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
