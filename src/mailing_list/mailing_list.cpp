/**
  @file    mailing_list.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/mailing_list/mailing_list.cgi
  This page shows the mailing list and associated functions to admins.

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

        sql::Statement *stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - Mailing List", false))
            return 0;

        if (jlwe.getPermissionValue("perm_email")) { //if logged in

            std::cout << "<script type=\"text/javascript\" src=\"/js/utils.js\"></script>\n";
            std::cout << "<script type=\"text/javascript\">\n";

            std::cout << "function resendEmail(email_address){\n";
            std::cout << "    if (confirm(\"Are you sure you want to resend a confirmation email to \\\"\" + email_address + \"\\\"?\") == true) {\n";
            std::cout << "        var jsonObj = {\n";
            std::cout << "            \"email\":email_address\n";
            std::cout << "        };\n";

            std::cout << "        postUrl('resend_verify_email.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "            function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, null, null);\n";
            std::cout << "            }, httpErrorResponseHandler);\n";
            std::cout << "    }\n";
            std::cout << "}\n\n";

            std::cout << "</script>\n";

            html.outputAdminMenu();

            std::cout << "<p id=\"page_note\" style=\"color:red;text-align:center;\"></p>";

            std::cout << "<h2 style=\"text-align:center\">Mailing list</h2>\n";

            std::cout << "<p style=\"text-align:center;\">\n";
            std::cout << "<a class=\"admin_button\" href=\"write_email.cgi\"><span>Send Email</span></a>\n";
            std::cout << "<a class=\"admin_button\" href=\"download_email_list.cgi\"><span>Download Email List</span></a>\n";
            std::cout << "</p>\n";

            std::cout << "<h2 style=\"text-align:center\">Email list</h2>\n";
            std::cout << "<p>Don't share the email list publicly. Some people have complained in the past about their email addresses been shared.</p>";
            std::cout << "<p><table align=\"center\"><tr>\n";
            std::cout << "<th>Email</th><th>Verified</th><th></th>\n";
            std::cout << "</tr>\n";

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT email,verify FROM email_list WHERE unsubscribed = 0;");
            while (res->next()) {
                std::cout << "<tr>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(1)) << "</td>\n";
                if (res->getInt(2)) {
                    std::cout << "<td>Yes</td>\n";
                } else {
                    std::cout << "<td>No</td>\n";
                }
                std::cout << "<td><button onclick=\"resendEmail('" << Encoder::javascriptAttributeEncode(res->getString(1)) << "')\" type=\"button\">Resend verification email</button></td>\n";
                std::cout << "</tr>\n";
            }
            delete res;
            delete stmt;
            std::cout << "</table></p>\n";

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
