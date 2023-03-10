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

            int messageId = 0;
            try {
                messageId = stoi(urlQueries.getValue("id"));
            } catch (...) {}

            std::string ip_address;
            std::string email_address;
            std::string from_name;
            std::string message;
            std::string timestamp;
            std::string status;

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT ip_address,timestamp,from_name,email_address,message,status FROM contact_form WHERE id = ?;");
            prep_stmt->setInt(1, messageId);
            res = prep_stmt->executeQuery();
            if (res->next()) {
                ip_address = res->getString(1);
                timestamp = res->getString(2);
                from_name = res->getString(3);
                email_address = res->getString(4);
                message = res->getString(5);
                status = res->getString(6);
            }
            delete res;
            delete prep_stmt;

            if (timestamp.size()) {

                std::cout << "<h2 style=\"text-align:center\">Message from " << Encoder::htmlEntityEncode(from_name) << "</h2>\n";

                if (status == "S") status = "Spam";
                if (status == "O") status = "Open";
                if (status == "R") status = "Resolved";
                if (status == "P") status = "Pending";

                std::string ip_country = JlweUtils::getGeoIPCountry(ip_address, jlwe.config.value("mmdbFilename", ""));

                std::cout << "<p>From: " << Encoder::htmlEntityEncode(from_name) << "<br />\n";
                std::cout << "Email: " << Encoder::htmlEntityEncode(email_address) << "<br />\n";
                std::cout << "Time received: " << Encoder::htmlEntityEncode(timestamp) << "<br />\n";
                std::cout << "IP address: " << Encoder::htmlEntityEncode(ip_address);
                if (ip_country.size())
                    std::cout << " (" << Encoder::htmlEntityEncode(ip_country) << ")";
                std::cout << "<br />\n";
                std::cout << "Status: " << Encoder::htmlEntityEncode(status) << "</p>\n";

                std::cout << "<p><pre>" << Encoder::htmlEntityEncode(message) << "</pre></p>\n";
            } else {
                std::cout << "<p>Message not found.</p>\n";
            }

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
