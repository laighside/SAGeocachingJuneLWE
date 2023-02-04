/**
  @file    user.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/log/user.cgi
  This is the main log for the JLWE software. Displays the contents of the user_log table (from MySQL).

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
        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - User Log", false))
            return 0;

        if (jlwe.isLoggedIn()) { //if logged in
            html.outputAdminMenu();

            std::cout << "<h2 style=\"text-align:center\">User Log</h2>\n";

	    std::string log_text = "";
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT CONCAT(FROM_UNIXTIME(timestamp),'UTC, ', userIP,', ', username,', ', action) FROM user_log WHERE timestamp > UNIX_TIMESTAMP(DATE_SUB(NOW(), INTERVAL 1 YEAR)) ORDER BY timestamp DESC;");
            while (res->next()) {
		log_text += res->getString(1) + "\n";
            }
            delete res;
            delete stmt;
            std::cout << "<p style=\"text-align:center\"><textarea rows=\"50\" cols=\"100\" style=\"white-space:nowrap;width:100%;\">" << Encoder::htmlEntityEncode(log_text) << "</textarea></p>\n";
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
