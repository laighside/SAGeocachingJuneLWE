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

            std::cout << "<h2 style=\"text-align:center\">Cache Map</h2>\n";

            std::cout << "<div id=\"map\" style=\"height: 500px; width: 100%; margin: 5px; position: relative; overflow: hidden;\"></div>\n";

            std::cout << "<script type=\"text/javascript\">\n";

            std::cout << "var gpx_file = '" << std::string(jlwe.config.at("http")) << std::string(jlwe.config.at("websiteDomain")) << "/cgi-bin/gpx_builder/download_gpx.cgi';\n";

            std::cout << "function initMap() {\n";

            std::cout << "    //load man google map\n";
            std::cout << "    map = new google.maps.Map(document.getElementById('map'), {\n";
            std::cout << "        center: {lat: -34.88, lng: 138.58},\n";
            std::cout << "        zoom: 10\n";
            std::cout << "    });\n";

            /*std::cout << "    //add osm roads (kml)\n";
            std::cout << "    if (osm_roads_kml.length > 0){\n";
            std::cout << "        var kmlRoadsLayer = new google.maps.KmlLayer(osm_roads_kml, {\n";
            std::cout << "            map: map\n";
            std::cout << "        });\n";
            std::cout << "    };\n";*/

            std::cout << "    //add caches (gpx)\n";
            std::cout << "    if (gpx_file.length > 0){\n";
            std::cout << "        var gpxLayer = new GPXfile(gpx_file, map);\n";
            std::cout << "    };\n";

            std::cout << "    //add game zone outline (kml)\n";
            std::cout << "    if (kml_file_current.length > 0){\n";
            std::cout << "        var kmlLayer = new google.maps.KmlLayer(kml_file_current, {\n";
            std::cout << "            map: map\n";
            std::cout << "        });\n";
            std::cout << "    };\n";

            std::cout << "}\n";

            std::cout << "        </script>\n";

            std::cout << "<script async defer src=\"https://maps.googleapis.com/maps/api/js?key=" + std::string(jlwe.config.value("GoogleMapsApiKey", "")) + "&callback=initMap\"></script>\n";


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
