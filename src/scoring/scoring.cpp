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

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - Scoring", false))
            return 0;

        if (jlwe.getPermissionValue("perm_pptbuilder")) { //if logged in

            html.outputAdminMenu();

            std::cout << FormElements::includeJavascript("/js/utils.js");
            std::cout << FormElements::includeJavascript("/js/page_tab_tools.js");

            std::cout << FormElements::includeJavascript("/js/form_elements.js");
            std::cout << FormElements::includeJavascript("/js/scoring.js");
            std::cout << FormElements::includeJavascript("/js/scoring_team_list.js");
            std::cout << FormElements::includeJavascript("/js/scoring_team_scores.js");
            std::cout << FormElements::includeJavascript("/js/scoring_powerpoint.js");

            std::cout << "<h2 style=\"text-align:center\">JLWE Scoring and Powerpoint builder</h2>\n";

            std::cout << FormElements::pageTabs({{"team_list", "Team List"}, {"team_scores", "Team Scores"}, {"leaderboard", "Leaderboard"}, {"ppt_builder", "Powerpoint"}});

            // Team list
            std::cout << "<div id=\"team_list\" class=\"pageTabContent\">\n";
            std::cout << "<h2 style=\"text-align:center\">Game Team List</h2>\n";

            std::cout << "<p>The Auto Importer will attempt to use the data from the GPX builder to create a list of teams, team members and their caches. Run this after all caches are entered into the GPX builder. After the auto import has run, you can correct any errors below (drag and drop the caches to move them to different teams).</p>\n";
            std::cout << "<p style=\"text-align:center\"><input type=\"button\" onclick=\"autoTeamImport();\" value=\"Run Auto Team Import\" ></p>\n";
            //std::cout << "<p style=\"text-align:center;font-style:italic;\">(Auto import is disabled since it relies on registration data which is currently disabled)</p>\n";
            std::cout << "<p>The team members will be shown on the Powerpoint Presentation and the caches allocated to each team are used to calculate the zone bonus points for each team. The grey rows indicate non-competing teams, these are usually the event organisers. If you wish to remove a team from the leaderboard and powerpoint, set them as non-competing.</p>\n";

            std::cout << "<table id=\"team_list_table\" align=\"center\"><tr>\n";
            std::cout << "<th>Team Name</th><th>Team Members</th><th>Caches Hidden</th>\n";
            std::cout << "</table>\n";

            std::cout << "<table align=\"center\" style=\"margin-top:10px;\"><tr>\n";
            std::cout << "<td style=\"font-style:italic;\">Unallocated caches</td><td id=\"team_caches_-1\" style=\"min-width:150px;max-width:410px;\" ondrop=\"dropCache(event)\" ondragover=\"allowDropCache(event)\" data-team-id=\"-1\"></td>\n";
            std::cout << "</tr></table>\n";

            std::cout << "<p style=\"text-align:center\"><input type=\"button\" onclick=\"addNewTeam();\" value=\"Create New Team\"></p>\n";

            std::cout << "</div>\n";

            // Team scores
            std::cout << "<div id=\"team_scores\" class=\"pageTabContent\">\n";

            std::cout << "<h2 style=\"text-align:center\">Team Scores</h2>\n";
            std::cout << "<p>Copy the zone and cache return points into the Excel scoring spreadsheet. After calculating the final scores, enter then below and they will appear on the Powerpoint presentation.</p>\n";
            std::cout << "<p>To set a team as DSQ/DNF, enter a score of minus one hundred (-100)</p>\n";
            std::cout << "<table id=\"team_scores_table\" align=\"center\">\n";
            std::cout << "<tr><th>Team</th><th>Zone Points</th><th>Return Points</th><th>Final Score</th></tr>\n";
            std::cout << "</table>\n";

            std::cout << "<p id=\"team_scores_total_p\"></p>\n";

            std::cout << "</div>\n";

            // Leaderboard
            std::cout << "<div id=\"leaderboard\" class=\"pageTabContent\">\n";

            std::cout << "<h2 style=\"text-align:center\">Scoreboard</h2>\n";
            std::cout << "<table id=\"scoreboard_table\" align=\"center\">\n";
            std::cout << "<tr><th>Position</th><th>Score</th><th>Team Name</th><th>Team Members</th></tr>\n";
            std::cout << "</table>\n";

            std::cout << "</div>\n";

            // Powerpoint
            std::cout << "<div id=\"ppt_builder\" class=\"pageTabContent\">\n";

            std::cout << "<p style=\"text-align:center;\">\n";
            std::cout << "<a class=\"admin_button\" href=\"download_ppt.cgi\"><span>Download Powerpoint</span></a>\n";
            std::cout << "</p>\n";

            std::cout << "<p>Proof-read the presentation after downloading! Some of the slides require further input. The order of the slides may also need adjusting depending on the number of teams.</p>\n";
            std::cout << "<p>If there is a tie for any of the podium positions or the NAGA, the presentation will not be correct! Ties elsewhere in the leaderboard may also cause issues.</p>\n";

            std::cout << "<h2 style=\"text-align:center\">Powerpoint slides</h2>\n";
            std::cout << "<p>Edit the order and content of the powerpoint slides below. The slides with team scores are automatically filled in with content from the \"Team Scores\" tab.</p>\n";
            std::cout << "<p>Adding a dash \"-\" to the start of a line will indent it on the powerpoint slide. For example:</p>\n";
            std::cout << "<div style=\"width:100%;overflow:hidden;\">\n";
            std::cout << "<div style=\"float:left;width:33.33%;text-align:right;\"><p style=\"text-align:left;border: 1px solid #404040;border-radius:3px;display:inline-block;padding:10px;font-family:monospace;\">Main line<br />- Sub line</p></div>\n";
            std::cout << "<div style=\"float:left;width:33.33%;\"><p style=\"text-align:center\">will look like:</p></div>\n";
            std::cout << "<div style=\"float:left;width:33.33%;text-align:left;\"><div style=\"text-align:left;display:inline-block;\"><ul><li>Main line<ul><li>Sub line</li></ul></li></ul></div></div>\n";
            std::cout << "</div>\n";
            std::cout << "<p><span style=\"margin:5px;\">" << FormElements::htmlSwitch("slideReorderToggleCB", false) << "</span> Allow drag and drop reordering</p>\n";
            std::cout << "<div id=\"powerpoint_slides\"></div>\n";

            std::cout << "</div>\n";

            std::cout << FormElements::includeJavascript("/js/menu.js");

            std::cout << "<script type=\"text/javascript\">\n";
            std::cout << "document.getElementsByClassName(\"defaultPageTab\")[0].click();\n";

            std::cout << "document.getElementById(\"page_tab_button_team_list\").addEventListener(\"click\", openTeamListTab, false);\n";
            std::cout << "document.getElementById(\"page_tab_button_team_scores\").addEventListener(\"click\", openTeamScoresTab, false);\n";
            std::cout << "document.getElementById(\"page_tab_button_leaderboard\").addEventListener(\"click\", openLeaderboardTab, false);\n";
            std::cout << "document.getElementById(\"page_tab_button_ppt_builder\").addEventListener(\"click\", openPowerpointTab, false);\n";

            std::cout << "document.getElementById(\"slideReorderToggleCB\").addEventListener('change', slideReorderChanged);\n";

            std::cout << "openTeamListTab();\n";

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
