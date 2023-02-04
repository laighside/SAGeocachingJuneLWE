/**
  @file    logout.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This is called when the user clicks on logout

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "core/HtmlTemplate.h"
#include "core/JlweCore.h"

int main () {
    try {
        JlweCore jlwe;

        // output header
        std::cout << "Set-Cookie:accessToken=;Domain=" << std::string(jlwe.config.at("websiteDomain")) << ";Path=/; Secure; HttpOnly;\r\n";
        std::cout << "Content-type:text/html\r\n\r\n";

        HtmlTemplate html(true);
        if (!html.outputHeader(&jlwe, "JLWE Logout", false))
            return 0;

        std::cout << "<p>You have been logged out. <a href=\"/index.html\">Return home</a></p>";

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
