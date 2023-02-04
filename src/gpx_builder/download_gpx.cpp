/**
  @file    download_gpx.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the download at /cgi-bin/gpx_builder/download_gpx.cgi
  Creates the GPX file with all the GPX Builder caches in it
  This is NOT the public gpx download, organisers download this one, edit it with GSAK as necessary then upload it for the public

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <ctime>
#include <string>
#include <vector>

#include "../core/Encoder.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweUtils.h"
#include "../core/JlweCore.h"

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_gpxbuilder")) { //if logged in
            int year = JlweUtils::getCurrentYear();
            std::string currentTime = JlweUtils::timeToW3CDTF(time(nullptr));

            double min_lat = 0;
            double max_lat = 0;
            double min_lon = 0;
            double max_lon = 0;
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT MIN(latitude),MAX(latitude),MIN(longitude),MAX(longitude) FROM caches;");
            while (res->next()) {
                min_lat = res->getDouble(1);
                max_lat = res->getDouble(2);
                min_lon = res->getDouble(3);
                max_lon = res->getDouble(4);
            }
            delete res;
            delete stmt;

            std::string code_prefix = jlwe.getGlobalVar("gpx_code_prefix");
            std::string gpx_state = jlwe.getGlobalVar("gpx_state");
            std::string gpx_country = jlwe.getGlobalVar("gpx_country");

            // output header
            std::cout << "Content-type:application/gpx+xml\r\n";
            std::cout << "Content-Disposition: attachment; filename=jlwe_" << year << ".gpx\r\n\r\n";

            std::cout << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
            std::cout << "<gpx xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n";
            std::cout << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n";
            std::cout << "version=\"1.0\" creator=\"" << Encoder::htmlAttributeEncode(jlwe.config.at("websiteDomain")) << "\"\n";
            std::cout << "xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd http://www.groundspeak.com/cache/1/0/1 http://www.groundspeak.com/cache/1/0/1/cache.xsd\"\n";
            std::cout << "xmlns=\"http://www.topografix.com/GPX/1/0\">\n";
            std::cout << " <desc>June LWE Geocache file</desc>\n";
            std::cout << " <author>" << Encoder::htmlEntityEncode(jlwe.config.at("websiteDomain")) << "</author>\n";
            std::cout << " <email>" << Encoder::htmlEntityEncode(jlwe.config.at("adminEmail")) << "</email>\n";
            std::cout << " <time>" << Encoder::htmlEntityEncode(currentTime) << "</time>\n";
            std::cout << " <keywords>cache, geocache, jlwe</keywords>\n";
            std::cout << " <bounds minlat=\"" << min_lat << "\" minlon=\"" << min_lon << "\" maxlat=\"" << max_lat << "\" maxlon=\"" << max_lon << "\"/>\n";

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT cache_number,cache_name,team_name,latitude,longitude,public_hint,camo,permanent,private_property FROM caches;");
            while (res->next()){
                std::string cache_number = res->getString(1);
                if (cache_number.size() == 1)
                    cache_number = "0" + cache_number;
                std::string cache_name = res->getString(2);
                std::string cache_name_full = JlweUtils::makeFullCacheName(res->getInt(1), res->getString(2), res->getInt(7), res->getInt(8));
                std::string team_name = res->getString(3);
                std::string cache_code = code_prefix + cache_number; //GC code
                if (cache_name.size() == 0)
                    cache_name = cache_code;

                // put camo and permanent status in description field
                std::string desc = "<p>";
                if (res->getInt(7)) {
                    desc = desc + "Creative/camouflaged hide: Yes<br/>";
                } else {
                    //desc = desc + "Creative/camouflaged hide: No<br/>";
                }
                if (res->getInt(8)) {
                    desc = desc + "Permanent cache: Yes<br/>";
                } else {
                    //desc = desc + "Permanent cache: No<br/>";
                }
                if (res->getInt(9)) {
                    desc = desc + "Front yard/private property cache: Yes<br/>";
                } else {
                    //desc = desc + "Front yard/private property cache: No<br/>";
                }
                desc = desc + "</p>";

                std::cout << " <wpt lat=\"" << res->getString(4) << "\" lon=\"" << res->getString(5) << "\">\n";
                std::cout << " <time>" << Encoder::htmlEntityEncode(currentTime) << "</time>\n";
                std::cout << " <name>" << Encoder::htmlEntityEncode(cache_code) << "</name>\n";
                std::cout << " <desc>" << Encoder::htmlEntityEncode(code_prefix + cache_name_full + " by " + team_name) << "</desc>\n";
                std::cout << " <sym>Geocache</sym>\n";
                std::cout << " <type>Geocache|Traditional Cache</type>\n";
                std::cout << " <groundspeak:cache id=\"" << cache_number << "\" available=\"True\" archived=\"False\" xmlns:groundspeak=\"http://www.groundspeak.com/cache/1/0/1\">\n";
                std::cout << "  <groundspeak:name>" << Encoder::htmlEntityEncode(cache_name_full) << "</groundspeak:name>\n";
                std::cout << "  <groundspeak:placed_by>" << Encoder::htmlEntityEncode(team_name) << "</groundspeak:placed_by>\n";
                std::cout << "  <groundspeak:owner id=\"0\">" << Encoder::htmlEntityEncode(team_name) << "</groundspeak:owner>\n";
                std::cout << "  <groundspeak:type>Traditional Cache</groundspeak:type>\n";
                std::cout << "  <groundspeak:container>Regular</groundspeak:container>\n";
                std::cout << "  <groundspeak:attributes></groundspeak:attributes>\n";
                std::cout << "  <groundspeak:difficulty>1</groundspeak:difficulty>\n";
                std::cout << "  <groundspeak:terrain>1</groundspeak:terrain>\n";
                std::cout << "  <groundspeak:country>" << Encoder::htmlEntityEncode(gpx_country) << "</groundspeak:country>\n";
                std::cout << "  <groundspeak:state>" << Encoder::htmlEntityEncode(gpx_state) << "</groundspeak:state>\n";
                std::cout << "  <groundspeak:short_description html=\"False\"></groundspeak:short_description>\n";
                std::cout << "  <groundspeak:long_description html=\"True\">" << Encoder::htmlEntityEncode(desc) << "</groundspeak:long_description>\n";
                std::cout << "  <groundspeak:encoded_hints>" << Encoder::htmlEntityEncode(res->getString(6)) << "</groundspeak:encoded_hints>\n";
                std::cout << "  <groundspeak:logs></groundspeak:logs>\n";
                std::cout << "  <groundspeak:travelbugs></groundspeak:travelbugs>\n";
                std::cout << " </groundspeak:cache>\n";
                std::cout << " </wpt>\n";
            }
            delete res;
            delete stmt;

            std::cout << "</gpx>\n";

        } else { // not logged in
            if (jlwe.isLoggedIn()) {
                HtmlTemplate::outputPageWithMessage(&jlwe, "You don't have permission to view this area.", "JLWE Admin area");
            } else {
                HtmlTemplate::outputPageWithMessage(&jlwe, "You need to be logged in to view this area.", "JLWE Admin area");
            }
        }

    } catch (const sql::SQLException &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << e.what() << " (MySQL error code: " << std::to_string(e.getErrorCode()) << ")\n";
    } catch (const std::exception &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << e.what();
    }

    return 0;
}
