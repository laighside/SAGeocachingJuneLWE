/**
  @file    builder_map.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/gpx_builder/builder_map.cgi
  This page shows all the GPX Builder caches on a map

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"

int main () {
    try {
        JlweCore jlwe;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - GPX Builder", false))
            return 0;

        if (jlwe.getPermissionValue("perm_gpxbuilder")) { //if logged in
            std::string message = "";

            html.outputAdminMenu();

            std::cout << FormElements::includeJavascript("/cgi-bin/js_files.cgi");
            std::cout << FormElements::includeJavascript("/js/utils.js");
            std::cout << FormElements::includeJavascript("/js/gpx_map.js");
            std::cout << FormElements::includeJavascript("/js/maps.js");

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

            std::cout << "<h2 style=\"text-align:center\">Cache Map</h2>\n";

            std::cout << "<div id=\"map_area\" style=\"height: 500px; width: 100%; margin: 5px; position: relative; overflow: hidden;\"></div>\n";

            std::cout << "<script type=\"text/javascript\">\n";

            std::cout << "var gpx_file = '" << std::string(jlwe.config.at("http")) << std::string(jlwe.config.at("websiteDomain")) << "/cgi-bin/gpx_builder/download_gpx.cgi';\n";
            std::cout << "var map;\n";

            std::cout << "function onPageLoad() {\n";
            std::cout << "    map = loadMap('map_area', kml_file_current);\n";

            std::cout << "    // add caches (gpx)\n";
            std::cout << "    if (gpx_file.length > 0){\n";
            std::cout << "        var gpxLayer = new GPXfile(gpx_file, map, map_type);\n";
            std::cout << "    };\n";

            std::cout << "}\n";

            std::cout << "</script>\n";

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
