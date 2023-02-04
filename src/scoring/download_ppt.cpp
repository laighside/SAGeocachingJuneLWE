/**
  @file    download_ppt.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/scoring/download_ppt.cgi
  This creates the PowerPoint file.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"

#include "PowerPoint.h"

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_pptbuilder")) { //if logged in

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

            int placesCount = static_cast<int>(places.size()) - 1; // -1 since don't include NAGA award
            if (placesCount < 2)
                throw std::invalid_argument("There are " + std::to_string(placesCount + 1) + " teams on the scoreboard. At least 3 team scores are required to build the PowerPoint.");

            PowerPoint ppt;

            // First slide is the title slide
            ppt.addJlweTitleSlide(CgiEnvironment::getDocumentRoot() + "/img/jlwe_logo.png", JlweUtils::getCurrentYearString(), jlwe.getGlobalVar("ppt_town"));

            // NAGA slide goes first
            PowerPoint::teamScore lastPlace = places.at(places.size() - 1);
            ppt.addJlweSinglePlaceSlide(lastPlace, "NAGA", JlweUtils::numberToOrdinal(lastPlace.position) + " Place", {3,4});

            if (placesCount > 30) {
                std::vector<PowerPoint::teamScore> slidePlaces = {places.begin() + 30, places.begin() + std::min(static_cast<unsigned long>(35), places.size() - 1)};
                ppt.addJlwePlacesSlide(slidePlaces);
	    }

            if (disqualified.size())
                ppt.addJlweDisqualifiedSlide(disqualified);

            if (placesCount > 25) {
                std::vector<PowerPoint::teamScore> slidePlaces = {places.begin() + 25, places.begin() + std::min(static_cast<unsigned long>(30), places.size() - 1)};
                ppt.addJlwePlacesSlide(slidePlaces);
	    }

            ppt.addGenericSlide("Bingo/Scavenger Hunt", "(write names of winners here)");

	    if (placesCount > 20) {
                std::vector<PowerPoint::teamScore> slidePlaces = {places.begin() + 20, places.begin() + std::min(static_cast<unsigned long>(25), places.size() - 1)};
                ppt.addJlwePlacesSlide(slidePlaces);
	    }

            ppt.addGenericSlide("Freddo's cache", "");

	    if (placesCount > 15) {
                std::vector<PowerPoint::teamScore> slidePlaces = {places.begin() + 15, places.begin() + std::min(static_cast<unsigned long>(20), places.size() - 1)};
                ppt.addJlwePlacesSlide(slidePlaces);
            }

            ppt.addJlweOtherPrizesSlide();

            ppt.addGenericSlide("Everyone Stand Up...", "");

	    if (placesCount > 10) {
                std::vector<PowerPoint::teamScore> slidePlaces = {places.begin() + 10, places.begin() + std::min(static_cast<unsigned long>(15), places.size() - 1)};
                ppt.addJlwePlacesSlide(slidePlaces);
	    }

            ppt.addJlweRisingStarSlide(CgiEnvironment::getDocumentRoot() + "/img/carto_graphics_logo.png");

            if (placesCount > 5) {
                std::vector<PowerPoint::teamScore> slidePlaces = {places.begin() + 5, places.begin() + std::min(static_cast<unsigned long>(10), places.size() - 1)};
                ppt.addJlwePlacesSlide(slidePlaces);
            }

            std::vector<PowerPoint::bestCache> caches;
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT title,cache FROM best_caches ORDER BY dsp_order;");
            while (res->next()) {
                caches.push_back({res->getString(1), res->getString(2)});
            }
            delete res;
            delete stmt;
            ppt.addJlweBestCachesSlide(caches);

            if (placesCount > 3) {
                std::vector<PowerPoint::teamScore> slidePlaces = {places.begin() + 2, places.begin() + std::min(static_cast<unsigned long>(5), places.size() - 1)};
                ppt.addJlwePlacesSlide(slidePlaces);
	    }

            ppt.addGenericSlide("Good Samaritans", "(write names of good samaritans here)");

            // Slides for the winner and runner-up
            if (placesCount > 1) {
                ppt.addJlweSinglePlaceSlide(places.at(0), "The winner is...", "With " + PowerPoint::scoreToString(places.at(0).score) + " points...", {0,0,3,4});
                ppt.addJlweSinglePlaceSlide(places.at(1), "Runner Up", PowerPoint::scoreToString(places.at(1).score) + " points", {3,4});
	    }

            // Full leaderboard slide at the end, which includes the disqualified teams as well
            places.insert(places.end(), disqualified.begin(), disqualified.end());
            ppt.addJlweLeaderboardSlide(places);


            std::string ppt_file = ppt.savePowerPointFile();

            FILE *file = fopen(ppt_file.c_str(), "rb");
            if (file) { //if file exists in filesystem
                //output header
                std::cout << "Content-type:application/vnd.openxmlformats-officedocument.presentationml.presentation\r\n";
                std::cout << "Content-Disposition: attachment; filename=jlwe_ppt_" << JlweUtils::getCurrentYearString() << ".pptx\r\n\r\n";

                char buffer[1024];
                size_t size = 1024;
                while (size == 1024) {
                    size = fread(buffer, 1, 1024, file);
                    std::cout.write(buffer, static_cast<long>(size));
                }
                fclose(file);
            } else {
                std::cout << "Content-type:text/plain\r\n\r\n";
                std::cout << "Error: unable to read ppt file\n";
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
