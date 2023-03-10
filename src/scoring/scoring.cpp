/**
  @file    scoring.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/scoring/scoring.cgi
  This page has the scoring stuff and the powerpoint builder.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/Encoder.h"
#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - Scoring", false))
            return 0;

        if (jlwe.getPermissionValue("perm_pptbuilder")) { //if logged in
            std::string message = "";

            html.outputAdminMenu();

            std::cout << FormElements::includeJavascript("/js/utils.js");
            std::cout << FormElements::includeJavascript("/js/page_tab_tools.js");

            std::cout << FormElements::includeJavascript("/js/scoring.js");

            /*std::cout << "<script>\n";

            std::cout << "function setBestCache(title){\n";
            std::cout << "    var jsonObj = {\n";
            std::cout << "        \"title\":title,\n";
            std::cout << "        \"cache_name\":document.getElementById(title + \"_textbox\").value\n";
            std::cout << "    };\n";

            std::cout << "    postUrl('set_best_cache.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "            function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, null, null);\n";
            std::cout << "         }, httpErrorResponseHandler);\n";
            std::cout << "}\n\n";

            std::cout << "</script>\n";*/


            std::cout << "<h2 style=\"text-align:center\">JLWE Scoring and Powerpoint builder</h2>\n";

            std::cout << FormElements::pageTabs({{"team_list", "Team List"}, {"team_scores", "Team Scores"}, {"ppt_builder", "Powerpoint"}});


            std::cout << "<div id=\"team_list\" class=\"pageTabContent\">\n";
            std::cout << "<h2 style=\"text-align:center\">Game Team List</h2>\n";

            std::cout << "<p>The Auto Importer will attempt to use the data from the GPX builder to create a list of teams, team members and their caches. Run this after all caches are entered into the GPX builder. After the auto import has run, you can correct any errors below (drag and drop the caches to move them to different teams).</p>\n";
            std::cout << "<p style=\"text-align:center\"><input type=\"button\" onclick=\"autoTeamImport();\" value=\"Run Auto Team Import\" ></p>\n";
            //std::cout << "<p style=\"text-align:center;font-style:italic;\">(Auto import is disabled since it relies on registration data which is currently disabled)</p>\n";

            std::cout << "<p>The team members will be shown on the Powerpoint Presentation and the caches allocated to each team are used to calculate the zone bonus points for each team. The grey rows indicate non-competing teams, these are usually the event organisers.</p>\n";


            std::cout << "<table id=\"game_teams_table\" align=\"center\"><tr>\n";
            std::cout << "<th>Team Name</th><th>Team Members</th><th>Caches Hidden</th>\n";
            std::cout << "</tr>\n";

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT team_id,team_name,team_members,competing FROM game_teams ORDER BY team_name;");
            while (res->next()){
                int team_id = res->getInt(1);
                std::cout << "<tr" + std::string(res->getInt(4) ? "" : " class=\"nonCompeteTeam\"") + ">\n";
                std::cout << "<td id=\"team_name_" + std::to_string(team_id) + "\" style=\"position:relative;\" data-value=\"" << Encoder::htmlAttributeEncode(res->getString(2)) << "\">" << Encoder::htmlEntityEncode(res->getString(2)) << "<svg class=\"iconButton\" style=\"float:right;\" onclick=\"editTeamName(" + std::to_string(team_id) + ")\" width=\"20\" height=\"20\"><image xlink:href=\"/img/edit.svg\" width=\"20\" height=\"20\"></image></svg></td>\n";
                std::cout << "<td id=\"team_members_" + std::to_string(team_id) + "\" style=\"position:relative;\" data-value=\"" << Encoder::htmlAttributeEncode(res->getString(3)) << "\">" << Encoder::htmlEntityEncode(res->getString(3)) << "<svg class=\"iconButton\" style=\"float:right;\" onclick=\"editTeamMembers(" + std::to_string(team_id) + ")\" width=\"20\" height=\"20\"><image xlink:href=\"/img/edit.svg\" width=\"20\" height=\"20\"></image></svg></td>\n";
                std::cout << "<td id=\"team_caches_" + std::to_string(team_id) + "\" ondrop=\"drop(event, this)\" ondragover=\"allowDrop(event)\" data-team-id=\"" + std::to_string(team_id) + "\"></td>\n";


                std::vector<FormElements::dropDownMenuItem> menuItems;
                menuItems.push_back({"deleteTeam(" + std::to_string(team_id) + ")", "Delete Team", true});
                menuItems.push_back({"setTeamCompeting(" + std::to_string(team_id) + ", this)", (res->getInt(4) ? "Set Non-Competing" : "Set Competing"), true});
                std::cout << "<td>" + FormElements::dropDownMenu(100 + team_id, menuItems) + "</td>\n";

                std::cout << "</tr>\n";
            }
            delete res;
            delete stmt;
            std::cout << "</table>\n";

            std::cout << "<table class=\"\" align=\"center\"><tr>\n";
            std::cout << "<td style=\"font-style:italic;\">Unallocated caches</td><td id=\"team_caches_-1\" style=\"min-width:150px;max-width:410px;\" ondrop=\"drop(event, this)\" ondragover=\"allowDrop(event)\" data-team-id=\"-1\"></td>\n";
            std::cout << "</tr></table>\n";

            std::cout << "<p style=\"text-align:center\"><input type=\"button\" onclick=\"addNewTeam();\" value=\"Create New Team\"></p>\n";


            std::cout << "<script>\n";

            std::cout << "var initalCacheAllocation = [";
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT cache_number,team_id FROM cache_handout;");
            while (res->next()){
                std::cout << "{cache:" + std::to_string(res->getInt(1)) + ",team:" + std::to_string(res->getInt(2)) + "},";
            }
            delete res;
            delete stmt;
            std::cout << "{}];\n";

            std::cout << "loadCaches(initalCacheAllocation);\n";

            std::cout << "</script>\n";

            std::cout << "</div>\n";
            std::cout << "<div id=\"team_scores\" class=\"pageTabContent\">\n";


            std::cout << "<h2 style=\"text-align:center\">Team Scores</h2>\n";
            std::cout << "<p>Copy the zone and cache return points into the Excel scoring spreadsheet. After calculating the final scores, enter then below and they will appear on the Powerpoint presentation.</p>\n";
            std::cout << "<table id=\"team_scores_table\" align=\"center\">\n";
            std::cout << "<tr><th>Team</th><th>Zone Points</th><th>Return Points</th><th>Final Score</th></tr>\n";
            std::cout << "</table>\n";

            std::cout << "</div>\n";

            std::cout << "<div id=\"ppt_builder\" class=\"pageTabContent\">\n";

            std::cout << "<p style=\"text-align:center;\">\n";
            //std::cout << "<a class=\"admin_button\" href=\"/edit_scores.html\"><span>Edit Scores</span></a>\n";
            std::cout << "<a class=\"admin_button\" href=\"download_ppt.cgi\"><span>Download Powerpoint</span></a>\n";
            std::cout << "</p>\n";

            std::cout << "<p>Proof-read the presentation after downloading! Some of the slides require further input. The order of the slides may also need adjusting depending on the number of teams.</p>\n";
            std::cout << "<p>If there is a tie for any of the podium positions or the NAGA, the presentation will not be correct! Ties elsewhere in the leaderboard may also cause issues.</p>\n";


            std::cout << "<h2 style=\"text-align:center\">Scoreboard</h2>\n";
            std::cout << "<table id=\"scoreboard_table\" align=\"center\">\n";
            std::cout << "<tr><th>Position</th><th>Score</th><th>Team Name</th><th>Team Members</th></tr>\n";
            std::cout << "</table>\n";


            std::cout << "<h2 style=\"text-align:center\">Best Caches</h2>\n";
            std::cout << "<p>Don't just write \"Number 91\", go outside, have a look at what cache 91 is and then write \"91 - Giant Anchor\". This makes the presentation a lot less confusing for the audience!</p>\n";
            std::cout << "<table align=\"center\"><tr>\n";
            std::cout << "<th>Award</th><th style=\"min-width:300px;\" >Winning Cache</th>\n";
            std::cout << "</tr>\n";

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT id,title,cache FROM best_caches ORDER BY dsp_order;");
            while (res->next()){
                int award_id = res->getInt(1);
                std::string title = res->getString(2);
                std::string award_value = res->getString(3);
                std::cout << "<tr>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(title) << "</td>\n";
                std::cout << "<td id=\"best_cache_" + std::to_string(award_id) + "\" style=\"position:relative;\" data-value=\"" << Encoder::htmlAttributeEncode(award_value) << "\">" << Encoder::htmlEntityEncode(award_value) << "<svg class=\"iconButton\" style=\"float:right;\" onclick=\"editBestCache(" + std::to_string(award_id) + ")\" width=\"20\" height=\"20\"><image xlink:href=\"/img/edit.svg\" width=\"20\" height=\"20\"></image></svg></td>\n";
                //std::cout << "<td><input type=\"text\" id=\"" << Encoder::htmlAttributeEncode(title) << "_textbox\" size=\"40\" value=\"" << Encoder::htmlAttributeEncode(res->getString(2)) << "\"></td>\n";
                //std::cout << "<td><input type=\"button\" onclick=\"setBestCache('" << Encoder::javascriptAttributeEncode(title) << "');\" value=\"Save\" /></td>\n";
                std::cout << "</tr>\n";
            }
            delete res;
            delete stmt;
            std::cout << "</table>\n";

            std::cout << "</div>\n";

            std::cout << FormElements::includeJavascript("/js/menu.js");

            std::cout << "<script type=\"text/javascript\">\n";
            std::cout << "document.getElementsByClassName(\"defaultPageTab\")[0].click();\n";

            std::cout << "document.getElementById(\"page_tab_button_team_scores\").addEventListener(\"click\", openTeamScoresTab, false);\n";
            std::cout << "document.getElementById(\"page_tab_button_ppt_builder\").addEventListener(\"click\", openPowerpointTab, false);\n";

            std::cout << "</script>\n";

        } else {
            if (jlwe.isLoggedIn()) {
                std::cout << "<p>You don't have permission to view this area.</p>";
            } else {
                std::cout << "<p>You need to be logged in to view this area.</p>";
            }
        }

        html.outputFooter();

    } catch (const sql::SQLException &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << e.what() << " (MySQL error code: " << std::to_string(e.getErrorCode()) << ")\n";
    } catch (const std::exception &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << e.what();
    }

    return 0;
}
