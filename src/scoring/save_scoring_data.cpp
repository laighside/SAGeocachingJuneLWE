/**
  @file    save_scoring_data.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/scoring/save_scoring_data.cgi
  Take the JSON data from the upload_scoring_data table and puts it in the game_find_list table.
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
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

        sql::Statement *stmt;
        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_pptbuilder")) { //if logged in

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            std::string result = JsonUtils::makeJsonError("Something has gone wrong");

            int update_id = jsonDocument.at("id");
            bool include_new_teams = jsonDocument.value("include_new_teams", false);

            int lastest_id = 0;
            time_t lastest_time = 0;
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT id, unix_timestamp(upload_time) FROM upload_scoring_data ORDER BY upload_time DESC LIMIT 1;");
            if (res->next()) {
                lastest_id = res->getInt(1);
                lastest_time = res->getInt64(2);
            }
            delete res;
            delete stmt;

            if (lastest_id > update_id) {
                result = JsonUtils::makeJsonError("There was a newer score sheet upload at " + JlweUtils::timeToW3CDTF(lastest_time) + ". If you're sure you want to use this older data, please upload the spreadsheet again.");
            } else {
                std::string json_data = "";
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT id, unix_timestamp(upload_time), json_data FROM upload_scoring_data WHERE id = ?;");
                prep_stmt->setInt(1, update_id);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    json_data = res->getString(3);
                }
                delete res;
                delete prep_stmt;

                if (json_data.size() == 0) {
                    result = JsonUtils::makeJsonError("Update data not found (invalid id number?)");
                } else {
                    nlohmann::json team_finds_list = nlohmann::json::parse(json_data)["team_finds_list"];

                    // Loop through each team
                    for (nlohmann::json::iterator it = team_finds_list.begin(); it != team_finds_list.end(); ++it) {
                        int team_id = 0;
                        if (it.value()["team_id"].is_number())
                            team_id = it.value()["team_id"];
                        std::string team_name = "";
                        if (it.value()["team_name"].is_string())
                            team_name = it.value()["team_name"];

                        if (team_id == 0) {
                            if (include_new_teams && team_name.size()) {
                                // create new team
                                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT addNewTeam(?,?,?);");
                                prep_stmt->setString(1, team_name);
                                prep_stmt->setString(2, jlwe.getCurrentUserIP());
                                prep_stmt->setString(3, jlwe.getCurrentUsername());
                                res = prep_stmt->executeQuery();
                                if (res->next() && res->getInt(1) > 0) {
                                    team_id = res->getInt(1);
                                }
                                delete res;
                                delete prep_stmt;

                            } else {
                                // ignore new team
                                continue;
                            }
                        }

                        if (team_id == 0) // something when wrong, ignore and carry on
                            continue;

                        // Save the trad finds
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setTeamFindTrad(?,?,?,?,?);");
                        prep_stmt->setInt(1, team_id);
                        prep_stmt->setString(4, jlwe.getCurrentUserIP());
                        prep_stmt->setString(5, jlwe.getCurrentUsername());
                        int cache_number = 1;
                        for (nlohmann::json::iterator it2 = it.value()["trad_finds"].begin(); it2 != it.value()["trad_finds"].end(); ++it2) {
                            if (it2.value().is_number()) {
                                int value = it2.value();
                                prep_stmt->setInt(2, cache_number);
                                prep_stmt->setInt(3, value);
                                res = prep_stmt->executeQuery();
                                delete res;
                            }
                            cache_number++;
                        }
                        delete prep_stmt;

                        // Save extras finds
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setTeamFindExtra(?,?,?,?,?);");
                        prep_stmt->setInt(1, team_id);
                        prep_stmt->setString(4, jlwe.getCurrentUserIP());
                        prep_stmt->setString(5, jlwe.getCurrentUsername());
                        for (nlohmann::json::iterator it2 = it.value()["extras_finds"].begin(); it2 != it.value()["extras_finds"].end(); ++it2) {
                            if (it2.value().is_object()) {
                                if (it2.value()["id"].is_number() && it2.value()["value"].is_number()) {
                                    int item_id = it2.value()["id"];
                                    int value = it2.value()["value"];
                                    prep_stmt->setInt(2, item_id);
                                    prep_stmt->setInt(3, value);
                                    res = prep_stmt->executeQuery();
                                    delete res;
                                }
                            }
                        }
                        delete prep_stmt;

                        // Save late penalty
                        if (it.value()["late"].is_number()) {
                            int late = it.value()["late"];
                            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setTeamFindExtra(?,?,?,?,?);");
                            prep_stmt->setInt(1, team_id);
                            prep_stmt->setInt(2, -1);
                            prep_stmt->setInt(3, late);
                            prep_stmt->setString(4, jlwe.getCurrentUserIP());
                            prep_stmt->setString(5, jlwe.getCurrentUsername());
                            res = prep_stmt->executeQuery();
                            delete res;
                            delete prep_stmt;
                        }

                        // Save final score
                        if (it.value()["total_score"].is_number()) {
                            int total_score = it.value()["total_score"];
                            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setTeamFinalScore(?,?,?,?);");
                            prep_stmt->setInt(1, team_id);
                            prep_stmt->setInt(2, total_score * 10);
                            prep_stmt->setString(3, jlwe.getCurrentUserIP());
                            prep_stmt->setString(4, jlwe.getCurrentUsername());
                            res = prep_stmt->executeQuery();
                            delete res;
                            delete prep_stmt;

                        }

                        result = JsonUtils::makeJsonSuccess("Scoring data saved");
                    }
                }
            }

            std::cout << result;
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
