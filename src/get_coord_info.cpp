/**
  @file    get_coord_info.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  /cgi-bin/get_coord_info.cgi?lat=-34.8&lon=138.5
  Return format is always JSON
  Takes a lat/lon in the URL arugments and return the follow info about that point:
   - inside or outside the playing field
   - if inside any bonus point zones
   - distance from nearest road (based on osm_roads_kml file)
  This is called from javascript on /hide.html

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <set>

#include "core/KeyValueParser.h"
#include "core/CgiEnvironment.h"
#include "core/JlweUtils.h"
#include "core/JlweCore.h"
#include "core/JsonUtils.h"

#include "ext/nlohmann/json.hpp"

// This is for reading the KML files of the playing field, bonus zones and roads
#include "kml/KmlFile.h"

int main () {
    try {
        JlweCore jlwe;

        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        double lat = 0;
        try {
            lat = std::stod(urlQueries.getValue("lat"));
        } catch (...) {}
        double lon = 0;
        try {
            lon = std::stod(urlQueries.getValue("lon"));
        } catch (...) {}
        if (lat < -90 || lat > 90 || lon < -180 || lon > 180)
            throw std::invalid_argument("Invalid coordinates, lat must be in the range (-90,+90), long must be in the range (-180,+180)");

        std::string fileDir = jlwe.config.at("files").at("directory");

        std::string playingFieldKML = jlwe.getGlobalVar("playing_field_kml");
        if (!playingFieldKML.size())
            throw std::runtime_error("Playing field KML file is not set");

        std::string osmRoadsKML = jlwe.getGlobalVar("osm_roads_kml");
        if (!osmRoadsKML.size())
            throw std::runtime_error("Roads KML file is not set");

        KmlFile kmlPlayingField;
        if (!kmlPlayingField.loadFile(fileDir + playingFieldKML))
            throw std::runtime_error("Error loading KML file: " + kmlPlayingField.ParseError());

        // check if point is in the playing field
        bool pointInPlayingField = kmlPlayingField.pointInPolygon(lat, lon);

        nlohmann::json jsonDocument;

        jsonDocument["in_playing_field"] = pointInPlayingField;

        std::set<int> groups_done;

        if (pointInPlayingField == true){
            // check if point is in any bonus zones
            jsonDocument["bonus_zones"] = nlohmann::json::array();
            sql::Statement *stmt = jlwe.getMysqlCon()->createStatement();
            sql::ResultSet *res = stmt->executeQuery("SELECT kml_file,name,points,zone_group FROM zones WHERE enabled != 0 ORDER BY points DESC;");
            while (res->next()){
                if (groups_done.count(res->getInt(4)))
                    continue; // skip zone if that group is already matched

                nlohmann::json jsonOject;
                std::string zone_kml = res->getString(1);
                KmlFile kmlZone;
                if (kmlZone.loadFile(fileDir + zone_kml)) {
                    bool pointInZone = kmlZone.pointInPolygon(lat, lon);
                    if (pointInZone == true){
                        jsonOject["name"] = res->getString(2);
                        jsonOject["points"] = res->getInt(3);
                        jsonOject["group"] = res->getInt(4);
                        jsonDocument["bonus_zones"].push_back(jsonOject);
                        groups_done.insert(res->getInt(4));
                    }
                } else {
                    jsonOject["name"] = res->getString(2);
                    jsonOject["error"] = kmlZone.ParseError();
                    jsonDocument["bonus_zones"].push_back(jsonOject);
                }
            }
            delete res;
            delete stmt;

            // work out how far the point is from the nearest road
            KmlFile kmlRoads;
            if (kmlRoads.loadFile(fileDir + osmRoadsKML)) {
                int distance = static_cast<int>(round(kmlRoads.distanceFromPoint(lat, lon)));
                jsonDocument["from_osm_road"] = distance;
            } else {
                jsonDocument["road_kml_error"] = kmlRoads.ParseError();
                jsonDocument["from_osm_road"] = -1;
            }

        }

        // output JSON result
        std::cout << JsonUtils::makeJsonHeader() + jsonDocument.dump();

    } catch (sql::SQLException &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")");
    } catch (const std::exception &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()));
    }

    return 0;
}
