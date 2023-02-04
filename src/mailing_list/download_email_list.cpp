/**
  @file    download_email_list.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/mailing_list/download_email_list.cgi
  This page allows the list of emails to be downloaded then copy/pasted into Gmail, Outlook, etc.
  This can be used as a backup in case the mailing list fails for any reason.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/JlweCore.h"

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_email")) { //if logged in
            //output header
            std::cout << "Content-type:text/plain\r\n";
            std::cout << "Content-Disposition: attachment; filename=email_list.txt\r\n\r\n";

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT email FROM email_list WHERE verify = 1;");
            while (res->next()) {
                std::cout << res->getString(1) << ";";
            }
            delete res;
            delete stmt;
        } else { // not logged in
            std::cout << "Content-type:text/plain\r\n\r\n";
            std::cout << "You need to be logged in to view this area.\n";
        }
    } catch (const sql::SQLException &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what() << " (MySQL error code: " << std::to_string(e.getErrorCode()) << ")\n";
    } catch (const std::exception &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what();
    }

    return 0;
}
