/**
  @file    get_results.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/scoring/get_results.cgi
  Gets a list of game results for the /results page.
  GET requests, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"

#include "../ext/nlohmann/json.hpp"

#include "PointCalculator.h"

//                              green,      pink,     blue,      orange,    red,       teal,      purple,    yellow,    brown
const std::string colors[9] = {"#3B8541", "#E8B8D8", "#427AB1", "#FF9A34", "#B9262A", "#92CBC1", "#72388F", "#F8F794", "#A1843D"};

struct Team {
    int id;
    std::string name;
    int final_score;
    std::vector<int> find_list;
    std::vector<PointCalculator::ExtrasFind> extras_finds;
    int penatly_points;
    bool correct_final_score;
};

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::ResultSet *res;

        if (jlwe.getGlobalVar("public_results_enabled") == "1" || jlwe.getPermissionValue("perm_pptbuilder")) {

            // Check that number_game_caches is set to a valid value
            int number_game_caches = 0;
            try {
                number_game_caches = std::stoi(jlwe.getGlobalVar("number_game_caches"));
            } catch (...) {}
            if (number_game_caches < 1)
                throw std::invalid_argument("Invalid setting for number_game_caches = " + std::to_string(number_game_caches));

            PointCalculator point_calculator(&jlwe, number_game_caches);

            // Get list of teams and their finds
            bool has_teams_with_edited_score = false;
            std::vector<Team> team_list;
            nlohmann::json teamNames = nlohmann::json::array();
            nlohmann::json penaltiesArray = nlohmann::json::array();
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT team_id, team_name, final_score FROM game_teams WHERE competing = 1 AND final_score > -1000 ORDER BY final_score DESC;");
            while (res->next()) {
                Team t;
                t.id = res->getInt(1);
                t.name = res->getString(2);
                t.final_score = res->isNull(3) ? 0 : res->getInt(3);
                t.find_list = point_calculator.getTeamTradFindList(t.id);
                t.extras_finds = point_calculator.getTeamExtrasFindList(t.id);
                t.penatly_points = (point_calculator.getCachesNotReturned(t.id) * CACHE_RETURN_PENALTY) + (point_calculator.getMinutesLate(t.extras_finds) * MINUTES_LATE_PENALTY);
                int total_points = point_calculator.getTotalTradFindScore(t.find_list) + point_calculator.getTotalExtrasFindScore(t.extras_finds) + point_calculator.getTeamHideScore(t.id) + t.penatly_points;
                t.correct_final_score = (total_points * 10 == t.final_score);

                if (!t.correct_final_score) has_teams_with_edited_score = true;

                team_list.push_back(t);
                std::string dsp_name = (t.name.length() > 15) ? (t.name.substr(0, 13) + "...") : t.name;
                teamNames.push_back(dsp_name + (t.correct_final_score ? "" : "*"));
                penaltiesArray.push_back(t.correct_final_score ? t.penatly_points : 0);
            }
            delete res;
            delete stmt;

            // The points for trad caches
            std::vector<PointCalculator::CachePoints> *point_sources = point_calculator.getPointSourceList();
            nlohmann::json datasets = nlohmann::json::array();
            for (unsigned int i = 0; i < point_sources->size(); i++) {

                nlohmann::json scoreArray = nlohmann::json::array();
                nlohmann::json item({{"label", point_sources->at(i).item_name}});

                for (unsigned int j = 0; j < team_list.size(); j++) {
                    int total = 0;
                    if (point_sources->at(i).hide_or_find == "F") {
                        for (unsigned int k = 0; k < number_game_caches; k++)
                            total += team_list.at(j).find_list.at(k) * point_sources->at(i).points_list.at(k);

                    } else if (point_sources->at(i).hide_or_find == "H") {
                        std::vector<PointCalculator::BestScoreHides> best_caches = point_calculator.getBestScoreHidesForTeam(team_list.at(j).id);
                        for (unsigned int k = 0; k < best_caches.size(); k++) {
                            if (best_caches.at(k).point_source_id == point_sources->at(i).id) {
                                for (unsigned int m = 0; m < best_caches.at(k).cache_numbers.size(); m++) {
                                    int point_value = point_sources->at(i).points_list.at(best_caches.at(k).cache_numbers.at(m) - 1);
                                    total += point_value;
                                }
                            }
                        }

                    }
                    scoreArray.push_back(team_list.at(j).correct_final_score ? total : 0);
                }
                item["data"] = scoreArray;
                item["backgroundColor"] = colors[datasets.size() % 9];
                datasets.push_back(item);
            }

            // The points for extras
            std::vector<PointCalculator::ExtraItem> *extras_items = point_calculator.getExtrasItemsList();
            std::vector<char> active_extras_sources;
            for (unsigned int i = 0; i < extras_items->size(); i++) {
                if (std::find(active_extras_sources.begin(), active_extras_sources.end(), extras_items->at(i).type) == active_extras_sources.end()) {
                    active_extras_sources.push_back(extras_items->at(i).type);
                }
            }
            for (unsigned int i = 0; i < active_extras_sources.size(); i++) {

                std::string name(&active_extras_sources.at(i), 1);
                if (active_extras_sources.at(i) == 'P') name = "Puzzles";
                if (active_extras_sources.at(i) == 'B') name = "Black Thunder";
                if (active_extras_sources.at(i) == 'F') name = "Flash mob";
                if (active_extras_sources.at(i) == 'O') name = "Other";
                nlohmann::json scoreArray = nlohmann::json::array();
                nlohmann::json item({{"label", name}});

                for (unsigned int j = 0; j < team_list.size(); j++) {
                    int total = 0;

                    for (unsigned int k = 0; k < extras_items->size(); k++) {
                        if (active_extras_sources.at(i) == extras_items->at(k).type) {
                            for (unsigned int m = 0; m < team_list.at(j).extras_finds.size(); m++) {
                                if (extras_items->at(k).id == team_list.at(j).extras_finds.at(m).id) {
                                    total += (extras_items->at(k).points_value * team_list.at(j).extras_finds.at(m).value);
                                }
                            }
                        }
                    }
                    scoreArray.push_back(team_list.at(j).correct_final_score ? total : 0);
                }
                item["data"] = scoreArray;
                item["backgroundColor"] = colors[datasets.size() % 9];
                datasets.push_back(item);
            }

            // The penalty points
            datasets.push_back({{"label", "Penalties"}, {"data", penaltiesArray}, {"backgroundColor", "#C7C7C7"}});

            nlohmann::json chart_data;
            chart_data["datasets"] = datasets;
            chart_data["labels"] = teamNames;
            nlohmann::json jsonDocument;
            jsonDocument["chart_data"] = chart_data;
            jsonDocument["has_teams_with_edited_score"] = has_teams_with_edited_score;

            std::cout << JsonUtils::makeJsonHeader() << jsonDocument.dump();

        } else {
            std::cout << JsonUtils::makeJsonError("Game results are not yet public");
        }
    } catch (sql::SQLException &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")");
    } catch (const std::exception &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()));
    }

    return 0;
}
