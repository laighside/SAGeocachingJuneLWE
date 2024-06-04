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

#include "PointCalculator.h"

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

            PointCalculator point_calculator(&jlwe, number_game_caches);

            std::vector<bool> caches_allocated(static_cast<size_t>(number_game_caches), false);

            nlohmann::json jsonDocument;
            jsonDocument["number_game_caches"] = number_game_caches;

            bool warning_cache_not_in_handout = false;
            bool warning_cache_not_in_gpx = false;

            jsonDocument["cache_list"] = nlohmann::json::array();
            for (unsigned int i = 0; i < number_game_caches; i++)
                jsonDocument["cache_list"].push_back(nullptr);

            std::vector<PointCalculator::Cache> * cache_list = point_calculator.getCacheList();
            for (unsigned int i = 0; i < cache_list->size(); i++) {
                int cache_number = cache_list->at(i).cache_number;
                if (cache_number > number_game_caches)
                    continue;

                nlohmann::json jsonObject;
                jsonObject["cache_number"] = cache_number;
                jsonObject["team_id"] = cache_list->at(i).team_id;
                jsonObject["has_coordinates"] = cache_list->at(i).has_coordinates;
                jsonObject["handout"] = cache_list->at(i).handout;
                jsonObject["returned"] = cache_list->at(i).returned;
                jsonObject["total_hide_points"] = cache_list->at(i).total_hide_points;
                jsonObject["total_find_points"] = cache_list->at(i).total_find_points;
                jsonDocument["cache_list"][cache_number - 1] = jsonObject;
            }

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
                for (unsigned int i = 0; i < cache_list->size(); i++) {
                    if (cache_list->at(i).team_id == team_id) {
                        PointCalculator::Cache c = cache_list->at(i);
                        jsonObject["caches"].push_back(c.cache_number);

                        caches_allocated[static_cast<size_t>(c.cache_number - 1)] = true;

                        if (c.handout == false)
                            warning_cache_not_in_handout = true;
                        if (c.has_coordinates == false)
                            warning_cache_not_in_gpx = true;
                    }
                }

                std::vector<PointCalculator::BestScoreHides> best_cache_numbers = point_calculator.getBestScoreHidesForTeam(team_id);
                jsonObject["hide_points_best_cache_lists"] = nlohmann::json::object();
                for (unsigned int i = 0; i < best_cache_numbers.size(); i++) {
                    jsonObject["hide_points_best_cache_lists"][std::to_string(best_cache_numbers.at(i).point_source_id)] = best_cache_numbers.at(i).cache_numbers;
                }

                jsonObject["hide_points"] = point_calculator.getTeamHideScore(best_cache_numbers);

                std::vector<int> trad_finds = point_calculator.getTeamTradFindList(team_id);
                jsonObject["trad_find_points"] = point_calculator.getTotalTradFindScore(trad_finds);
                jsonObject["trad_finds"] = nlohmann::json::array();
                for (unsigned int i = 0; i < trad_finds.size(); i++)
                    jsonObject["trad_finds"].push_back(trad_finds.at(i));

                std::vector<PointCalculator::ExtrasFind> extra_finds = point_calculator.getTeamExtrasFindList(team_id);
                jsonObject["extra_find_points"] = point_calculator.getTotalExtrasFindScore(extra_finds);
                jsonObject["extra_finds"] = nlohmann::json::object();
                for (unsigned int i = 0; i < extra_finds.size(); i++) {
                    if (extra_finds.at(i).team_id == team_id)
                        jsonObject["extra_finds"][std::to_string(extra_finds.at(i).id)] = extra_finds.at(i).value;
                }

                int not_returned_caches = point_calculator.getCachesNotReturned(team_id);
                int late = point_calculator.getMinutesLate(extra_finds);
                jsonObject["not_returned_caches"] = not_returned_caches;
                jsonObject["late"] = late;
                jsonObject["penalties"] = (not_returned_caches * CACHE_RETURN_PENALTY) + (late * MINUTES_LATE_PENALTY);

                jsonDocument["teams"].push_back(jsonObject);
            }
            delete res;
            delete stmt;

            std::vector<PointCalculator::CachePoints> * trad_points = point_calculator.getPointSourceList();
            jsonDocument["trad_points"] = nlohmann::json::array();
            for (unsigned int i = 0; i < trad_points->size(); i++) {
                nlohmann::json jsonObject;
                jsonObject["id"] = trad_points->at(i).id;
                jsonObject["item_name"] = trad_points->at(i).item_name;
                jsonObject["hide_or_find"] = trad_points->at(i).hide_or_find;
                jsonObject["points_list"] = nlohmann::json::array();
                for (unsigned int j = 0; j < trad_points->at(i).points_list.size(); j++)
                    jsonObject["points_list"].push_back(trad_points->at(i).points_list.at(j));
                jsonDocument["trad_points"].push_back(jsonObject);
            }

            std::vector<PointCalculator::ExtraItem> * extras_items = point_calculator.getExtrasItemsList();
            jsonDocument["extras_points"] = nlohmann::json::array();
            for (unsigned int i = 0; i < extras_items->size(); i++) {
                nlohmann::json jsonObject;
                jsonObject["id"] = extras_items->at(i).id;
                jsonObject["short_name"] = extras_items->at(i).item_name_short;
                jsonObject["long_name"] = extras_items->at(i).item_name_long;
                jsonObject["point_value"] = extras_items->at(i).points_value;
                jsonDocument["extras_points"].push_back(jsonObject);
            }

            jsonDocument["warning_cache_not_in_handout"] = warning_cache_not_in_handout;
            jsonDocument["warning_cache_not_in_gpx"] = warning_cache_not_in_gpx;
            jsonDocument["use_totals_for_best_cache_calculation"] = point_calculator.use_totals_for_best_cache_calculation();

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
