/**
  @file    js_files.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This outputs file URLs and data required by the javascript on the map pages

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "core/CgiEnvironment.h"
#include "core/JlweUtils.h"
#include "core/JlweCore.h"
#include "core/KeyValueParser.h"

int main () {
    try {
        JlweCore jlwe;

        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);
        std::string year = urlQueries.getValue("year");

        if (year.size() == 0)
            year = JlweUtils::getCurrentYearString();

	sql::PreparedStatement *prep_stmt;
	sql::ResultSet *res;

        std::string kml_file_current = jlwe.getGlobalVar("playing_field_kml");
        std::string osm_roads_kml = jlwe.getGlobalVar("osm_roads_kml");
        std::string map_type = jlwe.getGlobalVar("map_type");
        std::string event_caches_gpx = jlwe.getGlobalVar("event_caches_gpx");
        std::string default_coordinates = jlwe.getGlobalVar("default_coordinates");
        std::string kml_file = "";
        std::string gpx_file = "";

        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT CONCAT(directory,filename) FROM files WHERE directory = ? AND RIGHT(filename, 4) = '.kml' AND public = 1;");
        prep_stmt->setString(1, "/" + year + "/");
        res = prep_stmt->executeQuery();
        if (res->next())
            kml_file = res->getString(1);
        delete res;
        delete prep_stmt;

        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT CONCAT(directory,filename) FROM files WHERE directory = ? AND RIGHT(filename, 4) = '.gpx' AND public = 1;");
        prep_stmt->setString(1, "/" + year + "/");
        res = prep_stmt->executeQuery();
        if (res->next())
            gpx_file = res->getString(1);
        delete res;
        delete prep_stmt;

        // output header
        std::cout << "Content-type:application/javascript\r\n\r\n";

        if (kml_file_current.size()) {
            std::cout << "var kml_file_current = '" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(jlwe.config.at("files").at("urlPrefix")) + kml_file_current + "';\n";
        } else {
            std::cout << "var kml_file_current = '';\n";
        }
        if (osm_roads_kml.size()) {
            std::cout << "var osm_roads_kml = '" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(jlwe.config.at("files").at("urlPrefix")) + osm_roads_kml + "';\n";
        } else {
            std::cout << "var osm_roads_kml = '';\n";
        }
        if (kml_file.size()) {
            std::cout << "var kml_src = '" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(jlwe.config.at("files").at("urlPrefix")) + kml_file + "';\n";
        } else {
            std::cout << "var kml_src = '';\n";
        }
        if (gpx_file.size()) {
            std::cout << "var gpx_src = '" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(jlwe.config.at("files").at("urlPrefix")) + gpx_file + "';\n";
        } else {
            std::cout << "var gpx_src = '';\n";
        }
        if (event_caches_gpx.size()) {
            std::cout << "var event_caches_gpx = '" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(jlwe.config.at("files").at("urlPrefix")) + event_caches_gpx + "';\n";
        } else {
            std::cout << "var event_caches_gpx = '';\n";
	}
        std::cout << "var map_type = '" << map_type << "';\n";
        std::cout << "var default_coordinates = '" << default_coordinates << "';\n";

    } catch (sql::SQLException &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what() << " (MySQL error code: " << std::to_string(e.getErrorCode()) << ")\n";
    } catch (const std::exception &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what();
    }
    return 0;
}
