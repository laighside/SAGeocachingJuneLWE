/**
  @file    contact_form.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/contact_form/contact_form.cgi
  This page shows the messages that have been submitted to the contact form

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/Encoder.h"
#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"

std::string statusToString(char status) {
    if (status == 'O') return "Open";
    if (status == 'S') return "Spam";
    if (status == 'P') return "Pending";
    if (status == 'R') return "Resolved";
    return "Unknown";
}

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - Contact Form", false))
            return 0;

        if (jlwe.isLoggedIn()) { //if logged in

            std::cout << "<script type=\"text/javascript\" src=\"/js/utils.js\"></script>\n";
            std::cout << "<script type=\"text/javascript\">\n";

            std::cout << "function markAsSpam(message_id){\n";
            std::cout << "        var jsonObj = {\n";
            std::cout << "            \"message_id\":parseInt(message_id),\n";
            std::cout << "            \"status\":\"S\"\n";
            std::cout << "        };\n";

            std::cout << "        postUrl('set_message_status.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "            function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, null, null);\n";
            std::cout << "            }, httpErrorResponseHandler);\n";
            std::cout << "}\n\n";

            std::cout << "</script>\n";

            html.outputAdminMenu();

            std::cout << "<p id=\"page_note\" style=\"color:red;text-align:center;\"></p>";

            std::cout << "<h2 style=\"text-align:center\">Contact Form</h2>\n";
            std::cout << "<p>These are the messages that have been submitted through the form on the contact page. Respond to them or mark them as spam. Coordinate with other admins so each (non-spam) message is only responded to once, then mark the message as \"Resolved\" once complete.</p>";

            std::cout << "<h2 style=\"text-align:center\">Message list</h2>\n";
            std::cout << "<p><table id=\"messageTable\" class=\"reg_table\" align=\"center\"><tr>\n";
            std::cout << "<th>Time</th><th>From</th><th>Email</th><th>Message</th><th>Status</th><th></th>\n";
            std::cout << "</tr>\n";

            std::cout << "<p style=\"text-align:right;\">Show open messages only: " << FormElements::htmlSwitch("openToggleCB", true) << "</p>\n";

            int rowId = 0;

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT id,timestamp,from_name,email_address,message,status FROM contact_form ORDER BY timestamp DESC;;");
            while (res->next()) {
                std::cout << "<tr" << ((res->getString(6) == "O") ? "" : " class=\"not_s\"") << ">\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(2)) << "</td>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(3)) << "</td>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(4)) << "</td>\n";
                std::string message = res->getString(5);
                message = JlweUtils::replaceString(message, "\n", " ");
                if (message.length() > 40)
                    message = message.substr(0, 37) + "...";
                std::cout << "<td>" << Encoder::htmlEntityEncode(message) << "</td>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(statusToString(res->getString(6)->at(0))) << "</td>\n";

                std::vector<FormElements::dropDownMenuItem> menuItems;
                menuItems.push_back({"location.href='/cgi-bin/contact_form/view_message.cgi?id=" + std::to_string(res->getInt(1)) + "'", "View message", true});
                menuItems.push_back({"markAsSpam(" + std::to_string(res->getInt(1)) + ")", "Mark as spam", true});
                std::cout << "<td>" << FormElements::dropDownMenu(rowId, menuItems) << "</tr>\n";

                rowId++;

                std::cout << "</tr>\n";
            }
            delete res;
            delete stmt;
            std::cout << "</table></p>\n";

            std::cout << "<p style=\"padding:20px\"></p>\n";

            std::cout << FormElements::includeJavascript("/js/menu.js");
            std::cout << "<script>\n";
            std::cout << "function toggleOpen(e) {\n";
            std::cout << "    var new_display = e.currentTarget.checked ? \"none\" : \"table-row\";\n";
            std::cout << "    var rows = document.getElementsByClassName(\"not_s\");\n";
            std::cout << "    for(var i = 0; i < rows.length; i++) {\n";
            std::cout << "        rows[i].style.display = new_display;\n";
            std::cout << "    }\n";
            std::cout << "}\n";
            std::cout << "toggleOpen({currentTarget:document.getElementById(\"openToggleCB\")});\n";
            std::cout << "document.getElementById(\"openToggleCB\").addEventListener('change', toggleOpen);\n";

            std::cout << "</script>\n";

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
