/**
  @file    jlwe_year.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This script handles all the requests URLs with years like this /2022.html
  ModRewrite redirects requests to this script
  The content on these pages comes from the MySQL database and HTML files

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "core/CgiEnvironment.h"
#include "core/Encoder.h"
#include "core/HtmlTemplate.h"
#include "core/JlweCore.h"
#include "core/JlweUtils.h"
#include "core/KeyValueParser.h"

int main () {
    try {
        JlweCore jlwe;

        // get request data
        std::string page_request = CgiEnvironment::getRequestUri();

        // remove any url arguments (like fbclid)
        if (page_request.find("?") != std::string::npos)
            page_request = page_request.substr(0, page_request.find("?"));

        std::string year_str = page_request.substr(page_request.find_last_of('/') + 1);
        year_str = year_str.substr(0, year_str.find_first_of('.'));
        int year = 0;
        try {
            year = std::stoi(year_str);
        } catch (...) {}

        HtmlTemplate html(true);
        html.outputHttpHtmlHeader();

        if (!html.outputHeader(&jlwe, std::to_string(year) + " JLWE Event", false))
            return 0;

        std::string request_path = CgiEnvironment::getDocumentRoot() + "/" + std::to_string(year) + ".html";
        std::string html_page = "";
        try {
            html_page = JlweUtils::readFileToString(request_path.c_str());
        } catch (const std::exception &e) {
            // do nothing if the file can't be opened, the empty html_page will signal the error
        }
        if (html_page.size() == 0) {
            std::cout << "<p>The page " << Encoder::htmlEntityEncode(page_request) << " could not be found on the server</p>";
        } else {
            size_t files_index = html_page.find("**FILES**");

            std::cout << html_page.substr(0, files_index) << "\n";

            std::cout << "<h2>Files</h2>\n";

            std::cout << "<a href=\"/gmap.html?year=" << year << "\">Click here to view the game caches in Google Maps</a><br/><br/>\n";


            sql::PreparedStatement *prep_stmt;
            sql::ResultSet *res;

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT CONCAT(directory,filename),filename, size, owner, CONVERT_TZ(date_uploaded, '+00:00','+9:30') FROM files WHERE directory = ? AND public = 1;");
            prep_stmt->setString(1, "/" + std::to_string(year) + "/");
            res = prep_stmt->executeQuery();

            while (res->next()){
                int file_size = static_cast<int>(std::ceil(static_cast<double>(res->getInt(3)) / 1024)); // File size in KB
                std::cout << "<a href=\"" << std::string(jlwe.config.at("files").at("urlPrefix")) << res->getString(1) << "\">" << Encoder::htmlEntityEncode(res->getString(2)) << "</a> (" << file_size << "K) added by " << Encoder::htmlEntityEncode(res->getString(4)) << " on " << Encoder::htmlEntityEncode(res->getString(5)) << "<br/>";
            }

            delete res;
            delete prep_stmt;

        std::cout << html_page.substr(files_index + 9);
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
