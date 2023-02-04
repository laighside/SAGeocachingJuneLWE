/**
  @file    save_team_info.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/scoring/save_team_info.cgi
  This sets various values for game teams: team name, team members, caches, competing/non-competing, final score.
  Also allows for deleting game teams.
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

            int team_id = jsonDocument.at("team_id");

            std::string json_output = JsonUtils::makeJsonError("Invalid request");
            if (jsonDocument.contains("team_name")) {
                std::string team_name = jsonDocument.at("team_name");

                if (team_id > 0) {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setTeamName(?,?,?,?);");
                    prep_stmt->setInt(1, team_id);
                    prep_stmt->setString(2, team_name);
                    prep_stmt->setString(3, jlwe.getCurrentUserIP());
                    prep_stmt->setString(4, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    if (res->next() && res->getInt(1) == 0) {
                        json_output = JsonUtils::makeJsonSuccess("Team name updated");
                    } else {
                        json_output = JsonUtils::makeJsonError("Team not found");
                    }
                    delete res;
                    delete prep_stmt;
                } else {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT addNewTeam(?,?,?);");
                    prep_stmt->setString(1, team_name);
                    prep_stmt->setString(2, jlwe.getCurrentUserIP());
                    prep_stmt->setString(3, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    if (res->next() && res->getInt(1) > 0) {

                        nlohmann::json jsonDocumentOut;
                        jsonDocumentOut["success"] = true;
                        jsonDocumentOut["message"] = "New team created";
                        jsonDocumentOut["team_id"] = res->getInt(1);
                        json_output = JsonUtils::makeJsonHeader() + jsonDocumentOut.dump();
                    } else {
                        json_output = JsonUtils::makeJsonError("Unable to create new team in database");
                    }
                    delete res;
                    delete prep_stmt;
                }
            }
            if (jsonDocument.contains("team_members")) {
                std::string team_members = jsonDocument.at("team_members");

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setTeamMembers(?,?,?,?);");
                prep_stmt->setInt(1, team_id);
                prep_stmt->setString(2, team_members);
                prep_stmt->setString(3, jlwe.getCurrentUserIP());
                prep_stmt->setString(4, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next() && res->getInt(1) == 0) {
                    json_output = JsonUtils::makeJsonSuccess("Team members updated");
                } else {
                    json_output = JsonUtils::makeJsonError("Team not found");
                }
                delete res;
                delete prep_stmt;
            }
            if (jsonDocument.contains("cache_number")) {
                int cache_number = jsonDocument.at("cache_number");

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setHandoutCacheTeam(?,?,?,?);");
                prep_stmt->setInt(1, cache_number);
                prep_stmt->setInt(2, team_id);
                prep_stmt->setString(3, jlwe.getCurrentUserIP());
                prep_stmt->setString(4, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next() && res->getInt(1) == 0) {
                    json_output = JsonUtils::makeJsonSuccess("Cache team updated");
                } else {
                    json_output = JsonUtils::makeJsonError("Cache not found");
                }
                delete res;
                delete prep_stmt;
            }
            if (jsonDocument.contains("competing")) {
                bool competing = jsonDocument.at("competing");

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setTeamCompeting(?,?,?,?);");
                prep_stmt->setInt(1, team_id);
                prep_stmt->setInt(2, competing);
                prep_stmt->setString(3, jlwe.getCurrentUserIP());
                prep_stmt->setString(4, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next() && res->getInt(1) == 0) {
                    json_output = JsonUtils::makeJsonSuccess("Team competing updated");
                } else {
                    json_output = JsonUtils::makeJsonError("Team not found");
                }
                delete res;
                delete prep_stmt;
            }
            if (jsonDocument.contains("final_score")) {

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setTeamFinalScore(?,?,?,?);");
                prep_stmt->setInt(1, team_id);
                if (jsonDocument.at("final_score").is_null()) {
                    prep_stmt->setNull(2, 0);
                } else {
                    prep_stmt->setInt(2, jsonDocument.at("final_score"));
                }
                prep_stmt->setString(3, jlwe.getCurrentUserIP());
                prep_stmt->setString(4, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next() && res->getInt(1) == 0) {
                    json_output = JsonUtils::makeJsonSuccess("Team score updated");
                } else {
                    json_output = JsonUtils::makeJsonError("Team not found");
                }
                delete res;
                delete prep_stmt;
            }
            if (jsonDocument.contains("delete_team")) {
                if (jsonDocument.at("delete_team")) {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT deleteTeam(?,?,?);");
                    prep_stmt->setInt(1, team_id);
                    prep_stmt->setString(2, jlwe.getCurrentUserIP());
                    prep_stmt->setString(3, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    if (res->next() && res->getInt(1) == 0) {
                        json_output = JsonUtils::makeJsonSuccess("Team deleted");
                    } else {
                        json_output = JsonUtils::makeJsonError("Team not found");
                    }
                    delete res;
                    delete prep_stmt;
                }
            }

            std::cout << json_output;
        } else {
            std::cout << JsonUtils::makeJsonError("you need to be logged in to view this area");
        }
    } catch (sql::SQLException &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")");
    } catch (const std::exception &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()));
    }

    return 0;
}
