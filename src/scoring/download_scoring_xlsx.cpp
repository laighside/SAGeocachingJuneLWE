/**
  @file    download_scoring_xlsx.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/scoring/download_scoring_xlsx.cgi
  This creates the scoring spreadsheet file.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"

#include "WriteScoringXLSX.h"
#include "PointCalculator.h"

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_pptbuilder")) { //if logged in

            // Check that number_game_caches is set to a valid value
            int number_game_caches = 0;
            try {
                number_game_caches = std::stoi(jlwe.getGlobalVar("number_game_caches"));
            } catch (...) {}
            if (number_game_caches < 1)
                throw std::invalid_argument("Invalid setting for number_game_caches = " + std::to_string(number_game_caches));

            PointCalculator point_calculator(&jlwe, number_game_caches);

            std::vector<WriteScoringXLSX::ExtraItem> defaultExtras = {{"Hide", 1}, {"C Return", CACHE_RETURN_PENALTY}, {"Late", MINUTES_LATE_PENALTY}};

            std::vector<PointCalculator::ExtraItem> * extras_items = point_calculator.getExtrasItemsList();
            std::vector<WriteScoringXLSX::ExtraItem> findExtras;
            for (unsigned int i = 0; i < extras_items->size(); i++) {
                findExtras.push_back({extras_items->at(i).item_name_short, extras_items->at(i).points_value});
            }

            WriteScoringXLSX xlsx(jlwe.config.at("ooxmlTemplatePath"), number_game_caches, findExtras, defaultExtras);

            // Get the list of teams and their finds
            std::vector<WriteScoringXLSX::TeamFinds> team_list;
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT team_id, team_name FROM game_teams WHERE competing = 1 ORDER BY team_name;");
            while (res->next()) {
                int team_id = res->getInt(1);
                std::vector<int> trad_finds = point_calculator.getTeamTradFindList(team_id);
                std::vector<PointCalculator::ExtrasFind> extra_finds = point_calculator.getTeamExtrasFindList(team_id);
                int hide_score = point_calculator.getTeamHideScore(team_id);
                int caches_not_returned = point_calculator.getCachesNotReturned(team_id);
                int late = point_calculator.getMinutesLate(extra_finds);

                std::vector<int> extra_finds_in_order;
                for (unsigned int i = 0; i < extras_items->size(); i++) {
                    int value = 0;
                    for (unsigned int j = 0; j < extra_finds.size(); j++)
                        if (extras_items->at(i).id == extra_finds.at(j).id)
                            value = extra_finds.at(j).value;
                    extra_finds_in_order.push_back(value);
                }

                team_list.push_back({res->getString(2), trad_finds, extra_finds_in_order, {hide_score, caches_not_returned, late}});
            }
            delete res;
            delete stmt;

            // Get list of point sources
            std::vector<PointCalculator::CachePoints> * trad_points = point_calculator.getPointSourceList();
            std::vector<WriteScoringXLSX::CachePoints> find_points;
            std::vector<WriteScoringXLSX::CachePoints> hide_points;
            for (unsigned int i = 0; i < trad_points->size(); i++) {
                if (trad_points->at(i).hide_or_find == "H")
                    hide_points.push_back({trad_points->at(i).id, trad_points->at(i).item_name, trad_points->at(i).points_list});
                if (trad_points->at(i).hide_or_find == "F")
                    find_points.push_back({trad_points->at(i).id, trad_points->at(i).item_name, trad_points->at(i).points_list});
            }

            // Create the spreadsheets
            xlsx.addEnterDataSheet(team_list, find_points.size() + hide_points.size());
            xlsx.addPointValuesSheet(find_points, hide_points);
            xlsx.addScoreCalculatorSheet();

            // Save the file
            std::string xlsx_file = xlsx.saveScoringXlsxFile(jlwe.config.at("websiteDomain"));

            FILE *file = fopen(xlsx_file.c_str(), "rb");
            if (file) { //if file exists in filesystem
                //output header
                std::cout << "Content-type:application/vnd.openxmlformats-officedocument.spreadsheetml.sheet\r\n";
                std::cout << "Content-Disposition: attachment; filename=jlwe_scoring_" << JlweUtils::getCurrentYearString() << ".xlsx\r\n\r\n";

                JlweUtils::readFileToOStream(file, std::cout);
                fclose(file);
            } else {
                std::cout << "Content-type:text/plain\r\n\r\n";
                std::cout << "Error: unable to read temp xlsx file\n";
            }

        } else {
            std::cout << "Content-type:text/plain\r\n\r\n";
            std::cout << "You need to be logged in to view this area.\n";
        }
    } catch (const sql::SQLException &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what() << " (MySQL error code: " << std::to_string(e.getErrorCode()) << ")\n";
    } catch (const std::exception &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what();
    }

    return 0;
}
