/**
  @file    view_message.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/contact_form/view_message.cgi?id=...
  This page shows the full message and details for a given message id

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/Encoder.h"
#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/KeyValueParser.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"

int main () {
    try {
        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        JlweCore jlwe;

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - View message", false))
            return 0;

        if (jlwe.isLoggedIn()) { //if logged in

            std::cout << "<script type=\"text/javascript\" src=\"/js/utils.js\"></script>\n";
            std::cout << "<script type=\"text/javascript\">\n";

            std::cout << "let user_email = \"" + Encoder::javascriptAttributeEncode(jlwe.getCurrentUserEmail()) + "\";\n";

            std::cout << "function forwardMessage(message_id){\n";
            std::cout << "    let email_address = prompt(\"Email to forward to:\", user_email);\n";
            std::cout << "    if (email_address != null) {\n";
            std::cout << "        var jsonObj = {\n";
            std::cout << "            \"message_id\":parseInt(message_id),\n";
            std::cout << "            \"email\":email_address\n";
            std::cout << "        };\n";

            std::cout << "        postUrl('forward_message.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "            function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, null, null);\n";
            std::cout << "            }, httpErrorResponseHandler);\n";
            std::cout << "    }\n";
            std::cout << "}\n\n";

            std::cout << "function setMessageStatus(message_id){\n";
            std::cout << "    var new_status = document.getElementById(\"status_drop_down\").value;\n";
            std::cout << "    new_status = new_status.substring(0, 1);\n";
            std::cout << "        var jsonObj = {\n";
            std::cout << "            \"message_id\":parseInt(message_id),\n";
            std::cout << "            \"status\":new_status\n";
            std::cout << "        };\n";

            std::cout << "        postUrl('set_message_status.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "            function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, true, null, null);\n";
            std::cout << "            }, httpErrorResponseHandler);\n";
            std::cout << "}\n\n";

            std::cout << "</script>\n";

            int messageId = 0;
            try {
                messageId = stoi(urlQueries.getValue("id"));
            } catch (...) {}

            std::string ip_address;
            std::string email_address;
            std::string from_name;
            std::string message;
            time_t timestamp = 0;
            std::string status;

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT ip_address,UNIX_TIMESTAMP(timestamp),from_name,email_address,message,status FROM contact_form WHERE id = ?;");
            prep_stmt->setInt(1, messageId);
            res = prep_stmt->executeQuery();
            if (res->next()) {
                ip_address = res->getString(1);
                timestamp = res->getInt64(2);
                from_name = res->getString(3);
                email_address = res->getString(4);
                message = res->getString(5);
                status = res->getString(6);
            }
            delete res;
            delete prep_stmt;

            if (timestamp) {

                std::cout << "<h2 style=\"text-align:center\">Message from " << Encoder::htmlEntityEncode(from_name) << "</h2>\n";

                std::cout << "<p><input type=\"button\" onclick=\"location.href='/cgi-bin/contact_form/contact_form.cgi'\" value=\"Return to list\"></p>\n";

                std::string ip_country = JlweUtils::getGeoIPCountry(ip_address, jlwe.config.value("mmdbFilename", ""));

                std::cout << "<p>From: " << Encoder::htmlEntityEncode(from_name) << "<br />\n";
                std::cout << "Email: " << Encoder::htmlEntityEncode(email_address) << "<br />\n";
                std::cout << "Time received: <span class=\"date_time\" data-value=\"" << timestamp << "\"></span><br />\n";
                std::cout << "IP address: " << Encoder::htmlEntityEncode(ip_address);
                if (ip_country.size())
                    std::cout << " (" << Encoder::htmlEntityEncode(ip_country) << ")";
                std::cout << "<br />\n";

                std::cout << "Status: <select id=\"status_drop_down\" onchange=\"setMessageStatus(" << messageId << ")\">\n";
                std::cout << "<option value=\"Open\"" << ((status == "O") ? " selected" : "") << ">Open</option>\n";
                std::cout << "<option value=\"Resolved\"" << ((status == "R") ? " selected" : "") << ">Resolved</option>\n";
                std::cout << "<option value=\"Spam\"" << ((status == "S") ? " selected" : "") << ">Spam</option>\n";
                std::cout << "<option value=\"Pending\"" << ((status == "P") ? " selected" : "") << ">Pending</option>\n";
                std::cout << "</select></p>\n";

                std::cout << "<p><pre>" << Encoder::htmlEntityEncode(message) << "</pre></p>\n";

                std::cout << "<p>To respond to this message, forward it to your email with the button below then reply to it using your email provider. Please don't forward spam messages.</p>\n";
                std::cout << "<p><input type=\"button\" onclick=\"forwardMessage(" << messageId << ")\" value=\"Forward to email\"></p>\n";
            } else {
                std::cout << "<p>Message not found.</p>\n";
            }

            std::cout << FormElements::includeJavascript("/js/format_date_time.js");

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
