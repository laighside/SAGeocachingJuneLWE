/**
  @file    download_event_registrations.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the download at /cgi-bin/registration/download_event_registrations.cgi
  Downloads the Excel file that contains the list of registrations, camping and dinner orders

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/KeyValueParser.h"

#include "WriteRegistrationXLSX.h"

int main () {
    try {
        JlweCore jlwe;

        if (jlwe.getPermissionValue("perm_registrations")) { //if logged in

            KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);
            bool full = !(urlQueries.getValue("simple") == "true");

            std::vector<WriteRegistrationXLSX::cacheLog> cache_logs;
            std::string events_gpx = jlwe.getGlobalVar("event_caches_gpx");
            if (events_gpx.size())
                cache_logs = WriteRegistrationXLSX::readLogsFromGPXfile(std::string(jlwe.config.at("files").at("directory")) + events_gpx);

            time_t jlwe_date = 0;
            try {
                jlwe_date = std::stoll(jlwe.getGlobalVar("jlwe_date"));
            } catch (...) {}

            WriteRegistrationXLSX xlsx(jlwe.config.at("ooxmlTemplatePath"));

            xlsx.addEventRegistrationsSheet(jlwe.getMysqlCon(), full, cache_logs);
            xlsx.addCampingSheet(jlwe.getMysqlCon(), full, jlwe_date);
            xlsx.addDinnerSheet(jlwe.getMysqlCon(), full);

            // Save the file
            std::string xlsx_file = xlsx.saveXlsxFile("JLWE Event Registrations", jlwe.config.at("websiteDomain"));

            FILE *file = fopen(xlsx_file.c_str(), "rb");
            if (file) { //if file exists in filesystem
                //output header
                std::cout << "Content-type:application/vnd.openxmlformats-officedocument.spreadsheetml.sheet\r\n";
                std::cout << "Content-Disposition: attachment; filename=jlwe_event_registrations_" << JlweUtils::getCurrentYearString() << ".xlsx\r\n\r\n";

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
