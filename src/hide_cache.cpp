/**
  @file    hide_cache.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  /cgi-bin/hide_cache.cgi
  This page handles the submission of the /hide form where users can submit there cache hides online

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "core/CgiEnvironment.h"
#include "core/KeyValueParser.h"
#include "core/PostDataParser.h"
#include "core/HtmlTemplate.h"
#include "core/Encoder.h"
#include "core/JlweCore.h"

int main ()
{
    try {
        JlweCore jlwe;

        // cache data is submitted as a POST request
        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << "Content-type:text/html\r\n\r\n";

            std::cout << "<html><head>\n";
            std::cout << "<title>JLWE - error</title>\n";
            std::cout << "</head><body>\n";
            std::cout << "<h2>JLWE - error</h2>\n";

            std::cout << "<p>" << postData.errorText() << "<br/>\n";
            std::cout << "<a href=/hide.html>Try again</a></p>\n";

            std::cout << "</body></html>\n";
            return 0;
        }
        postData.parseUrlEncodedForm();

        // default response to the user
        std::string result = "<p>Something has gone wrong. Please let us know at " + std::string(jlwe.config.at("adminEmail")) + "</p>";

        // parse and validate input
        int cache_number = 0;
        try {
            cache_number = std::stoi(postData.getValue("cache_number"));
        } catch (...) {}
        double lat = 0;
        double lon = 0;
        try {
            lat = std::stod(postData.getValue("lat"));
        } catch (...) {}
        try {
            lon = std::stod(postData.getValue("lon"));
        } catch (...) {}
        int zone_points = 0;
        int osm_distance = 0;
        int actual_distance = 0;
        try {
            zone_points = std::stoi(postData.getValue("zone_points"));
        } catch (...) {}
        try {
            osm_distance = std::stoi(postData.getValue("osm_distance"));
        } catch (...) {}
        try {
            actual_distance = std::stoi(postData.getValue("actual_distance"));
        } catch (...) {}

        sql::PreparedStatement *prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT addUserCache(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
        prep_stmt->setString(1, postData.getValue("team_name"));
        prep_stmt->setString(2, postData.getValue("phone_number"));
        prep_stmt->setInt(3, cache_number);
        prep_stmt->setString(4, postData.getValue("cache_name"));
        prep_stmt->setDouble(5, lat);
        prep_stmt->setDouble(6, lon);
        prep_stmt->setString(7, postData.getValue("public_hint"));
        prep_stmt->setString(8, postData.getValue("private_hint"));
        prep_stmt->setInt(9, (postData.getValue("camo") == "true") ? 1 : 0);
        prep_stmt->setInt(10, (postData.getValue("perm") == "true") ? 1 : 0);
        prep_stmt->setInt(11, (postData.getValue("private") == "true") ? 1 : 0);
        prep_stmt->setInt(12, zone_points);
        prep_stmt->setInt(13, osm_distance);
        prep_stmt->setInt(14, actual_distance);
        prep_stmt->setString(15, jlwe.getCurrentUserIP());
        prep_stmt->setString(16, jlwe.getCurrentUsername());
        sql::ResultSet *res = prep_stmt->executeQuery();
        if (res->next()) {
            if (res->getInt(1) == 0) {
                result = "<p>Cache number " + std::to_string(cache_number) + " has been submitted successfully. If we find any problems, we will contact you on " + Encoder::htmlEntityEncode(postData.getValue("phone_number")) + "</p>";
                result += "<p><a href=\"/hide.html\">Click here to hide another cache</a></p>";
            }
        }
        delete res;
        delete prep_stmt;

        // output HTML content
        HtmlTemplate html(true);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "Hide cache form", false))
            return 0;

        std::cout << "<h1>June LWE cache hide form</h1>\n";
        std::cout << result << "\n";

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
