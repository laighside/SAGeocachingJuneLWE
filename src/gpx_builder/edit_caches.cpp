/**
  @file    edit_caches.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/gpx_builder/edit_caches.cgi
  This page allows admins to enter and edit caches. Also used for reviewing user submitted caches.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/KeyValueParser.h"

int main () {
    KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

    try {
        JlweCore jlwe;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "Edit caches - JLWE GPX Builder", false))
            return 0;

        if (jlwe.getPermissionValue("perm_gpxbuilder")) { //if logged in

            int number_game_caches = 0;
            try {
                number_game_caches = std::stoi(jlwe.getGlobalVar("number_game_caches"));
            } catch (...) {}
            if (number_game_caches < 1)
                throw std::invalid_argument("Invalid setting for number_game_caches = " + std::to_string(number_game_caches));

            std::cout << "<script type=\"text/javascript\">\n";
            int id_number = 0;
            try {
                id_number = std::stoi(urlQueries.getValue("reviewCacheId"));
                std::cout << "    var id_number = " << id_number << ";\n";
            } catch (...) {}
            bool isUserCache = (id_number > 0);

            int cache_number = 0;
            try {
                cache_number = std::stoi(urlQueries.getValue("cache"));
                std::cout << "    var cache_number = " << cache_number << ";\n";
            } catch (...) {}

            std::cout << "</script>\n";

            std::cout << FormElements::includeJavascript("/js/edit_caches.js");
            std::cout << FormElements::includeJavascript("/cgi-bin/js_files.cgi");
            std::cout << FormElements::includeJavascript("/js/utils.js");
            std::cout << FormElements::includeJavascript("/js/geo_maths.js");
            std::cout << FormElements::includeJavascript("/js/maps.js");
            std::cout << FormElements::includeJavascript("/js/ext/coordinate-parser/coordinate-number.js");
            std::cout << FormElements::includeJavascript("/js/ext/coordinate-parser/coordinates.js");
            std::cout << FormElements::includeJavascript("/js/ext/coordinate-parser/validator.js");

            std::string map_type = jlwe.getGlobalVar("map_type");
            if (map_type == "leaflet") {
                std::cout << "<!-- Leaflet JS -->\n";
                std::cout << "<link rel=\"stylesheet\" href=\"/css/leaflet.css\" />\n";
                std::cout << "<script type=\"text/javascript\" src=\"/js/ext/leaflet.js\"></script>\n";
                std::cout << "<!-- For KML files -->\n";
                std::cout << "<script src='//api.tiles.mapbox.com/mapbox.js/plugins/leaflet-omnivore/v0.3.1/leaflet-omnivore.min.js'></script>\n";
            }

            if (map_type == "google") {
                std::cout << "<!-- For Google maps -->\n";
                std::cout << "<script async defer src=\"https://maps.googleapis.com/maps/api/js?key=" + std::string(jlwe.config.value("GoogleMapsApiKey", "")) + "&callback=onPageLoad\"></script>\n";
            }

            std::cout << "<h1>Edit June LWE cache hides</h1>\n";
            if (isUserCache) {
                std::cout << "<p>Review the details below, be sure to check that the distance from road is correct.<br/>\n";
                std::cout << "If editing the coordinates, click verify to show them on the map.<br/>\n";
                std::cout << "You can contact the hider <strong id=\"hidder_name\"></strong> on <strong id=\"hidder_phone\"></strong> if you have any doubts about the infomation provided.<br/>\n";
                std::cout << "Remember to save changes once done.</p>\n";
            } else {
                std::cout << "<p>Enter the cache number first (this will load any existing data for this cache)<br/>\n";
                std::cout << "After entering/editing the coordinates, click verify to show them on the map.<br/>\n";
                std::cout << "<strong>Always make sure the cache hider sees the map and confirms the coordinates are correct!</strong><br/>\n";
                std::cout << "Remember to save changes before moving onto the next cache.</p>\n";
            }

            std::cout << "<p id=\"page_note\" style=\"color: red;\"></p>\n";
            std::cout << "<p><button onclick=\"location.href='/cgi-bin/gpx_builder/gpx_builder.cgi'\" type=\"button\">Return to cache list</button></p>\n";

            std::cout << "<div style=\"overflow:auto;\"><form>\n";
            std::cout << "    <table border=\"0\" align=\"left\" style=\"border:0\">\n";
            std::cout << "            <tr style=\"border:0\">\n";
            std::cout << "              <td><span style=\"float:right\">Cache number:</span></td>\n";
            std::cout << "              <td><input type=\"number\" id=\"cache_number\" min=\"1\" max=\"" << number_game_caches << "\" value=\"1\" onchange=\"getCacheDetails()\"> <span id=\"cache_number_note\">" << (isUserCache ? "" : "(enter this first!)") << "</span></td>\n";

            std::cout << "              <td rowspan=\"7\">\n";
            std::cout << "              <div style=\"width: 100%;display:table;\">\n";
            std::cout << "              <div style=\"display:table-cell;\">Coordinates:</div>\n";
            std::cout << "              <div style=\"display:table-cell;padding-right:5px;\"><input type=\"text\" id=\"coordinates\" style=\"width:100%;\"></div>\n";
            std::cout << "              <div style=\"display:table-cell;\"><input type=\"button\" onclick=\"updateCoords(true)\" value=\"Verify\"></div>\n";
            std::cout << "              </div>\n";
            std::cout << "              <div id=\"map_area\" style=\"height:300px;width:400px;margin:5px;\"></div>\n";
            std::cout << "              <span id=\"playing_field\"></span><br/>\n";
            if (isUserCache) {
                std::cout << "              <p style=\"line-height:2em\">Walking distance: (select the correct one)<br />\n";
                std::cout << FormElements::radioButton("walking_distance_osm", "walking_distance", "OSM distance: <span id=\"osm_distance\" style=\"font-weight:bold;\">Verify coordinates to see OSM distance</span>", "osm", "", false, false) + "<br />\n";
                std::cout << FormElements::radioButton("walking_distance_user", "walking_distance", "User submitted distance:  <span id=\"user_distance\" style=\"font-weight:bold;\"></span>", "user", "", false, false) + "<br />\n";
                std::cout << FormElements::radioButton("walking_distance_other", "walking_distance", "Other: ", "other", "", false, false);
                std::cout << "<input type=\"number\" id=\"other_distance\" min=\"0\" max=\"100000\" step=\"10\" value=\"0\" /> meters</p>\n";
            } else {
                std::cout << "              <div id=\"distance_block\" style=\"height: 120px;\"><p>Distance from nearest road (OSM data): <span id=\"osm_distance\" style=\"font-weight:bold;\"></span></p>\n";
                std::cout << "              <p>Is this distance correct? ";
                std::cout << FormElements::radioButton("distance_correct_yes", "distance_correct", "Yes", "yes", "distanceCorrectChanged()", false, false) << "&nbsp;&nbsp;&nbsp;";
                std::cout << FormElements::radioButton("distance_correct_no", "distance_correct", "No", "no", "distanceCorrectChanged()", false, false);
                std::cout << "              </p>\n";
                std::cout << "              <p id=\"actual_distance_block\" style=\"display: none;\">Enter the actual distance from the road: <input type=\"number\" id=\"actual_distance\" min=\"0\" max=\"100000\" step=\"10\" value=\"0\">m</p></div>\n";
            }
            std::cout << "              </td>\n";
            std::cout << "            </tr>\n";
            std::cout << "            <tr>\n";
            std::cout << "              <td><span style=\"float:right\">Cache name:</span></td>\n";
            std::cout << "              <td><input type=\"text\" id=\"cache_name\" style=\"width:100%;\"></td>\n";
            std::cout << "            </tr>\n";
            std::cout << "            <tr>\n";
            std::cout << "              <td><span style=\"float:right\">Placed by:</span></td>\n";
            std::cout << "              <td><input type=\"text\" id=\"team_name\" style=\"width:100%;\"></td>\n";
            std::cout << "            </tr>\n";
            std::cout << "            <tr>\n";
            std::cout << "              <td><span style=\"float:right\">Public hint:</span></td>\n";
            std::cout << "              <td><textarea id=\"public_hint\" rows=\"3\" cols=\"35\" placeholder=\"Enter hint\"></textarea></td>\n";
            std::cout << "            </tr>\n";
            std::cout << "            <tr>\n";
            std::cout << "              <td><span style=\"float:right\">Detailed hint:</span></td>\n";
            std::cout << "              <td><textarea id=\"detailed_hint\" rows=\"3\" cols=\"35\" placeholder=\"Enter hint\"></textarea></td>\n";
            std::cout << "            </tr>\n";
            std::cout << "            <tr>\n";
            std::cout << "              <td><span style=\"float:right\">Attributes:</span></td>\n";
            std::cout << "              <td>\n";
            std::cout << "              <div style=\"line-height: 2em;\">\n";
            std::cout << "              <span class=\"checkbox_container\"><label>Creative/camouflaged hide\n";
            std::cout << "                <input type=\"checkbox\" id=\"camo\" value=\"true\" />\n";
            std::cout << "                <span class=\"checkmark\"></span>\n";
            std::cout << "              </label></span><br />\n";
            std::cout << "              <span class=\"checkbox_container\"><label>Will become a permanent cache\n";
            std::cout << "                <input type=\"checkbox\" id=\"permanent\" value=\"true\" />\n";
            std::cout << "                <span class=\"checkmark\"></span>\n";
            std::cout << "              </label></span><br />\n";
            std::cout << "              <span class=\"checkbox_container\"><label>Cache is on private property (with permission)\n";
            std::cout << "                <input type=\"checkbox\" id=\"private\" value=\"true\" />\n";
            std::cout << "                <span class=\"checkmark\"></span>\n";
            std::cout << "              </label></span><br />\n";
            std::cout << "              </div>\n";
            std::cout << "              <p><span style=\"font-style: italic;\">If unsure, leave boxes un-checked</span></p>\n";
            std::cout << "              </td>\n";
            std::cout << "            </tr>\n";
            std::cout << "            <tr>\n";
            std::cout << "              <td><span style=\"float:right\">Photos:</span></td>\n";
            std::cout << "              <td id=\"photo_cell\">\n";
            std::cout << "              </td>\n";
            std::cout << "            </tr>\n";
            std::cout << "            <tr>\n";
            std::cout << "              <td colspan=\"3\" align=\"center\">" << (isUserCache ? "<input type=\"button\" onclick=\"deleteUserCache()\" value=\"Delete cache\"> " : "") << "<input type=\"button\" id=\"save_button\" onclick=\"setCacheDetails()\" value=\"Save changes\"></td>\n";
            std::cout << "            </tr>\n";
            std::cout << "    </table>\n";
            std::cout << "</form></div>\n";

            std::cout << "<p><span style=\"font-weight: bold;\">Walking caches:</span> The automatic road distance calculation uses the OSM map which doesn't know about every single road so this distance may be incorrect. Also note that the cache hider may not be aware of the shortest route to access their cache. All caches that have walking points attached to them need to be double checked by organisers (satellite images can help with this).</p>\n";
            std::cout << "<p><span style=\"font-weight: bold;\">Hide bonus points:</span> For teams with more than 2 hides, the software will automatically choose the 2 caches that give the most points. Please <span style=\"font-weight: bold;\">do not</span> add \"Bonus\" (or similar) to the placed by names to mark the caches teams would like to take there hide bonus from. If the placed by name is different, the caches (and points) will get allocated to a different team.</p>\n";

            if (map_type == "leaflet") {
                std::cout << "<script>\n";
                std::cout << "onPageLoad()\n";
                std::cout << "</script>\n";
            }

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
