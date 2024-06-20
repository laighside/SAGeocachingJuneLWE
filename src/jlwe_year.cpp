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
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "core/CgiEnvironment.h"
#include "core/Encoder.h"
#include "core/HtmlTemplate.h"
#include "core/JlweCore.h"
#include "core/JlweUtils.h"
#include "core/KeyValueParser.h"

#define FILE_ICONS_URL  "/img/file_icons/"
#define THUMBNAIL_URL   "/cgi-bin/files/thumbnail.cgi"
static const std::map<std::string, std::string> file_type_icons = {
    {"gpx", "gpx.svg"},
    {"kml", "kml.svg"},
    {"kmz", "kml.svg"},
    {"doc", "doc.svg"},
    {"docx", "doc.svg"},
    {"xls", "xls.svg"},
    {"xlsx", "xls.svg"},
    {"ppt", "ppt.svg"},
    {"pptx", "ppt.svg"}
};

// These are the filetypes with thumbnail images
static const std::vector<std::string> img_file_types = {"bmp", "gif", "ico", "jpg", "jpeg", "png"};
static const std::vector<std::string> doc_file_types = {"pdf", "ps"};

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

            std::cout << "<h2>Caches</h2>\n";

            std::cout << "<p><a href=\"/map?year=" << year << "\">Click here to view the online map of caches</a></p>\n";

            std::cout << "<h2>Files</h2>\n";

            sql::PreparedStatement *prep_stmt;
            sql::ResultSet *res;

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT CONCAT(directory,filename),filename, size, owner, CONVERT_TZ(date_uploaded, '+00:00','+9:30') FROM files WHERE directory = ? AND public = 1;");
            prep_stmt->setString(1, "/" + std::to_string(year) + "/");
            res = prep_stmt->executeQuery();

            bool hasFiles = false;
            std::cout << "<div class=\"file_container\">\n";
            while (res->next()){
                hasFiles = true;

                int64_t file_size = res->getInt64(3);
                std::string file_size_str = std::to_string(file_size) + "&nbsp;B";
                if (file_size >= 1e3)
                    file_size_str = std::to_string(static_cast<int>(std::ceil(static_cast<double>(file_size) / 1e3))) + "&nbsp;kB";
                if (file_size >= 1e6)
                    file_size_str = std::to_string(static_cast<int>(std::ceil(static_cast<double>(file_size) / 1e6))) + "&nbsp;MB";
                if (file_size >= 1e9)
                    file_size_str = std::to_string(static_cast<int>(std::ceil(static_cast<double>(file_size) / 1e9))) + "&nbsp;GB";

                std::string filename = res->getString(2);

                std::string type_icon = std::string(FILE_ICONS_URL) + "file.svg";
                bool has_thumbnail = false;
                size_t idx = filename.find_last_of('.');
                if (idx != std::string::npos) {
                    std::string extension = filename.substr(idx + 1);
                    try {
                        type_icon = FILE_ICONS_URL + file_type_icons.at(extension);
                    } catch (...) {}

                    if (std::find(img_file_types.begin(), img_file_types.end(), extension) != img_file_types.end()) {
                        type_icon = std::string(THUMBNAIL_URL) + "?type=img&file=" + Encoder::urlEncode(res->getString(1)) + "&w=320&h=240";
                        has_thumbnail = true;
                    }
                    if (std::find(doc_file_types.begin(), doc_file_types.end(), extension) != doc_file_types.end()) {
                        type_icon = std::string(THUMBNAIL_URL) + "?type=doc&file=" + Encoder::urlEncode(res->getString(1)) + "&w=320&h=240";
                        has_thumbnail = true;
                    }
                }

                std::cout << "<div class=\"file\">\n";
                std::cout << "<a href=\"" << std::string(jlwe.config.at("files").at("urlPrefix")) << res->getString(1) << "\">\n";

                std::cout << "<span class=\"file_icon\">\n";
                std::cout << "<img src=\"" << type_icon << "\" alt=\"" << (has_thumbnail ? "thumbnail" : "icon") << "\" " << (has_thumbnail ? "class=\"thumbnail\" " : "") << "/>\n";
                std::cout << "</span>\n";

                std::cout << "<span class=\"file_name\">" << Encoder::htmlEntityEncode(filename) << " (" << file_size_str << ")</span>\n";

                std::cout << "</a>\n";
                std::cout << "</div>";
            }

            if (!hasFiles)
                std::cout << "<p style=\"padding-top:80px;text-align:center;font-style:italic;\">No files</p>\n";

            std::cout << "</div>\n";

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
