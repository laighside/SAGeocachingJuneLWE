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

            std::cout << "function clearCachesTable(){\n";
            std::cout << "    if (confirm(\"This will erase all data in the caches and user hidden caches tables. Are you sure you want to proceed?\") == true) {\n";
            std::cout << "        var jsonObj = {\n";
            std::cout << "            \"confirm\":true,\n";
            std::cout << "            \"table\":\"caches\"\n";
            std::cout << "        };\n";

            std::cout << "        postUrl('clear_table.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "                function(data, responseCode) {\n";
            std::cout << "                    httpResponseHandler(data, responseCode, false, null, null);\n";
            std::cout << "             }, httpErrorResponseHandler);\n";
            std::cout << "    }\n";
            std::cout << "}\n\n";

            std::cout << "function clearGameTeamsTable(){\n";
            std::cout << "    if (confirm(\"This will erase all data in the game teams table. Are you sure you want to proceed?\") == true) {\n";
            std::cout << "        var jsonObj = {\n";
            std::cout << "            \"confirm\":true,\n";
            std::cout << "            \"table\":\"game_teams\"\n";
            std::cout << "        };\n";

            std::cout << "        postUrl('clear_table.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "                function(data, responseCode) {\n";
            std::cout << "                    httpResponseHandler(data, responseCode, false, null, null);\n";
            std::cout << "             }, httpErrorResponseHandler);\n";
            std::cout << "    }\n";
            std::cout << "}\n\n";

            std::cout << "function clearEmailListTable(){\n";
            std::cout << "    if (confirm(\"This will erase all data in the mailing list. Are you sure you want to proceed?\") == true) {\n";
            std::cout << "        var jsonObj = {\n";
            std::cout << "            \"confirm\":true,\n";
            std::cout << "            \"table\":\"email_list\"\n";
            std::cout << "        };\n";

            std::cout << "        postUrl('clear_table.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "                function(data, responseCode) {\n";
            std::cout << "                    httpResponseHandler(data, responseCode, false, null, null);\n";
            std::cout << "             }, httpErrorResponseHandler);\n";
            std::cout << "    }\n";
            std::cout << "}\n\n";

            std::cout << "function clearPublicFileUploadTable(){\n";
            std::cout << "    if (confirm(\"This will erase all data in the public file upload table, and permanently delete all the files uploaded. Are you sure you want to proceed?\") == true) {\n";
            std::cout << "        var jsonObj = {\n";
            std::cout << "            \"confirm\":true,\n";
            std::cout << "            \"table\":\"public_file_upload\"\n";
            std::cout << "        };\n";

            std::cout << "        postUrl('clear_table.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "                function(data, responseCode) {\n";
            std::cout << "                    httpResponseHandler(data, responseCode, false, null, null);\n";
            std::cout << "             }, httpErrorResponseHandler);\n";
            std::cout << "    }\n";
            std::cout << "}\n\n";

            std::cout << "function clearRegistrationsTable(){\n";
            std::cout << "    if (confirm(\"This will erase all data in the registrations, camping and dinner orders tables. Are you sure you want to proceed?\") == true) {\n";
            std::cout << "        var jsonObj = {\n";
            std::cout << "            \"confirm\":true,\n";
            std::cout << "            \"table\":\"registrations\"\n";
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
            std::cout << "<p>This clears the cache handout table.</p>\n";
            std::cout << "<p>Number of game caches: <input type=\"number\" name=\"new_cache_count_input\" id=\"new_cache_count_input\" min=\"1\" max=\"10000\" step=\"1\" value=\"100\"> <input type=\"button\" onclick=\"clearHandoutTable();\" value=\"Clear table and reset cache count\" class=\"red_button\" ></p>\n";

            std::cout << "<h3>Cache table</h3>\n";
            std::cout << "<p>This clears the list of caches in the GPX builder.</p>\n";
            std::cout << "<p><input type=\"button\" onclick=\"clearCachesTable();\" value=\"Clear caches table\" class=\"red_button\" ></p>\n";

            std::cout << "<h3>Game teams table</h3>\n";
            std::cout << "<p>This clears the list of competing teams.</p>\n";
            std::cout << "<p><input type=\"button\" onclick=\"clearGameTeamsTable();\" value=\"Clear game teams table\" class=\"red_button\" ></p>\n";

            std::cout << "<h3>Public file upload table</h3>\n";
            std::cout << "<p>This deletes all the files in the public_upload folder.</p>\n";
            std::cout << "<p><input type=\"button\" onclick=\"clearPublicFileUploadTable();\" value=\"Clear public file upload\" class=\"red_button\" ></p>\n";

            std::cout << "<h3>Email list table</h3>\n";
            std::cout << "<p>This clears the mailing list.</p>\n";
            std::cout << "<p><input type=\"button\" onclick=\"clearEmailListTable();\" value=\"Clear email list table\" class=\"red_button\" ></p>\n";

            std::cout << "<h3>Registrations tables</h3>\n";
            std::cout << "<p>This clears the event registrations, caming and dinner orders tables.</p>\n";
            std::cout << "<p><input type=\"button\" onclick=\"clearRegistrationsTable();\" value=\"Clear registrations/camping/dinner tables\" class=\"red_button\" ></p>\n";

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
