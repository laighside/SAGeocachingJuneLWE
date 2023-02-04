/**
  @file    write_email.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/mailing_list/write_email.cgi
  This page has the form that admins use to send an email to the mailing list

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <cmath>
#include <string>

#include "../core/Encoder.h"
#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"

int main () {
    try {
        JlweCore jlwe;

        std::string year = JlweUtils::getCurrentYearString();

        sql::Statement *stmt;
        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - Write Email", false))
            return 0;

        if (jlwe.getPermissionValue("perm_email")) { //if logged in
            html.outputAdminMenu();

            int email_count = 0;
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT COUNT(*) FROM email_list WHERE verify = 1 AND unsubscribed = 0;");
            if (res->next()) {
                email_count = res->getInt(1);
            }
            delete res;
            delete stmt;

            std::cout << "<script type=\"text/javascript\">\n";
            std::cout << "    function ShowHideDiv(pastChk) {\n";
            std::cout << "        var pastYearFiles = document.getElementById(\"pastYearFiles\");\n";
            std::cout << "        pastYearFiles.style.display = pastChk.checked ? \"block\" : \"none\";\n";
            std::cout << "    }\n";
            std::cout << "    function ShowHideRecipients(allChk) {\n";
            std::cout << "        var email_list_box = document.getElementById(\"email_list_box\");\n";
            std::cout << "        email_list_box.style.display = allChk.checked ? \"none\" : \"block\";\n";
            std::cout << "        if (allChk.checked) {\n";
            std::cout << "            var email_checkboxs = document.getElementsByClassName(\"email_select_checkbox\");\n";
            std::cout << "            for (let i = 0; i < email_checkboxs.length; i++)\n";
            std::cout << "                email_checkboxs[i].checked = true;\n";
            std::cout << "        }\n";
            std::cout << "    }\n";
            std::cout << "    function validateForm(form) {\n";
            std::cout << "        var sendToAll = document.getElementById(\"sendToAll\");\n";
            std::cout << "        if (sendToAll.checked)\n";
            std::cout << "            return true;\n";
            std::cout << "        var email_checkboxs = document.getElementsByClassName(\"email_select_checkbox\");\n";
            std::cout << "        var selectedCount = 0;\n";
            std::cout << "        for (let i = 0; i < email_checkboxs.length; i++)\n";
            std::cout << "            if (email_checkboxs[i].checked)\n";
            std::cout << "                selectedCount++;\n";
            std::cout << "        return confirm('Are you sure you only want to send to ' + selectedCount.toString() + ' of " + std::to_string(email_count) + " recipients?');\n";
            std::cout << "    }\n";
            std::cout << "</script>\n";

            std::cout << "<h1>Send email to mailing list</h1>\n";
            std::cout << "<p>Fill out the form below to send an email to everyone in the mailing list.<br/>\n";
            std::cout << "Note that this will allow you to send files marked as 'Private' - be careful which files you select.<br/>\n";
            std::cout << "Emails will not be sent to unverified addresses.</p>\n";
            std::cout << "<p><form action=\"send_email.cgi\" method=\"post\" onsubmit=\"return validateForm(this);\">\n";
            std::cout << "<table border=\"0\" align=\"left\" style=\"border:0\">\n";
            std::cout << "<tr style=\"border:0\"><td style=\"border:0\">Subject:</td><td style=\"border:0\"><input type=\"text\" name=\"subject\" size=\"60\"></td></tr>\n";
            std::cout << "<tr style=\"border:0\"><td style=\"border:0\">Reply-to:</td><td style=\"border:0\"><input type=\"email\" name=\"reply\" size=\"60\" value=\"noreply@" << std::string(jlwe.config.at("websiteDomain")) << "\"></td></tr>\n";
            std::cout << "<tr style=\"border:0\"><td style=\"border:0\">Email text:</td><td style=\"border:0\"><textarea name=\"message\" rows=\"16\" cols=\"60\"></textarea></tr>\n";
            std::cout << "<tr style=\"border:0\"><td style=\"border:0;vertical-align:top;\">Attachments:</td><td style=\"border:0\"><p>\n";

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT CONCAT(directory,filename),size FROM files WHERE directory = ? AND RIGHT(filename, 1) != '/';");
            prep_stmt->setString(1, "/" + year + "/");
            res = prep_stmt->executeQuery();
            while (res->next()) {
                std::string filename = res->getString(1);
                int file_size = res->getInt(2);
                std::cout << "<input type=\"checkbox\" name=\"" << Encoder::htmlAttributeEncode(filename) << "\" value=\"true\">" << Encoder::htmlEntityEncode(filename) << " (" << static_cast<int>(ceil(static_cast<double>(file_size) / 1024)) << "K)<br>\n";
            }
            delete res;
            delete prep_stmt;

            std::cout << "</p>\n";

            std::cout << "<div style=\"background-color: lightgrey;margin:10px;\"><p style=\"font-style:italic;\"><input type=\"checkbox\" id=\"pastYears\" onclick=\"ShowHideDiv(this)\" />Show files from past years</p>\n";

            std::cout << "<p><div id=\"pastYearFiles\" style=\"display:none;margin: 10px\">";

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT CONCAT(directory,filename),size FROM files WHERE directory != ? AND RIGHT(filename, 1) != '/' ORDER BY year DESC;");
            prep_stmt->setString(1, "/" + year + "/");
            res = prep_stmt->executeQuery();
            while (res->next()) {
                std::string filename = res->getString(1);
                int file_size = res->getInt(2);
                std::cout << "<input type=\"checkbox\" name=\"" << Encoder::htmlAttributeEncode(filename) << "\" value=\"true\">" << Encoder::htmlEntityEncode(filename) << " (" << static_cast<int>(ceil(static_cast<double>(file_size) / 1024)) << "K)<br>\n";
            }
            delete res;
            delete prep_stmt;

            std::cout << "</div></p></div>";
            std::cout << "</tr>\n";

            std::cout << "<tr style=\"border:0\"><td style=\"border:0;vertical-align:top;\">Recipients:</td>\n";
            std::cout << "<td style=\"border:0\"><p><input type=\"checkbox\" id=\"sendToAll\" name=\"sendToAll\" value=\"true\" onclick=\"ShowHideRecipients(this)\" checked>Send email to all " + std::to_string(email_count) + " recipients on mailing list</input></p>\n";


            std::cout << "<div id=\"email_list_box\" style=\"height:300px;overflow-y:scroll;border: 1px solid black;display: none;\">\n";

            std::cout << "<table class=\"reg_table\" align=\"center\" style=\"width: 100%;margin:0px;\">\n";
            std::cout << "<tr><th>Email</th><th>Username</th><th>Date registered</th>\n";
            std::cout << "</tr>\n";

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT email_list.email, event_registrations.gc_username, DATE(event_registrations.timestamp) FROM email_list LEFT JOIN event_registrations ON "
                                     "(email_list.email=event_registrations.email_address AND event_registrations.timestamp = (SELECT MIN(timestamp) FROM event_registrations er1 WHERE er1.email_address = email_list.email)) "
                                     "WHERE email_list.verify = 1 AND email_list.unsubscribed = 0 ORDER BY event_registrations.timestamp;");
            while (res->next()) {
                std::string email_address = res->getString(1);
                std::cout << "<tr>\n";

                std::cout << "<td>\n";
                //std::cout << "<td>" << FormElements::checkbox("checkbox_" + Encoder::htmlAttributeEncode(email_address), Encoder::htmlEntityEncode(email_address), true) << "</td>\n";
                // need this to add custom style
                std::string id = "checkbox_" + Encoder::htmlAttributeEncode(email_address);
                std::cout << "<span class=\"checkbox_container\"><label>" + Encoder::htmlEntityEncode(email_address) + "\n";
                std::cout << "<input class=\"email_select_checkbox\" type=\"checkbox\" id=\"" + id + "\" name=\"" + id + "\" value=\"true\" checked />\n";
                std::cout << "<span class=\"checkmark\"></span>\n";
                std::cout << "</label></span>\n";

                std::cout << "</td>\n";

                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(2)) << "</td>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(3)) << "</td>\n";
                std::cout << "</tr>\n";
            }
            delete res;
            delete stmt;

            std::cout << "</table>\n";
            std::cout << "</div>";

            std::cout << "</td></tr>\n";

            std::cout << "<tr style=\"border:0\"><td style=\"border:0\" colspan=\"2\" align=\"center\">Please press submit once and wait - it will take several seconds to send all the emails.<br/><input type=\"submit\"></td></tr>\n";
            std::cout << "</table></form>\n";
            std::cout << "</p>\n";
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
