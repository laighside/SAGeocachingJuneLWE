/**
  @file    get_slides.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/scoring/get_slides.cgi
  Gets a list of powerpoint slides.
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
#include "PowerPoint.h"

#include "../ext/nlohmann/json.hpp"

nlohmann::json teamScoreToJson(PowerPoint::teamScore team_score) {
    nlohmann::json jsonObject;
    jsonObject["team_name"] = team_score.team_name;
    jsonObject["team_members"] = team_score.team_members;
    jsonObject["score"] = team_score.score;
    jsonObject["position"] = team_score.position;
    return jsonObject;
}

nlohmann::json getBestCachesJson(JlweCore *jlwe) {
    nlohmann::json jsonArray = nlohmann::json::array();
    sql::Statement *stmt = jlwe->getMysqlCon()->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT id,title,cache FROM best_caches ORDER BY dsp_order;");
    while (res->next()){
        nlohmann::json jsonObject;
        jsonObject["id"] = res->getInt(1);
        jsonObject["title"] = res->getString(2);
        jsonObject["award_value"] = res->getString(3);
        jsonArray.push_back(jsonObject);
    }
    delete res;
    delete stmt;
    return jsonArray;
}

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_pptbuilder")) { //if logged in

            // Get leaderboard positions
            std::vector<PowerPoint::teamScore> places;
            std::vector<PowerPoint::teamScore> disqualified;
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT team_name, team_members, final_score FROM game_teams WHERE competing = 1 AND final_score IS NOT NULL ORDER BY final_score DESC, team_name;");
            int i = 1;
            int previous_score = 0;
            int previous_position = i;
            while (res->next()) {
                int score = res->getInt(3);
                int position = i;
                if (score == previous_score) {
                    position = previous_position;
                } else {
                    previous_score = score;
                    previous_position = i;
                }

                PowerPoint::teamScore place = {res->getString(1).substr(0, 30), res->getString(2).substr(0, 200), score, position};
                if (score > -1000) {
                    places.push_back(place);
                } else {
                    disqualified.push_back(place);
                }

                i++;
            }
            delete res;
            delete stmt;



            nlohmann::json jsonDocument = nlohmann::json::array();

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT id, type, title, enabled, content FROM powerpoint_slides ORDER BY slide_order;");
            while (res->next()) {
                std::string slide_type = res->getString(2);
                std::string slide_content = res->getString(5);
                nlohmann::json jsonObject;
                jsonObject["id"] = res->getInt(1);
                jsonObject["type"] = slide_type;
                jsonObject["title"] = res->getString(3);
                jsonObject["enabled"] = (res->getInt(4) != 0);

                if (slide_type == "naga") {
                    jsonObject["enabled"] = (places.size() >= 3);
                    if (places.size() >= 3)
                        jsonObject["data"] = teamScoreToJson(places.at(places.size() - 1));
                }
                if (slide_type == "disqualified") {
                    nlohmann::json disqualified_teams = nlohmann::json::array();
                    for (size_t i = 0; i < disqualified.size(); i++) {
                        disqualified_teams.push_back(teamScoreToJson(disqualified.at(i)));
                    }
                    jsonObject["data"] = disqualified_teams;
                    jsonObject["enabled"] = (disqualified.size() > 0 && res->getInt(4) != 0);
                }
                if (slide_type == "winner") {
                    jsonObject["enabled"] = (places.size() >= 1);
                    if (places.size() >= 1)
                        jsonObject["data"] = teamScoreToJson(places.at(0));
                }
                if (slide_type == "runnerup") {
                    jsonObject["enabled"] = (places.size() >= 2);
                    if (places.size() >= 2)
                        jsonObject["data"] = teamScoreToJson(places.at(1));
                }
                if (slide_type == "scores") {
                    nlohmann::json scoresArray = nlohmann::json::array();
                    int startIndex = -1;
                    try {
                        startIndex = std::stoi(slide_content.substr(0, slide_content.find('-')));
                    } catch (...) {}
                    int endIndex = -1;
                    try {
                        endIndex = std::stoi(slide_content.substr(slide_content.find('-') + 1));
                    } catch (...) {}
                    if (startIndex > 0 && endIndex > 0 && startIndex <= static_cast<int>(places.size()) - 1) {
                        startIndex = std::min(startIndex, static_cast<int>(places.size()) - 1);
                        endIndex = std::min(endIndex, static_cast<int>(places.size()) - 1);
                        if (startIndex > 0 && endIndex > 0 && startIndex <= endIndex) {
                            for (int i = endIndex; i >= startIndex; i--) {
                                scoresArray.push_back(teamScoreToJson(places.at(i - 1)));
                            }
                        }
                    }
                    jsonObject["data"] = scoresArray;
                    jsonObject["enabled"] = (scoresArray.size() > 0);
                }
                if (slide_type == "best_caches") {
                    jsonObject["data"] = getBestCachesJson(&jlwe);
                }
                if (slide_type == "generic") {
                    jsonObject["data"] = slide_content;
                }

                jsonDocument.push_back(jsonObject);
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
