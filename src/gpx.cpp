/**
  @file    gpx.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This is used for downloading the public GPX file for the current year
  ModRewrite is used to rewrite /gpx to this script
  /gpx?year=2012 can be used to get GPX files from past years

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "core/CgiEnvironment.h"
#include "core/HtmlTemplate.h"
#include "core/JlweCore.h"
#include "core/JlweUtils.h"
#include "core/KeyValueParser.h"

#define BUFFER_SIZE 65536

int main () {
    try {
        JlweCore jlwe;

        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);
        std::string year = urlQueries.getValue("year");

        if (year.size() == 0)
            year = JlweUtils::getCurrentYearString();

        std::string file_dir = jlwe.config.at("files").at("directory");

        // any public file ending with .gpx in the folder named after the year is considered the GPX file for that year
        sql::PreparedStatement *prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT directory, filename, public FROM files WHERE directory = ? AND RIGHT(filename, 4) = '.gpx' ORDER BY public DESC;");
        prep_stmt->setString(1, "/" + year + "/");
        sql::ResultSet *res = prep_stmt->executeQuery();

        bool validFile = false;

        if (res->next()) { // if the file exists in MySQL (if there is more than one GPX file, the first one is used)
            if (res->getInt(3) != 0 || jlwe.isLoggedIn()) { // if public file or logged in

                std::string folder = res->getString(1);
                std::string filename = res->getString(2);
                std::string full_filename = file_dir + folder + filename;
                std::string mime_type = JlweUtils::getMIMEType(full_filename);

                FILE *file = fopen(full_filename.c_str(), "rb");
                if (file) { //if file exists in filesystem
                    //output header
                    std::cout << "Content-type:" << mime_type << "\r\n";
                    std::cout << "Content-Disposition: attachment; filename=" << filename << "\r\n\r\n";

                    char buffer[BUFFER_SIZE];
                    size_t size = BUFFER_SIZE;
                    while (size == BUFFER_SIZE){
                        size = fread(buffer, 1, BUFFER_SIZE, file);
                        std::cout.write(buffer, size);
                    }
                    validFile = true;
                    fclose(file);
                }
            }
        }

        // output error in HTML format if there is no public GPX file
        if (validFile == false){
            HtmlTemplate::outputHttpHtmlHeader();

	    std::cout << "<!DOCTYPE html>\n<html>\n<body>\n";
	    std::cout << "<p style=\"text-align:center;font-size: 48px;\">The GPX file is not available yet. Please try again on Sunday morning of the June LWE.</p>\n";
	    std::cout << "</body>\n</html>\n";
        }

        delete res;
        delete prep_stmt;

        // log download request
        // ignore any errors, the users don't care about the logging
        try {
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT insertFileDownloadLog(?,?,?,?);");
            prep_stmt->setString(1, "/gpx");
            prep_stmt->setString(2, CgiEnvironment::getUserAgent());
            prep_stmt->setString(3, jlwe.getCurrentUserIP());
            prep_stmt->setInt(4, validFile ? 200 : 404);
            res = prep_stmt->executeQuery();
            delete res;
            delete prep_stmt;
        } catch (...) {}

    } catch (const sql::SQLException &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what() << " (MySQL error code: " << std::to_string(e.getErrorCode()) << ")\n";
    } catch (const std::exception &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what();
    }
    return 0;
}
