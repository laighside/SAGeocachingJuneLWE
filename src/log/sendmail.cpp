/**
  @file    sendmail.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/log/sendmail.cgi
  This displays the sendmail log. Useful for debugging email issues.
  Requires that the www-data user has read permissions to the mail log file.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"

#define MAIL_LOG "/var/log/mail.log"

int main () {
    try {
        JlweCore jlwe;

        if (jlwe.isLoggedIn()) { //if logged in

            // output header
            std::cout << "Content-type:text/plain\r\n";
            std::cout << "Content-Disposition: inline; filename=mail.log\r\n\r\n";

            std::cout << JlweUtils::readFileToString(MAIL_LOG);

        } else { //not logged in
            std::cout << "Content-type:text/plain\r\n\r\n";
            std::cout << "You need to be logged in to view this area.\n";
        }
    } catch (sql::SQLException &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what() << " (MySQL error code: " << std::to_string(e.getErrorCode()) << ")\n";
    } catch (const std::exception &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what();
    }

    return 0;
}
