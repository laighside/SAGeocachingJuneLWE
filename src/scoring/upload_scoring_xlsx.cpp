/**
  @file    upload_scoring_xlsx.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/files/upload_scoring_xlsx.cgi
  Uploads the completed scoring spreadsheet.
  POST requests only, with multipart/form-data data, return type is HTML.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/Encoder.h"
#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/PostDataParser.h"

#include "PointCalculator.h"
#include "../ext/nlohmann/json.hpp"
#include "../ext/OpenXLSX/OpenXLSX.hpp"

#define MAX_SHEET_COLS  400
#define MAX_SHEET_ROWS  200

struct ExtraItemXLSX {
    int id;
    std::string item_name;
    int points_value;
    int column_idx;
};

void add_parse_result_message(nlohmann::json * jsonObject, const std::string &message, const std::string &color) {
    nlohmann::json object;
    object["message"] = message;
    if (color.size()) {
        object["color"] = color;
    } else {
        object["color"] = nullptr;
    }
    jsonObject->push_back(object);
}

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_pptbuilder")) { //if logged in

            // Parse POST data
            PostDataParser postData(jlwe.config.at("files").at("maxUploadSize"));
            if (postData.hasError()) {
                HtmlTemplate::outputPageWithMessage(&jlwe, postData.errorText(), "JLWE - Upload spreadsheet");
                return 0;
            }

            if (postData.getFiles()->size() != 1) {
                HtmlTemplate::outputPageWithMessage(&jlwe, "Error: There are " + std::to_string(postData.getFiles()->size()) + " files. Upload must have only one file", "JLWE - Upload spreadsheet");
                return 0;
            }

            // make temp folder and filenames
            char dir_template[] = "/tmp/tmpdir.XXXXXX";
            char *dir_name = mkdtemp(dir_template);
            if (dir_name == nullptr)
                throw std::runtime_error("Unable to create temporary directory");

            std::string tmp_filename = std::string(dir_name) + "/scoring.xlsx";

            // make temp file
            FILE* temp_file = fopen(tmp_filename.c_str(), "wb");
            if (!temp_file)
                throw std::runtime_error("Unable to create temporary file");

            fwrite(postData.getFiles()->at(0).data.c_str(), 1, postData.getFiles()->at(0).data.size(), temp_file);
            fclose(temp_file);

            // Check that number_game_caches is set to a valid value
            int number_game_caches = 0;
            try {
                number_game_caches = std::stoi(jlwe.getGlobalVar("number_game_caches"));
            } catch (...) {}
            if (number_game_caches < 1)
                throw std::invalid_argument("Invalid setting for number_game_caches = " + std::to_string(number_game_caches));

            PointCalculator point_calculator(&jlwe, number_game_caches);

            std::vector<ExtraItemXLSX> allExtras = {{-1, "Hide", 1, 0}, {-2, "C Return", CACHE_RETURN_PENALTY, 0}, {-3, "Late", MINUTES_LATE_PENALTY, 0}};

            std::vector<PointCalculator::ExtraItem> * extras_items = point_calculator.getExtrasItemsList();
            //std::vector<ExtraItemXLSX> findExtras;
            for (unsigned int i = 0; i < extras_items->size(); i++)
                allExtras.push_back({extras_items->at(i).id, extras_items->at(i).item_name, extras_items->at(i).points_value, 0});

            std::vector<PointCalculator::Cache> * cache_list = point_calculator.getCacheList();

            OpenXLSX::XLDocument xlsx_doc(tmp_filename);
            OpenXLSX::XLWorksheet enter_data_sheet = xlsx_doc.workbook().worksheet("Enter Data");

            nlohmann::json resultJson = nlohmann::json::object();
            resultJson["parse_result"] = nlohmann::json::array();

            add_parse_result_message(&resultJson["parse_result"], "Reading data from XLSX file...", "");

            // Loop through all the column titles and work out what they are
            int teamNameIdx = 0;
            std::vector<int> trad_caches_idx(static_cast<size_t>(number_game_caches), 0);
            for (int i = 1; i <= MAX_SHEET_COLS; i++) {
                std::string title = enter_data_sheet.cell(1, i).value();
                bool valid_title = false;
                if (!title.size())
                    continue;

                if (JlweUtils::compareStringsNoCase(title, "Teams")) {
                    if (teamNameIdx)
                        add_parse_result_message(&resultJson["parse_result"], "Duplicate column: Teams", "red");
                    teamNameIdx = i;
                    valid_title = true;
                }

                for (int j = 1; j <= number_game_caches; j++) {
                    if (JlweUtils::compareStringsNoCase(title, "c" + std::to_string(j))) {
                        if (trad_caches_idx[j - 1])
                            add_parse_result_message(&resultJson["parse_result"], "Duplicate column: " + title, "red");
                        trad_caches_idx[j - 1] = i;
                        valid_title = true;
                    }
                }

                for (unsigned int j = 0; j < allExtras.size(); j++) {
                    if (JlweUtils::compareStringsNoCase(title, allExtras.at(j).item_name)) {
                        if (allExtras[j].column_idx)
                            add_parse_result_message(&resultJson["parse_result"], "Duplicate column: " + allExtras.at(j).item_name, "red");
                        allExtras[j].column_idx = i;
                        valid_title = true;
                    }
                }

                // known columns that we don't care about
                if (JlweUtils::compareStringsNoCase(title, "Trad Found"))
                    continue;
                if (JlweUtils::compareStringsNoCase(title, "Extras Found"))
                    continue;

                if (!valid_title)
                    add_parse_result_message(&resultJson["parse_result"], "Unknown column: " + title + " (data in this column will be ignored)", "");
            }

            // Check for any missing columns
            if (teamNameIdx) {
                add_parse_result_message(&resultJson["parse_result"], "Team names found in column " + std::to_string(teamNameIdx), "");
            } else {
                throw std::runtime_error("Team names column not found (looking for \"Teams\")");
            }
            for (int j = 1; j <= trad_caches_idx.size(); j++) {
                if (trad_caches_idx.at(j - 1) == 0)
                    add_parse_result_message(&resultJson["parse_result"], "Finds column for cache " + std::to_string(j) + " was not found", "red");
            }
            for (unsigned int j = 0; j < allExtras.size(); j++) {
                if (allExtras.at(j).column_idx == 0)
                    add_parse_result_message(&resultJson["parse_result"], "Finds column for " + allExtras.at(j).item_name + " was not found", "red");
            }

            // Work out how many rows of find data there is
            int lastRowIdx = MAX_SHEET_ROWS;
            for (int i = 2; i <= MAX_SHEET_ROWS; i++) {
                if (JlweUtils::compareStringsNoCase(enter_data_sheet.cell(i, 1).value(), "Totals")) {
                    lastRowIdx = i - 1;
                    break;
                }
                if (JlweUtils::compareStringsNoCase(enter_data_sheet.cell(i, 1).value(), "Find points")) {
                    lastRowIdx = i - 1;
                    break;
                }
            }
            add_parse_result_message(&resultJson["parse_result"], "Searching rows 2 to " + std::to_string(lastRowIdx) + " for find data", "");

            // Loop through and read each row
            nlohmann::json team_finds_list = nlohmann::json::array();
            for (int i = 2; i <= lastRowIdx; i++) {
                std::string team_name = enter_data_sheet.cell(i, teamNameIdx).value();
                bool row_has_find_data = false;

                nlohmann::json trad_find_list = nlohmann::json::array();
                for (int j = 1; j <= number_game_caches; j++)
                    trad_find_list.push_back(nullptr);

                for (int j = 1; j <= number_game_caches; j++) {
                    int col_idx = trad_caches_idx.at(j - 1);
                    int cell_value = -1;
                    if (col_idx) {
                        if (enter_data_sheet.cell(i, col_idx).value().type() == OpenXLSX::XLValueType::Integer) {
                            cell_value = enter_data_sheet.cell(i, col_idx).value();
                            row_has_find_data = true;
                        } else if (enter_data_sheet.cell(i, col_idx).value().type() == OpenXLSX::XLValueType::Empty) {
                            cell_value = 0;
                        }
                        if (cell_value < 0 || cell_value > 1) {
                            add_parse_result_message(&resultJson["parse_result"], "Invalid find value for cache " + std::to_string(j) + " by team " + (team_name.size() > 0 ? team_name : ("Row " + std::to_string(i))) + " (Traditional finds must be 0 or 1)", "red");
                        } else {
                            trad_find_list[j - 1] = cell_value;
                            if (cell_value == 1 && cache_list->at(j - 1).has_coordinates == false)
                                add_parse_result_message(&resultJson["parse_result"], "Find recorded on cache (" + std::to_string(j) + ") that is not in GPX file (this cache will have no points associated with it)", "red");
                        }
                    }
                }

                nlohmann::json extras_find_list = nlohmann::json::array();
                for (unsigned int j = 0; j < allExtras.size(); j++)
                    extras_find_list.push_back({{"id", allExtras.at(j).id}, {"item_name", allExtras.at(j).item_name}, {"value", nullptr}});

                for (unsigned int j = 0; j < allExtras.size(); j++) {
                    int col_idx = allExtras.at(j).column_idx;
                    int cell_value = -1;
                    if (col_idx) {
                        if (enter_data_sheet.cell(i, col_idx).value().type() == OpenXLSX::XLValueType::Integer) {
                            int cell_value = enter_data_sheet.cell(i, col_idx).value();
                            extras_find_list[j]["value"] = cell_value;
                            row_has_find_data = true;
                        } else if (enter_data_sheet.cell(i, col_idx).value().type() == OpenXLSX::XLValueType::Empty) {
                            extras_find_list[j]["value"] = 0;
                        } else {
                            add_parse_result_message(&resultJson["parse_result"], "Invalid find value for " + allExtras.at(j).item_name + " by team " + (team_name.size() > 0 ? team_name : ("Row " + std::to_string(i))) + " (it should be a number)", "red");
                        }
                    }
                }

                // skip empty rows
                if (team_name.size() == 0 && row_has_find_data == false)
                    continue;

                // find datra but no team name found
                if (team_name.size() == 0 && row_has_find_data == true)
                    add_parse_result_message(&resultJson["parse_result"], "Row " + std::to_string(i) + " has find data but no team name (the finds in this row will be ignored)", "red");

                // valid team name
                if (team_name.size()) {
                    nlohmann::json team_finds;
                    team_finds["team_id"] = nullptr;
                    team_finds["team_name"] = team_name;
                    team_finds["trad_finds"] = trad_find_list;
                    team_finds["extras_finds"] = extras_find_list;
                    team_finds["total_score"] = nullptr;
                    team_finds_list.push_back(team_finds);
                }
            }

            // Check team name list against database
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT team_id, team_name FROM game_teams WHERE competing = 1 ORDER BY team_name;");
            while (res->next()) {
                int team_id = res->getInt(1);
                std::string team_name = res->getString(2);
                bool found_in_spreadsheet = false;

                for (nlohmann::json::iterator it = team_finds_list.begin(); it != team_finds_list.end(); ++it) {
                    if (JlweUtils::compareStringsNoCase(team_name, it.value()["team_name"])) {
                        it.value()["team_id"] = team_id;
                        found_in_spreadsheet = true;
                    }
                }

                if (!found_in_spreadsheet)
                    add_parse_result_message(&resultJson["parse_result"], "Find data for team \"" + team_name + "\" not found in spreadsheet", "red");
            }
            delete res;
            delete stmt;

            for (nlohmann::json::iterator it = team_finds_list.begin(); it != team_finds_list.end(); ++it)
                if (it.value()["team_id"].is_null())
                    add_parse_result_message(&resultJson["parse_result"], "Team \"" + std::string(it.value()["team_name"]) + "\" not found in database, a new team will be created", "");

            // Get final scores from score calculator sheet
            OpenXLSX::XLWorksheet score_calculator_sheet = xlsx_doc.workbook().worksheet("Score Calculator");
            int scoreTeamNameIdx = 0;
            int finalScoreIdx = 0;
            for (int i = 1; i <= MAX_SHEET_COLS; i++) {
                std::string title;
                if (score_calculator_sheet.cell(1, i).value().type() == OpenXLSX::XLValueType::String)
                    title = std::string(score_calculator_sheet.cell(1, i).value());
                if (!title.size())
                    continue;
                if (JlweUtils::compareStringsNoCase(title, "Teams")) {
                    if (scoreTeamNameIdx)
                        add_parse_result_message(&resultJson["parse_result"], "Duplicate column in Score Calculator sheet: Teams", "red");
                    scoreTeamNameIdx = i;
                }
                if (JlweUtils::compareStringsNoCase(title, "Total")) {
                    finalScoreIdx = i;
                }
            }
            if (scoreTeamNameIdx == 0)
                add_parse_result_message(&resultJson["parse_result"], "Teams column not found in Score Calculator sheet", "red");
            if (finalScoreIdx == 0)
                add_parse_result_message(&resultJson["parse_result"], "Total column not found in Score Calculator sheet", "red");
            if (scoreTeamNameIdx > 0 && finalScoreIdx > 0) {
                // Loop through and read each row
                for (int i = 2; i <= lastRowIdx; i++) {
                    std::string team_name;
                    if (score_calculator_sheet.cell(i, scoreTeamNameIdx).value().type() == OpenXLSX::XLValueType::String)
                        team_name = std::string(score_calculator_sheet.cell(i, scoreTeamNameIdx).value());
                    if (!team_name.size())
                        continue;

                    if (score_calculator_sheet.cell(i, finalScoreIdx).value().type() == OpenXLSX::XLValueType::Integer) {
                        int final_score = score_calculator_sheet.cell(i, finalScoreIdx).value();

                        bool team_found = false;
                        for (nlohmann::json::iterator it = team_finds_list.begin(); it != team_finds_list.end(); ++it) {
                            if (JlweUtils::compareStringsNoCase(team_name, it.value()["team_name"])) {
                                it.value()["total_score"] = final_score;
                                team_found = true;
                            }
                        }
                        if (!team_found)
                            add_parse_result_message(&resultJson["parse_result"], "Team \"" + team_name + "\" appears in Score Calculator sheet but not Enter Data sheet (final score will be ignored)", "red");
                    } else {
                        add_parse_result_message(&resultJson["parse_result"], "Invalid total score value (Score Calculator sheet) for team " + (team_name.size() > 0 ? team_name : ("Row " + std::to_string(i))) + " (it should be a number)", "red");
                    }
                }
            }
            for (nlohmann::json::iterator it = team_finds_list.begin(); it != team_finds_list.end(); ++it)
                if (it.value()["total_score"].is_null())
                    add_parse_result_message(&resultJson["parse_result"], "Total score for team \"" + std::string(it.value()["team_name"]) + "\" not found in spreadsheet", "");

            xlsx_doc.close(); // finished reading from the file
            add_parse_result_message(&resultJson["parse_result"], "Data for " + std::to_string(team_finds_list.size()) + " teams read from spreadsheet", "");

            add_parse_result_message(&resultJson["parse_result"], "Checking score calculations...", "");

            // Extract default extra items
            for (nlohmann::json::iterator it = team_finds_list.begin(); it != team_finds_list.end(); ++it) {
                it.value()["hide_points"] = it.value()["extras_finds"][0]["value"];
                it.value()["cache_return"] = it.value()["extras_finds"][1]["value"];
                it.value()["late"] = it.value()["extras_finds"][2]["value"];
                it.value()["extras_finds"].erase(it.value()["extras_finds"].begin(), it.value()["extras_finds"].begin() + 3);
            }

            // Check the total score values and make changes table
            int new_team_count = 0;
            std::string html_table = "<table align=\"center\">\n";
            html_table += "<tr><th>Team</th><th>Traditionals</th><th>Extras</th><th>Hide Points</th><th>Penalties</th><th>Total Score</th></tr>\n";
            for (nlohmann::json::iterator it = team_finds_list.begin(); it != team_finds_list.end(); ++it) {
                int team_id = 0;
                if (it.value()["team_id"].is_number())
                    team_id = it.value()["team_id"];
                std::string team_name = "";
                if (it.value()["team_name"].is_string())
                    team_name = it.value()["team_name"];

                if (team_id == 0)
                    new_team_count++;

                html_table += "<tr>";
                html_table += "<td" + std::string((team_id > 0) ? "" : " class=\"green_background\"") + ">" + Encoder::htmlEntityEncode(team_name.size() > 20 ? (team_name.substr(0, 17) + "...") : team_name) + "</td>";

                // get trad find list/points
                std::vector<int> trad_find_list;
                int total_find = 0;
                int total_not_find = 0;
                int total_missing = 0;
                for (nlohmann::json::iterator it2 = it.value()["trad_finds"].begin(); it2 != it.value()["trad_finds"].end(); ++it2) {
                    if (!it2.value().is_number()) total_missing++;
                    if (it2.value().is_number() && it2.value() == 0) total_not_find++;
                    if (it2.value().is_number() && it2.value() > 0) {
                        trad_find_list.push_back(1);
                        total_find++;
                    } else {
                        trad_find_list.push_back(0);
                    }
                }
                int trad_points = point_calculator.getTotalTradFindScore(trad_find_list);
                html_table += "<td>" + std::to_string(total_find) + " found, " + std::to_string(total_not_find) + " not found, " + std::to_string(total_missing) + " missing</td>";

                // get extras find list/points
                std::vector<PointCalculator::ExtrasFind> extras_find_list;
                total_find = 0;
                total_not_find = 0;
                total_missing = 0;
                for (nlohmann::json::iterator it2 = it.value()["extras_finds"].begin(); it2 != it.value()["extras_finds"].end(); ++it2) {
                    if (it2.value().is_object() && it2.value()["id"].is_number() && it2.value()["value"].is_number()) {
                        int id = it2.value()["id"];
                        int value = it2.value()["value"];
                        extras_find_list.push_back({team_id, id, value});
                        if (value) {
                            total_find++;
                        } else {
                            total_not_find++;
                        }
                    } else {
                        total_missing++;
                    }
                }
                int extras_points = point_calculator.getTotalExtrasFindScore(extras_find_list);
                html_table += "<td>" + std::to_string(total_find) + " found, " + std::to_string(total_not_find) + " not found, " + std::to_string(total_missing) + " missing</td>";

                // get (and check) hide points
                int db_hide_points = 0;
                if (team_id)
                    db_hide_points = point_calculator.getTeamHideScore(team_id);
                int ss_hide_points = 0;
                if (it.value()["hide_points"].is_number())
                    ss_hide_points = it.value()["hide_points"];
                if (ss_hide_points != db_hide_points)
                    add_parse_result_message(&resultJson["parse_result"], "Hide points for team \"" + team_name + "\" (" + std::to_string(ss_hide_points) + ") does not match website (" + std::to_string(db_hide_points) + ") (changes to hide points must be done on the website)", "red");

                html_table += "<td" + std::string((ss_hide_points == db_hide_points) ? "" : " class=\"red_background\"") + ">" + std::to_string(ss_hide_points) + " points</td>";

                // get (and check) cache return
                int db_cache_return = 0;
                if (team_id)
                    db_cache_return = point_calculator.getCachesNotReturned(team_id);
                int ss_cache_return = 0;
                if (it.value()["cache_return"].is_number())
                    ss_cache_return = it.value()["cache_return"];
                if (ss_cache_return != db_cache_return)
                    add_parse_result_message(&resultJson["parse_result"], "Cache return for team \"" + team_name + "\" (" + std::to_string(ss_cache_return) + ") does not match website (" + std::to_string(db_cache_return) + ") (changes to cache return must be done on the website)", "red");

                // get minutes late
                int late = 0;
                if (it.value()["late"].is_number())
                    late = it.value()["late"];

                html_table += "<td" + std::string((ss_cache_return == db_cache_return) ? "" : " class=\"red_background\"") + ">" + (it.value()["cache_return"].is_number() ? std::to_string(ss_cache_return) : "-") + " CR, " + (it.value()["late"].is_number() ? std::to_string(late) : "-") + " late</td>";

                // calculate total score
                int total_score_calc = trad_points + extras_points + db_hide_points + (db_cache_return * CACHE_RETURN_PENALTY) + (late * MINUTES_LATE_PENALTY);
                int total_score_ss = 0;
                if (it.value()["total_score"].is_number())
                    total_score_ss = it.value()["total_score"];
                if (total_score_ss != total_score_calc)
                    add_parse_result_message(&resultJson["parse_result"], "Total score for team \"" + team_name + "\" (" + std::to_string(total_score_ss) + ") does not match calculated value (" + std::to_string(total_score_calc) + ")", "red");

                html_table += "<td" + std::string((total_score_ss == total_score_calc) ? "" : " class=\"red_background\"") + ">" + (it.value()["total_score"].is_number() ? std::to_string(total_score_ss) : "-") + " points</td>";

                html_table += "</tr>\n";
            }
            html_table += "</table>\n";

            // Save data to database
            resultJson["team_finds_list"] = team_finds_list;
            int temp_data_id = 0;
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT addUploadScoringData(?,?,?,?);");
            prep_stmt->setString(1, postData.getFiles()->at(0).filename);
            prep_stmt->setString(2, resultJson.dump());
            prep_stmt->setString(3, jlwe.getCurrentUserIP());
            prep_stmt->setString(4, jlwe.getCurrentUsername());
            res = prep_stmt->executeQuery();
            if (res->next()){
                temp_data_id = res->getInt(1);
            }
            delete res;
            delete prep_stmt;

            if (!temp_data_id)
                add_parse_result_message(&resultJson["parse_result"], "Error saving temp data to database", "red");

            rmdir(dir_name);

            // Make HTML output
            HtmlTemplate html(false);
            html.outputHttpHtmlHeader();
            if (!html.outputHeader(&jlwe, "JLWE - Upload spreadsheet", false))
                return 0;

            std::cout << "<p>Uploading <span style=\"font-weight:bold;\">" << Encoder::htmlEntityEncode(postData.getFiles()->at(0).filename) << "</span></p>";
            std::cout << "<p>";
            for (nlohmann::json::iterator it = resultJson["parse_result"].begin(); it != resultJson["parse_result"].end(); ++it) {
                if (!it.value()["color"].is_null())
                    std::cout << "<span style=\"color:" << std::string(it.value()["color"]) << ";\">";
                std::cout << Encoder::htmlEntityEncode(it.value()["message"]);
                if (!it.value()["color"].is_null())
                    std::cout<< "</span>";
                std::cout << "<br />\n";
            }
            std::cout << "</p>\n";

            std::cout << "<h3>Summary of changes</h3>\n";
            if (new_team_count)
                std::cout << "<p>Teams in green were not found in the database and can be created as new teams.</p>\n";
            std::cout << "<p>Cells in red are errors that will likely cause the final score to be incorrect.</p>\n";
            std::cout << html_table;

            std::cout << "<p>If this data all looks correct, use the buttons below to save it to the website database. This will overwrite any existing find data on the website. (depending on the number of teams, it make take up to a minute to save so be patient)</p>\n";
            if (new_team_count) {
                std::cout << "<p style=\"text-align:center\"><input type=\"button\" class=\"save_button\" onclick=\"save_scoring_data(this, " + std::to_string(temp_data_id) + ", true)\" value=\"Save data (with new teams)\"> <input type=\"button\" class=\"save_button\" onclick=\"save_scoring_data(this, " + std::to_string(temp_data_id) + ", false)\" value=\"Save data (ignore new teams)\">";
            } else {
                std::cout << "<p style=\"text-align:center\"><input type=\"button\" class=\"save_button\" onclick=\"save_scoring_data(this, " + std::to_string(temp_data_id) + ", false)\" value=\"Save data\">";
            }
            std::cout << " <input type=\"button\" class=\"save_button\" onclick=\"window.location.href='scoring.cgi';\" value=\"Cancel (discard changes)\"></p>\n";

            std::cout << FormElements::includeJavascript("/js/utils.js");

            std::cout << "<script>\n";
            std::cout << "function save_scoring_data(button, id, include_new_teams) {\n";
            std::cout << "    var buttonText = button.value;";
            std::cout << "    button.value = \"Saving...\";";
            std::cout << "    var els = document.getElementsByClassName(\"save_button\");";
            std::cout << "    Array.prototype.forEach.call(els, function(x){x.setAttribute(\"disabled\", \"true\")});";
            std::cout << "    var jsonObj = {\n";
            std::cout << "        \"id\":id,\n";
            std::cout << "        \"include_new_teams\":include_new_teams\n";
            std::cout << "    };\n";
            std::cout << "    postUrl(\"save_scoring_data.cgi\", JSON.stringify(jsonObj), null,\n";
            std::cout << "        function(data, responseCode) {\n";
            std::cout << "            httpResponseHandler(data, responseCode, false, function() {\n";
            std::cout << "                window.location.href = \"scoring.cgi\";\n";
            std::cout << "            }, function(){\n";
            std::cout << "                button.value = buttonText;";
            std::cout << "                var els = document.getElementsByClassName(\"save_button\");";
            std::cout << "                Array.prototype.forEach.call(els, function(x){x.removeAttribute(\"disabled\")});";
            std::cout << "            });\n";
            std::cout << "    }, function(statusCode, statusText) {\n";
            std::cout << "        button.value = buttonText;";
            std::cout << "        var els = document.getElementsByClassName(\"save_button\");";
            std::cout << "        Array.prototype.forEach.call(els, function(x){x.removeAttribute(\"disabled\")});";
            std::cout << "        if (statusText && typeof statusText == 'string') {\n";
            std::cout << "            alert(\"HTTP error: \" + statusText);\n";
            std::cout << "        } else {\n";
            std::cout << "            alert(\"HTTP error: Unknown\");\n";
            std::cout << "        }\n";
            std::cout << "    });\n";
            std::cout << "}\n";
            std::cout << "</script>\n";

            html.outputFooter();

        } else {
            HtmlTemplate::outputHttpHtmlHeader();
            std::cout << "<p>You need to be logged in to view this area.</p>\n";
        }
    } catch (sql::SQLException &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << "<p>Error: " << e.what() << "</p>";
    } catch (const std::exception &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << "<p>Error: " << e.what() << "</p>";
    }

    return 0;
}
