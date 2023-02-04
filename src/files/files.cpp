/**
  @file    files.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/files/files.cgi
  This page displays the file manager in an iframe with a link to the full version.
  The actual file manager is based on h5ai and is hosted at /h5ai/

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/Encoder.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"

int main () {
    try {
        JlweCore jlwe;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - Files", false))
            return 0;

        if (jlwe.getPermissionValue("perm_file")) { //if logged in

            html.outputAdminMenu();

            std::cout << "<h2 style=\"text-align:center\">Files</h2>\n";

            std::cout << "<p style=\"text-align:center;\"><a href=\"/h5ai/\">Click to view the File Manager in a full window</a></p>\n";

            std::cout << "<iframe src=\"/h5ai/index.html\" title=\"File Manager\" width=\"100%\" height=\"600\" style=\"border:1px solid black;margin-top:20px;\">Your browser does not support inline frames.</iframe>\n";

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
