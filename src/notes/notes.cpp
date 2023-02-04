/**
  @file    notes.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/notes/notes.cgi
  This page displays (and allows editing of) notes that made by organisers.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/Encoder.h"
#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/KeyValueParser.h"

// For markdown rendering
#include "../ext/md4c/md4c-html.h"

static void md4c_process_output(const MD_CHAR* text, MD_SIZE size, void* userdata)
{
    std::string *str_out = reinterpret_cast<std::string*>(userdata);
    *str_out += std::string(text, size);
}

int main () {
    try {
        JlweCore jlwe;
        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        sql::Statement *stmt;
        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area", false))
            return 0;

        if (jlwe.isLoggedIn()) { //if logged in
            int notes_version = 0;
            try {
                notes_version = std::stoi(urlQueries.getValue("version"));
            } catch (...) {}

            std::string md_text = "";
            std::string timestamp = "";

            if (notes_version > 0) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT timestamp,markdown FROM admin_notes WHERE id = ?;");
                prep_stmt->setInt(1, notes_version);
            } else {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT timestamp,markdown FROM admin_notes ORDER BY timestamp DESC LIMIT 1;");
            }
            res = prep_stmt->executeQuery();
            if (res->next()){
                timestamp = res->getString(1);
                md_text = res->getString(2);
            }
            delete res;
            delete prep_stmt;

            std::string str_out = "";

            int ret = md_html(md_text.c_str(), static_cast<MD_SIZE>(md_text.size()), md4c_process_output, reinterpret_cast<void*>(&str_out), 0, 0);
            if (ret != 0) {
                str_out = "<p>Markdown Parsing failed</p>";
            }



            // HTML output
            html.outputAdminMenu();

            std::cout << "<h2 style=\"text-align:center\">Notes</h2>\n";
            std::cout << "<p style=\"padding-bottom:10px;\">These notes are viewable and editable by all admins on the site. Use it for to-do lists, sharing ideas and any other notes you wish to make about the event.</p>\n";

            if (notes_version > 0)
                std::cout << "<p id=\"page_note\" style=\"color:red;text-align:center;padding-bottom:10px;\">This is an old version of the notes as it appered on " << Encoder::htmlEntityEncode(timestamp) << ". <a href=\"notes.cgi\">Click here to view the current version.</a></p>";




            std::cout << FormElements::includeJavascript("/js/utils.js");
            std::cout << FormElements::includeJavascript("/js/page_tab_tools.js");

            std::cout << "<script>\n";

            std::cout << "function saveNotesMD(){\n";
            std::cout << "    var jsonObj = {\n";
            std::cout << "        \"markdown\":document.getElementById(\"md_edit\").value\n";
            std::cout << "    };\n";

            std::cout << "    postUrl('save_notes.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "        function(data, responseCode) {\n";
            std::cout << "            httpResponseHandler(data, responseCode, false, null, null);\n";
            std::cout << "        }, httpErrorResponseHandler);\n";
            std::cout << "}\n\n";

            std::cout << "</script>\n";

            std::cout << FormElements::pageTabs({{"view_tab", "View"}, {"edit_tab", "Edit"}, {"history_tab", "History"}});


            std::cout << "<div id=\"view_tab\" class=\"pageTabContent\">\n";
            std::cout << str_out;
            std::cout << "</div>\n";

            std::cout << "<div id=\"edit_tab\" class=\"pageTabContent\">\n";
            std::cout << "  <h3>Edit notes</h3>\n";
            if (notes_version > 0)
                std::cout << "<p id=\"page_note\" style=\"color:red;text-align:center;padding-bottom:10px;\">You are editing an old version of the notes. <a href=\"notes.cgi\">Click here to view the current version.</a></p>";
            std::cout << "<p>Edit the notes using Markdown. For help, visit <a href=\"https://commonmark.org/help/\">https://commonmark.org/help/</a></p>\n";
            std::cout << "<p style=\"text-align:center;\"><textarea id=\"md_edit\" rows=\"30\" style=\"width:100%;\">" << Encoder::htmlEntityEncode(md_text) << "</textarea></p>\n";
            std::cout << "<p style=\"text-align:center;\"><input type=\"button\" onclick=\"saveNotesMD()\" value=\"Save changes\" /></p>\n";

            std::cout << "</div>\n";

            std::cout << "<div id=\"history_tab\" class=\"pageTabContent\">\n";
            std::cout << "  <h3>History</h3>\n";

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT admin_notes.id,admin_notes.timestamp,users.username,LENGTH(admin_notes.markdown) FROM admin_notes INNER JOIN users ON admin_notes.user_id=users.user_id ORDER BY admin_notes.timestamp DESC;");
            while (res->next()) {
                std::cout << "<p>";
                std::cout << "<a href=\"notes.cgi?version=" << res->getInt(1) << "\">" << Encoder::htmlEntityEncode(res->getString(2)) << "</a> by " << Encoder::htmlEntityEncode(res->getString(3));
                std::cout << " (" << res->getInt(4) << " bytes)";
                std::cout << "</p>\n";
            }
            delete res;
            delete stmt;

            std::cout << "</div>\n";

            std::cout << "<script>\n";
            std::cout << "document.getElementsByClassName(\"defaultPageTab\")[0].click();\n";
            std::cout << "</script>\n";


        } else {
            std::cout << "<p>You need to be logged in to view this area.</p>";
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
