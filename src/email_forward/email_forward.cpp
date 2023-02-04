/**
  @file    email_forward.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/email_forward/email_forward.cgi
  This page allows admins to view and edit the email forwarder settings.

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
        if (!html.outputHeader(&jlwe, "JLWE Admin area - Email Forwarding", false))
            return 0;

        if (jlwe.isLoggedIn()) { //if logged in
            std::cout << "<script type=\"text/javascript\" src=\"/js/utils.js\"></script>\n";
            std::cout << "<script type=\"text/javascript\">\n";

            std::cout << "function setPostJSON(jsonObj, callback){\n";
            std::cout << "    postUrl('/cgi-bin/email_forward/set_email_forwarder.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "        function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, function() {if (callback) callback(true);}, function() {if (callback) callback(false);});\n";
            std::cout << "         }, httpErrorResponseHandler);\n";
            std::cout << "}\n\n";

            std::cout << "function setEmail(sourceEmail){\n";
            std::cout << "    var jsonObj = {\n";
            std::cout << "        \"source\":sourceEmail,\n";
            std::cout << "        \"destination\":document.getElementById(sourceEmail + \"_destination_textbox\").value,\n";
            std::cout << "        \"deleteEmail\":false\n";
            std::cout << "    };\n";
            std::cout << "    setPostJSON(jsonObj, null);\n";
            std::cout << "}\n\n";

            std::cout << "function deleteEmail(sourceEmail){\n";
            std::cout << "    var jsonObj = {\n";
            std::cout << "        \"source\":sourceEmail,\n";
            std::cout << "        \"destination\":'',\n";
            std::cout << "        \"deleteEmail\":true\n";
            std::cout << "    };\n";
            std::cout << "    setPostJSON(jsonObj, null);\n";
            std::cout << "}\n\n";

            std::cout << "function addEmail(){\n";
            std::cout << "        sourceEmail = document.getElementById(\"new_email_textbox\").value;\n";
            std::cout << "        destinationEmail = document.getElementById(\"new_email_destination_textbox\").value;\n";
            std::cout << "    var jsonObj = {\n";
            std::cout << "        \"source\":sourceEmail,\n";
            std::cout << "        \"destination\":destinationEmail,\n";
            std::cout << "        \"deleteEmail\":false\n";
            std::cout << "    };\n";
            std::cout << "    setPostJSON(jsonObj,\n";
            std::cout << "        function(result) {\n";
            std::cout << "            if (result) {\n";
            std::cout << "                var table = document.getElementById(\"email_table\");\n";
            std::cout << "                var row = table.insertRow(table.rows.length - 1);\n";
            std::cout << "                var rowHTML = \"<td>\" + sourceEmail + \"@" << std::string(jlwe.config.at("emailForwarderDomain")) << "</td><td><input type=\\\"text\\\" id=\\\"\" + sourceEmail + \"_destination_textbox\\\" value=\\\"\" + destinationEmail + \"\\\" /></td>\";\n";
            std::cout << "                rowHTML += \"<td><input onclick=\\\"setEmail('\" + sourceEmail + \"');\\\" type=\\\"button\\\" value=\\\"Save\\\" /></td><td><input onclick=\\\"deleteEmail('\" + sourceEmail + \"');\\\" type=\\\"button\\\" value=\\\"Delete\\\" /></td>\";\n";
            std::cout << "                row.innerHTML = rowHTML\n";
            std::cout << "                document.getElementById(\"new_email_textbox\").value = '';\n";
            std::cout << "                document.getElementById(\"new_email_destination_textbox\").value = '';\n";
            std::cout << "            }\n";
            std::cout << "        });\n";
            std::cout << "}\n\n";

            std::cout << "</script>\n";

            html.outputAdminMenu();

            std::cout << "<h2 style=\"text-align:center\">JLWE email settings</h2>\n";
            std::cout << "<p id=\"page_note\" style=\"color:red;text-align:center;\"></p>";
            std::cout << "<p style=\"color:red;text-align:center;\"><noscript>You need javascript enabled to use the admin tools on this site.</noscript></p>\n";

            std::cout << "<p>You can create @" << std::string(jlwe.config.at("emailForwarderDomain")) << " email addresses and any mail sent to them will be forwarded to the email provided below. You can create as many email addresses as you like.</p>\n";
            std::cout << "<p><table id=\"email_table\" align=\"center\"><tr>\n";
            std::cout << "<th>Source email</th><th>Destination email</th>\n";
            std::cout << "</tr>\n";

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT source,destination FROM email_forwarders;");
            bool editable = jlwe.getPermissionValue("perm_email_forward");
            while (res->next()) {
                std::string source = res->getString(1);
                std::cout << "<tr>\n";
                std::cout << "<td>" << source << "@" << std::string(jlwe.config.at("emailForwarderDomain")) << "</td>\n";
                std::cout << "<td><input type=\"text\" id=\"" << Encoder::htmlAttributeEncode(source) << "_destination_textbox\" value=\"" << Encoder::htmlAttributeEncode(res->getString(2)) << "\" " << (editable ? "" : "disabled") << "></td>\n";
                if (editable) {
                    std::cout << "<td><input onclick=\"setEmail('" << Encoder::javascriptAttributeEncode(source) << "');\" type=\"button\" value=\"Save\" /></td>\n";
                    std::cout << "<td><input onclick=\"deleteEmail('" << Encoder::javascriptAttributeEncode(source) << "');\" type=\"button\" value=\"Delete\" /></td>\n";
                }
                std::cout << "</tr>\n";
            }
            delete res;
            delete stmt;
            if (editable) {
                std::cout << "<td><input type=\"text\" id=\"new_email_textbox\" style=\"width:200px\" />@" << std::string(jlwe.config.at("emailForwarderDomain")) << "</td>\n";
                std::cout << "<td><input type=\"text\" id=\"new_email_destination_textbox\" /></td>\n";
                std::cout << "<td colspan=\"2\"><input onclick=\"addEmail();\" type=\"button\" value=\"Create New\" /></td>\n";
            }
            std::cout << "</table></p>\n";

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
