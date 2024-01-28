/**
  @file    server_status.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/log/server_status.cgi
  This displays the Apache server status page, but only to users that are logged in.
  To do this /server-status is configured to be only accessible by localhost. This page then makes the request and relays the data (if the user is logged in).

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>

#include "../core/JlweCore.h"
#include "../core/HttpRequest.h"

int main () {
    try {
        JlweCore jlwe;
        if (jlwe.getPermissionValue("perm_admin")) { //if logged in

            HttpRequest request("localhost/server-status");
            if (request.get()) {
                std::cout << "Content-type:text/html\r\n\r\n";
                std::cout << request.responseAsString();
            } else {
                std::cout << "Content-type:text/plain\r\n\r\n";
                std::cout << "Error getting server status.\n";
            }

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
