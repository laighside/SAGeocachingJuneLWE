/**
  @file    admin_tools.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/admin/admin_tools.cgi
  This page has stuff for managing the website

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/Encoder.h"
#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - Scoring", false))
            return 0;

        if (jlwe.getPermissionValue("perm_admin")) { //if logged in
            std::string message = "";

            html.outputAdminMenu();

            std::cout << FormElements::includeJavascript("/js/utils.js");

            std::cout << "<script>\n";

            std::cout << "function clearHandoutTable(){\n";
            std::cout << "    if (confirm(\"This will erase all data in the cache handout table. Are you sure you want to proceed?\") == true) {\n";
            std::cout << "        var jsonObj = {\n";
            std::cout << "            \"confirm\":true,\n";
            std::cout << "            \"table\":\"cache_handout\",\n";
            std::cout << "            \"cache_count\":parseInt(document.getElementById(\"new_cache_count_input\").value)\n";
            std::cout << "        };\n";

            std::cout << "        postUrl('clear_table.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "                function(data, responseCode) {\n";
            std::cout << "                    httpResponseHandler(data, responseCode, false, null, null);\n";
            std::cout << "             }, httpErrorResponseHandler);\n";
            std::cout << "    }\n";
            std::cout << "}\n\n";

            std::cout << "</script>\n";


            std::cout << "<h2 style=\"text-align:center\">JLWE Admin Tools</h2>\n";

            std::cout << "<h3>Cache handout table</h3>\n";
            std::cout << "<p>Number of game caches: <input type=\"number\" name=\"new_cache_count_input\" id=\"new_cache_count_input\" min=\"1\" max=\"10000\" step=\"1\" value=\"100\"> <input type=\"button\" onclick=\"clearHandoutTable();\" value=\"Clear table and reset cache count\" ></p>\n";

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
