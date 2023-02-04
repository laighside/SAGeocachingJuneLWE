/**
  @file    manage_subscription.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/mailing_list/manage_subscription.cgi
  This page allows users to verify their email or unsubscribe from the mailing list.
  Links to this page are included in emails sent to users
   - Verify: /cgi-bin/mailing_list/manage_subscription.cgi?verify=...
   - Unsubscribe: /cgi-bin/mailing_list/manage_subscription.cgi?unsubscribe=...

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/KeyValueParser.h"

int main () {
    try {
        JlweCore jlwe;
        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        std::string verify_token = urlQueries.getValue("verify");
        std::string unsub_token = urlQueries.getValue("unsubscribe");
        std::string result = "Something has gone wrong. Please let us know at " + std::string(jlwe.config.at("adminEmail"));

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        HtmlTemplate html(true);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Mailing List - Manage Subscription", false))
            return 0;

        std::cout << "<h1>June LWE Mailing List</h1>\n";

        if (verify_token.size()) {
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT verifyEmail(?,?,?);");
            prep_stmt->setString(1, verify_token);
            prep_stmt->setString(2, jlwe.getCurrentUserIP());
            prep_stmt->setString(3, jlwe.getCurrentUsername());
            res = prep_stmt->executeQuery();
            if (res->next()) {
                int r = res->getInt(1);
                if (r == 1)
                    result = "Your email address has been successfully verified.";
                if (r == 0)
                    result = "Your email address was not found in our systems.";
            }
            delete res;
            delete prep_stmt;
        }
        if (unsub_token.size()) {
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT unsubEmail(?,?,?);");
            prep_stmt->setString(1, unsub_token);
            prep_stmt->setString(2, jlwe.getCurrentUserIP());
            prep_stmt->setString(3, jlwe.getCurrentUsername());
            res = prep_stmt->executeQuery();
            if (res->next()) {
                int r = res->getInt(1);
                if (r == 1)
                    result = "Your email address has been successfully unsubscribed.";
                if (r == 0)
                    result = "Your email address was not found in our systems.";
            }
            delete res;
            delete prep_stmt;
        }

        std::cout << "<p>" << result << "</p>\n";

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
