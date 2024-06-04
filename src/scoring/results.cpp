/**
  @file    results.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the script at /cgi-bin/scoring/results.cgi
  This page has the game results for public viewing after the winners are announced
  Apache mod_rewrite is used to direct the following URL to this script: https://jlwe.org/results

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <cmath>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/Encoder.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/KeyValueParser.h"

#include "PointCalculator.h"

std::string scoreToText(int score) {
    if (score == -1000)
        return "DSQ/DNF";
    if (score % 10 == 0)
        return std::to_string(score / 10);
    return std::to_string(score / 10) + "." + std::to_string(std::abs(score % 10));
}

int main () {
    try {
        JlweCore jlwe;

        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        sql::PreparedStatement *prep_stmt;
        sql::Statement *stmt;
        sql::ResultSet *res;

        HtmlTemplate html(true);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Game Results", false))
            return 0;

        if (jlwe.getPermissionValue("perm_pptbuilder")) { //if logged in
            if (jlwe.getGlobalVar("public_results_enabled") != "1") {
                std::cout << "<p style=\"color:red;text-align:center;font-weight:bold;\">Game results are not yet public. This page is only visable to admins.</p>\n";
            }
        } else {
            if (jlwe.getGlobalVar("public_results_enabled") != "1") {
                std::cout << "<p style=\"text-align:center;font-weight:bold;\">Game results are not yet public. Check back here after the winner is announced.</p>\n";
                html.outputFooter();
                return 0;
            }
        }

        // Check that number_game_caches is set to a valid value
        int number_game_caches = 0;
        try {
            number_game_caches = std::stoi(jlwe.getGlobalVar("number_game_caches"));
        } catch (...) {}
        if (number_game_caches < 1)
            throw std::invalid_argument("Invalid setting for number_game_caches = " + std::to_string(number_game_caches));

        int team_id = 0;
        try {
            team_id = std::stoi(urlQueries.getValue("team_id"));
        } catch (...) {}

        bool show_trad_point_list = JlweUtils::compareStringsNoCase(urlQueries.getValue("cache_list"), "true");

        std::cout << "<style>h3 {margin-top:30px;margin-bottom:10px;}</style>\n";
        std::cout << "<style>.cacheIconContainer {width:50px; text-align:center; padding:3px; border: 1px solid #C0C0C0; display: inline-block;}</style>\n";
        std::cout << "<style>.blue_background {background-color:#80dcff;}</style>\n";

        if (team_id) {
            // output point breakdown for one team

            std::string team_name = "";
            int final_score = 0;
            bool has_final_score = false;
            bool valid_team_id = false;
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT team_name, final_score FROM game_teams WHERE competing = 1 AND team_id = ?;");
            prep_stmt->setInt(1, team_id);
            res = prep_stmt->executeQuery();
            if (res->next()) {
                team_name = res->getString(1);
                if (!res->isNull(2)) {
                    has_final_score = true;
                    final_score = res->getInt(2);
                }
                valid_team_id = true;
            }
            delete res;
            delete prep_stmt;

            if (valid_team_id) {
                std::cout << "<h2 style=\"text-align:center;\">Results for " << Encoder::htmlEntityEncode(team_name) << "</h2>\n";

                if (final_score == -1000) {
                    std::cout << "<p style=\"text-align:center;font-weight:bold;\">This team was disqualified or did not finish.</p>\n";
                } else {

                    if (!has_final_score)
                        std::cout << "<p style=\"text-align:center;font-weight:bold;\">Note: there is no final score set for this team. Please see event organisers for further details.</p>\n";

                    PointCalculator point_calculator(&jlwe, number_game_caches);
                    std::vector<PointCalculator::Cache> * cache_list = point_calculator.getCacheList();
                    std::vector<int> trad_finds = point_calculator.getTeamTradFindList(team_id);

                    std::cout << "<h3 style=\"text-align:center;\">Traditional cache finds</h3>\n";
                    std::cout << "<p style=\"text-align:center;\">Caches in green were found by " << Encoder::htmlEntityEncode(team_name) << ". Bonus points for &quot;hide &amp; find&quot; items are included here (hide points are shown in blue, these caches were hidden by " << Encoder::htmlEntityEncode(team_name) << ").</p>\n";

                    //std::cout << "<table align=\"center\" class=\"grey_table_border\">\n";
                    std::cout << "<p align=\"center\" class=\"grey_table_border\">\n";

                    for (int y = 0; y < ((number_game_caches + 9) / 10); y++) {
                        //std::cout << "<tr>\n";
                        for (int x = 0; x < 10; x++) {
                            int cache_number = y * 10 + x + 1;
                            if (cache_number > number_game_caches)
                                continue;

                            if (cache_list->at(cache_number - 1).has_coordinates) {
                                int find_points = cache_list->at(cache_number - 1).total_find_points;

                                std::string background_class = "";
                                if (trad_finds.at(cache_number - 1)) {
                                    background_class = " green_background";
                                    if (cache_list->at(cache_number - 1).team_id == team_id)
                                        background_class = " blue_background";
                                }

                                /*std::cout << "<td style=\"text-align:center;\"" << (trad_finds.at(cache_number - 1) ? " class=\"green_background\"" : "") << ">\n";
                                std::cout << "<span class=\"cacheIcon\">" << cache_number << "</span><br/>\n";
                                std::cout << "<span>" << find_points << " pts</span>\n";
                                std::cout << "</td>\n";*/
                                std::cout << "<span class=\"cacheIconContainer" << background_class << "\">\n";
                                std::cout << "<span class=\"cacheIcon\">" << cache_number << "</span><br/>\n";
                                std::cout << "<span>" << find_points << " pts</span>\n";
                                std::cout << "</span>";
                            } else {
                                /*std::cout << "<td style=\"text-align:center;\" class=\"grey_background\">\n";
                                std::cout << "<span class=\"cacheIcon\">" << cache_number << "</span><br/>\n";
                                std::cout << "<span>-</span>\n";
                                std::cout << "</td>\n";*/
                                std::cout << "<span class=\"cacheIconContainer grey_background\">\n";
                                std::cout << "<span class=\"cacheIcon\">" << cache_number << "</span><br/>\n";
                                std::cout << "<span>-</span>\n";
                                std::cout << "</span>";
                            }
                        }
                        //std::cout << "</tr>\n";

                    }
                    //std::cout << "</table>\n";
                    std::cout << "</p>\n";
                    int total_find_points = point_calculator.getTotalTradFindScore(trad_finds);
                    std::cout << "<p style=\"text-align:center;\">Total traditional cache find points: " << total_find_points << " points</p>\n";

                    std::cout << "<h3 style=\"text-align:center;\">Other finds</h3>\n";
                    std::vector<PointCalculator::ExtraItem> * extras_items = point_calculator.getExtrasItemsList();
                    std::vector<PointCalculator::ExtrasFind> extra_finds = point_calculator.getTeamExtrasFindList(team_id);

                    std::cout << "<table align=\"center\" class=\"grey_table_border\">\n";
                    std::cout << "<tr><th>Item</th><th>Value</th><th>Status</th><th>Points</th></tr>\n";

                    for (unsigned int i = 0; i < extras_items->size(); i++) {
                        int found_value = 0;
                        for (unsigned int j = 0; j < extra_finds.size(); j++) {
                            if (extras_items->at(i).id == extra_finds.at(j).id) {
                                found_value = extra_finds.at(j).value;
                            }
                        }
                        std::cout << "<tr" << (found_value > 0 ? " class=\"green_background\"" : "") << "><td>" << Encoder::htmlEntityEncode(extras_items->at(i).item_name_long) << "</td>\n";
                        std::cout << "<td>" << extras_items->at(i).points_value << "</td>\n";
                        if (found_value > 0) {
                            std::cout << "<td>Found" << (found_value > 1 ? (" (x" + std::to_string(found_value) + ")") : "") << "</td>\n";
                            std::cout << "<td>" << (extras_items->at(i).points_value * found_value) << " points</td>\n";

                        } else {
                            std::cout << "<td></td><td></td>\n";
                        }
                        std::cout << "</tr>\n";
                    }
                    std::cout << "</table>\n";
                    int total_extras_points = point_calculator.getTotalExtrasFindScore(extra_finds);
                    std::cout << "<p style=\"text-align:center;\">Total other find points: " << total_extras_points << " points</p>\n";


                    std::vector<PointCalculator::CachePoints> * trad_points = point_calculator.getPointSourceList();
                    std::vector<PointCalculator::CachePoints> trad_hide_points;
                    for (unsigned int i = 0; i < trad_points->size(); i++)
                        if (trad_points->at(i).hide_or_find == "H")
                            trad_hide_points.push_back(trad_points->at(i));

                    std::cout << "<h3 style=\"text-align:center;\">Cache hide points</h3>\n";
                    std::cout << "<p style=\"text-align:center;\">This is the &quot;hide only&quot; bonus points. The caches below were hidden by " << Encoder::htmlEntityEncode(team_name) << ". Only the points for the best 2 caches are counted.</p>\n";
                    std::cout << "<table align=\"center\" class=\"grey_table_border\">\n";
                    std::cout << "<tr><th colspan=\"2\">Cache</th>\n";
                    for (unsigned int i = 0; i < trad_hide_points.size(); i++)
                        std::cout << "<th>" << Encoder::htmlEntityEncode(trad_hide_points.at(i).item_name) << "</th>\n";
                    if (point_calculator.use_totals_for_best_cache_calculation())
                        std::cout << "<th>Total</th>";
                    std::cout << "</tr>\n";

                    std::vector<PointCalculator::BestScoreHides> best_cache_numbers = point_calculator.getBestScoreHidesForTeam(team_id);
                    std::vector<std::vector<int>> best_caches_by_point_source_idx;
                    for (unsigned int j = 0; j < trad_hide_points.size(); j++) {
                        std::vector<int> best_caches;
                        for (unsigned int i = 0; i < best_cache_numbers.size(); i++) {
                            if (best_cache_numbers.at(i).point_source_id == trad_hide_points.at(j).id)
                                best_caches = best_cache_numbers.at(i).cache_numbers;
                        }
                        best_caches_by_point_source_idx.push_back(best_caches);
                    }

                    for (unsigned int i = 0; i < cache_list->size(); i++) {
                        if (cache_list->at(i).team_id == team_id) {
                            PointCalculator::Cache c = cache_list->at(i);

                            std::string cache_name = c.cache_name;
                            if (cache_name.size() > 20)
                                cache_name = cache_name.substr(0, 17) + "...";

                            std::cout << "<tr>\n";
                            std::cout << "<td style=\"border-right-style:none;\"><span class=\"cacheIcon\">" << c.cache_number << "</span></td>\n";
                            std::cout << "<td style=\"border-left-style:none;\">" << Encoder::htmlEntityEncode(cache_name) << "</td>\n";

                            for (unsigned int j = 0; j < trad_hide_points.size(); j++) {
                                bool is_best_cache = (std::find(best_caches_by_point_source_idx.at(j).begin(), best_caches_by_point_source_idx.at(j).end(), c.cache_number) != best_caches_by_point_source_idx.at(j).end());
                                std::cout << "<td" << (is_best_cache ? " class=\"green_background\"" : "") << " style=\"text-align:center;\">" << trad_hide_points.at(j).points_list.at(c.cache_number - 1) << " points</td>\n";
                            }

                            if (point_calculator.use_totals_for_best_cache_calculation()) {
                                bool is_best_cache = false;
                                if (best_caches_by_point_source_idx.size())
                                    is_best_cache = (std::find(best_caches_by_point_source_idx.at(0).begin(), best_caches_by_point_source_idx.at(0).end(), c.cache_number) != best_caches_by_point_source_idx.at(0).end());
                                std::cout << "<td" << (is_best_cache ? " class=\"green_background\"" : "") << " style=\"text-align:center;\">" << c.total_hide_points << " points</td>\n";
                            }
                            std::cout << "</tr>\n";
                        }
                    }

                    std::cout << "</table>\n";
                    int total_hide_points = point_calculator.getTeamHideScore(best_cache_numbers);
                    std::cout << "<p style=\"text-align:center;\">Total hide points: " << total_hide_points << " points</p>\n";

                    std::cout << "<h3 style=\"text-align:center;\">Penalties</h3>\n";
                    std::cout << "<table align=\"center\" class=\"grey_table_border\">\n";
                    int not_returned_caches = point_calculator.getCachesNotReturned(team_id);
                    int late = point_calculator.getMinutesLate(extra_finds);
                    std::cout << "<tr><td>Caches not returned</td><td>" << not_returned_caches << " caches</td><td>" << (not_returned_caches * CACHE_RETURN_PENALTY) << " points</td></tr>\n";
                    std::cout << "<tr><td>Late finish</td><td>" << late << " minutes</td><td>" << (late * MINUTES_LATE_PENALTY) << " points</td></tr>\n";
                    std::cout << "</table>\n";
                    int total_penalty_points = (not_returned_caches * CACHE_RETURN_PENALTY) + (late * MINUTES_LATE_PENALTY);
                    std::cout << "<p style=\"text-align:center;\">Total penalties: " << total_penalty_points << " points</p>\n";

                    int grand_total = total_find_points + total_extras_points + total_hide_points + total_penalty_points;
                    std::cout << "<p style=\"text-align:center;font-weight:bold;\">Grand total: " << grand_total << " points</p>\n";


                    if (has_final_score) {
                        if (grand_total * 10 != final_score)
                            std::cout << "<p style=\"text-align:center;\">Note: the organisers have assigned this team a different score to what the website is able to calculate. The actual score is: <span style=\"font-weight:bold;\">" << scoreToText(final_score) << " points</span>. Please see event organisers for further details.</p>\n";
                    } else {
                        std::cout << "<p style=\"text-align:center;\">Note: there is no final score set for this team. Please see event organisers for further details.</p>\n";
                    }

                }
            } else {
                std::cout << "<p style=\"text-align:center;font-weight:bold;\">Invalid team_id: " << team_id << "</p>\n";
            }

        } else if (show_trad_point_list) {
            // show the list of trad cache points

            std::cout << "<h2 style=\"text-align:center\">Traditional cache points</h2>\n";

            PointCalculator point_calculator(&jlwe, number_game_caches);
            std::vector<PointCalculator::Cache> * cache_list = point_calculator.getCacheList();
            std::vector<PointCalculator::CachePoints> * trad_points = point_calculator.getPointSourceList();
            std::vector<PointCalculator::CachePoints> trad_find_points;
            for (unsigned int i = 0; i < trad_points->size(); i++)
                if (trad_points->at(i).hide_or_find == "F")
                    trad_find_points.push_back(trad_points->at(i));

            std::cout << "<p style=\"text-align:center;\">This is the &quot;hide &amp; find&quot; points for each cache.</p>\n";
            std::cout << "<table align=\"center\" class=\"grey_table_border\">\n";
            std::cout << "<tr><th colspan=\"2\">Cache</th>\n";
            for (unsigned int i = 0; i < trad_find_points.size(); i++)
                std::cout << "<th>" << Encoder::htmlEntityEncode(trad_find_points.at(i).item_name) << "</th>\n";
            std::cout << "<th>Total</th></tr>\n";

            for (unsigned int i = 0; i < cache_list->size(); i++) {
                PointCalculator::Cache c = cache_list->at(i);

                std::string cache_name = c.cache_name;
                if (cache_name.size() > 20)
                    cache_name = cache_name.substr(0, 17) + "...";

                if (c.has_coordinates) {
                    std::cout << "<tr" << (c.has_coordinates ? "" : " class=\"grey_background\"") << ">\n";
                    std::cout << "<td style=\"border-right-style:none;\"><span class=\"cacheIcon\">" << c.cache_number << "</span></td>\n";
                    std::cout << "<td style=\"border-left-style:none;\">" << Encoder::htmlEntityEncode(cache_name) << "</td>\n";

                    for (unsigned int j = 0; j < trad_find_points.size(); j++)
                        std::cout << "<td style=\"text-align:center;\">" << trad_find_points.at(j).points_list.at(c.cache_number - 1) << (trad_find_points.at(j).points_list.at(c.cache_number - 1) == 1 ? " point" : " points") << "</td>\n";

                    std::cout << "<td style=\"text-align:center;\">" << c.total_find_points << (c.total_find_points == 1 ? " point" : " points") << "</td>\n";
                    std::cout << "</tr>\n";
                } else {
                    std::cout << "<tr class=\"grey_background\">\n";
                    std::cout << "<td style=\"border-right-style:none;\"><span class=\"cacheIcon\">" << c.cache_number << "</span></td>\n";
                    std::cout << "<td style=\"border-left-style:none;font-style:italic;\">Not hidden</td>\n";
                    for (unsigned int j = 0; j < trad_find_points.size() + 1; j++)
                        std::cout << "<td></td>\n";
                    std::cout << "</tr>\n";
                }
            }

            std::cout << "</table>\n";

        } else {
            // no team id specified so just output scoreboard

            std::cout << "<h2 style=\"text-align:center\">Final Scoreboard</h2>\n";
            std::cout << "<p style=\"text-align:center\">(click on a team name for a breakdown of their points)</p>\n";
            std::cout << "<table align=\"center\">\n";
            std::cout << "<tr><th>Position</th><th>Team Name</th><th>Points</th></tr>\n";

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT team_id, team_name, final_score FROM game_teams WHERE competing = 1 ORDER BY final_score DESC;");
            int i = 1;
            int previous_score = 0;
            int previous_position = i;
            while (res->next()){
                int team_id = res->getInt(1);
                int score = res->getInt(3);
                std::cout << "<tr>\n";
                if (score == previous_score) {
                    std::cout << "<td>" << previous_position << "</td>\n";
                } else {
                    std::cout << "<td>" << i << "</td>\n";
                    previous_score = score;
                    previous_position = i;
                }
                std::cout << "<td><a href=\"?team_id=" << team_id << "\">" << Encoder::htmlEntityEncode(res->getString(2).substr(0, 30)) << "</a></td>\n";
                std::cout << "<td>" << (res->isNull(3) ? "-" : scoreToText(score)) << "</td>\n";
                std::cout << "</tr>\n";
                i++;
            }
            delete res;
            delete stmt;
            std::cout << "</table>\n";

            std::cout << "<p style=\"text-align:center\"><a href=\"?cache_list=true\">Click here for a breakdown of the points for each cache</a></p>\n";
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
