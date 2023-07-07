/**
  @file    get_scores.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/scoring/get_scores.cgi
  Gets a list of game teams, team members, zone and return points and final scores.
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

struct cache {
    int cache_number;
    int team_id;
    int zone_bonus;
    bool has_coordinates;
    bool handout;
    bool returned;
};

bool sortIntDesc (int i,int j) { return (i>j); }

int main () {
    try {
        JlweCore jlwe;

        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        sql::Statement *stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_pptbuilder")) { //if logged in

            bool include_non_compete = (urlQueries.getValue("include_non_compete") == "true");

            // Check that number_game_caches is set to a valid value
            int number_game_caches = 0;
            try {
                number_game_caches = std::stoi(jlwe.getGlobalVar("number_game_caches"));
            } catch (...) {}
            if (number_game_caches < 1)
                throw std::invalid_argument("Invalid setting for number_game_caches = " + std::to_string(number_game_caches));

            std::vector<bool> caches_allocated(static_cast<size_t>(number_game_caches), false);

            std::vector<cache> cache_list;

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT cache_handout.cache_number, cache_handout.team_id, caches.cache_number, IF(cache_handout.owner_name = '', 0, 1), caches.zone_bonus, cache_handout.returned FROM caches RIGHT OUTER JOIN cache_handout ON caches.cache_number=cache_handout.cache_number;");
            while (res->next()) {
                cache c;
                c.cache_number = res->getInt(1);
                c.team_id = res->getInt(2);
                c.has_coordinates = !(res->isNull(3));
                c.handout = (res->getInt(4) > 0);
                c.zone_bonus = res->getInt(5);
                c.returned = (res->getInt(6) > 0);
                cache_list.push_back(c);
            }
            delete res;
            delete stmt;


            nlohmann::json jsonDocument;

            bool warning_cache_not_in_handout = false;
            bool warning_cache_not_in_gpx = false;

            jsonDocument["teams"] = nlohmann::json::array();

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT team_id, team_name, team_members, competing, final_score FROM game_teams" + std::string(include_non_compete ? ";" : " WHERE competing = 1;"));
            while (res->next()) {

                nlohmann::json jsonObject;

                int team_id = res->getInt(1);
                jsonObject["team_id"] = team_id;
                jsonObject["team_name"] = res->getString(2);
                jsonObject["team_members"] = res->getString(3);
                jsonObject["competing"] = (res->getInt(4) > 0);
                if (res->isNull(5)) {
                    jsonObject["final_score"] = nullptr;
                } else {
                    jsonObject["final_score"] = res->getInt(5);
                }

                jsonObject["caches"] = nlohmann::json::array();

                std::vector<int> zone_points;
                int not_returned_points = 0;
                for (unsigned int i = 0; i < cache_list.size(); i++) {
                    if (cache_list.at(i).team_id == team_id) {
                        nlohmann::json jsonObject2;
                        cache c = cache_list.at(i);

                        jsonObject2["cache_number"] = c.cache_number;
                        jsonObject2["has_coordinates"] = c.has_coordinates;
                        jsonObject2["handout"] = c.handout;
                        jsonObject2["zone_bonus"] = c.zone_bonus;
                        jsonObject2["returned"] = c.returned;

                        if (c.returned == false)
                            not_returned_points += -2;
                        zone_points.push_back(c.zone_bonus);

                        if (c.handout == false)
                            warning_cache_not_in_handout = true;
                        if (c.has_coordinates == false)
                            warning_cache_not_in_gpx = true;

                        jsonObject["caches"].push_back(jsonObject2);

                        caches_allocated[static_cast<size_t>(c.cache_number - 1)] = true;
                    }
                }

                std::sort(zone_points.begin(), zone_points.end(), sortIntDesc);
                int zone_total = 0;
                if (zone_points.size() > 0)
                    zone_total += zone_points.at(0);
                if (zone_points.size() > 1)
                    zone_total += zone_points.at(1);

                jsonObject["zone_points"] = zone_total;
                jsonObject["returned_points"] = not_returned_points;

                jsonDocument["teams"].push_back(jsonObject);
            }
            delete res;
            delete stmt;

            jsonDocument["warning_cache_not_in_handout"] = warning_cache_not_in_handout;
            jsonDocument["warning_cache_not_in_gpx"] = warning_cache_not_in_gpx;

            jsonDocument["unallocated_caches"] = nlohmann::json::array();
            for (unsigned int i = 0; i < caches_allocated.size(); i++) {
                if (caches_allocated.at(i) == false)
                    jsonDocument["unallocated_caches"].push_back(i + 1);
            }

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
