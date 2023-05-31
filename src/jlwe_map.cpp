/**
  @file    jlwe_map.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This script handles all the public map page with the game caches on it. It can be used in phone browsers to play the game.
  ModRewrite redirects requests to this script
  The content from a HTML file with a name defined below

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "core/CgiEnvironment.h"
#include "core/JlweUtils.h"

//#define MAP_HTML_FILE "/map/google.html"
#define MAP_HTML_FILE "/map/leaflet.html"

int main () {
    std::string doc_root = CgiEnvironment::getDocumentRoot();

    // output header
    std::cout << "Content-type:text/html\r\n\r\n";

    // Read html from file
    std::string filename = doc_root + std::string(MAP_HTML_FILE);
    std::string map_html = JlweUtils::readFileToString(filename.c_str());

    if (filename.size() == 0) {
        std::cout << "<html>\nFile not found on server: " << MAP_HTML_FILE << "\n</html>";
        return 0;
    }

    // Add the link to the JS variables with query string, eg. ?year=2022
    std::string file_js = "<script type=\"text/javascript\" src=\"/cgi-bin/js_files.cgi?" + CgiEnvironment::getQueryString() + "\"></script>";
    map_html = JlweUtils::replaceString(map_html, "<!--FILES-->", file_js);

    std::cout << map_html;

    return 0;
}
