/**
  @file    auto_team_import.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/scoring/auto_team_import.cgi
  Attempts to automatically create a list of game teams from the GPX builder data and registration data.
  This will overwrite any exisiting data in the game_teams table.
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/KeyValueParser.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/JsonUtils.h"
#include "../core/PostDataParser.h"

#include "../ext/nlohmann/json.hpp"

struct gpxCache {
    int number;
    std::string team_name;
};

struct team {
    int id;
    std::string name;
};

int findTeamID(const std::string &name, std::vector<team> *teamList) {
    for (unsigned int i = 0; i < teamList->size(); i++) {
        if (JlweUtils::compareTeamNames(name, teamList->at(i).name))
            return teamList->at(i).id;
    }
    return -1;
}

int main () {
    try {
        JlweCore jlwe;

        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

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

            bool confirm = jsonDocument.value("confirm", false);

            if (confirm == true) {

                // clear any existing teams
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT clearTeamList(?,?);");
                prep_stmt->setString(1, jlwe.getCurrentUserIP());
                prep_stmt->setString(2, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                //res->next()
                delete res;
                delete prep_stmt;

                std::vector<gpxCache> gpxCaches;

                stmt = jlwe.getMysqlCon()->createStatement();
                res = stmt->executeQuery("SELECT cache_number,team_name FROM caches;");
                while (res->next()){
                    gpxCaches.push_back({res->getInt(1), res->getString(2)});
                }
                delete res;
                delete stmt;

                std::vector<team> teamList;

                for (unsigned int i = 0; i < gpxCaches.size(); i++) {
                    int teamID = findTeamID(gpxCaches.at(i).team_name, &teamList);
                    if (teamID < 0) {
                        // create new team
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT addNewTeam(?,?,?);");
                        prep_stmt->setString(1, gpxCaches.at(i).team_name);
                        prep_stmt->setString(2, jlwe.getCurrentUserIP());
                        prep_stmt->setString(3, jlwe.getCurrentUsername());
                        res = prep_stmt->executeQuery();
                        if (res->next())
                            teamID = res->getInt(1);
                        delete res;
                        delete prep_stmt;

                        if (teamID < 0)
                            throw std::runtime_error("Unable to create new team in database");

                        teamList.push_back({teamID, gpxCaches.at(i).team_name});
                    }

                    // assign cache to team
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setHandoutCacheTeam(?,?,?,?);");
                    prep_stmt->setInt(1, gpxCaches.at(i).number);
                    prep_stmt->setInt(2, teamID);
                    prep_stmt->setString(3, jlwe.getCurrentUserIP());
                    prep_stmt->setString(4, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    if (!res->next())
                        throw std::runtime_error("Unable to assign cache " + std::to_string(gpxCaches.at(i).number) + " to a team");
                    delete res;
                    delete prep_stmt;

                }


                for (unsigned int i = 0; i < teamList.size(); i++) {

                    std::vector<std::string> team_members;
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT owner_name FROM cache_handout WHERE team_id = ?;");
                    prep_stmt->setInt(1, teamList.at(i).id);
                    res = prep_stmt->executeQuery();
                    while (res->next()) {
                        std::string member = res->getString(1);
                        if (member.size()) {
                            for (unsigned int j = 0; j < team_members.size(); j++) {
                                if (JlweUtils::compareTeamNames(member, team_members.at(j)))
                                    continue;
                            }
                            team_members.push_back(member);
                        }
                    }
                    delete res;
                    delete prep_stmt;

                    if (team_members.size()) {
                        std::string team_members_str = team_members.at(0);
                        for (unsigned int j = 1; j < team_members.size(); j++) {
                            team_members_str += ", " + team_members.at(j);
                        }

                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setTeamMembers(?,?,?,?);");
                        prep_stmt->setInt(1, teamList.at(i).id);
                        prep_stmt->setString(2, team_members_str);
                        prep_stmt->setString(3, jlwe.getCurrentUserIP());
                        prep_stmt->setString(4, jlwe.getCurrentUsername());
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                        }
                        delete res;
                        delete prep_stmt;
                    }

                }

                std::cout << JsonUtils::makeJsonSuccess("Team list successfully imported. This page will now refresh.");

            } else {
                std::cout << JsonUtils::makeJsonError("Please confirm that you wish to overwrite data");
            }
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
