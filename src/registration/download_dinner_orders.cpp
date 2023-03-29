/**
  @file    download_dinner_orders.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the download at /cgi-bin/registration/download_dinner_orders.cgi
  Downloads the Excel file that contains the list of meals each person ordered, and total number of each meal type

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"

#include "DinnerOrderXLS.h"

int main () {
    try {
        JlweCore jlwe;

        if (jlwe.getPermissionValue("perm_registrations")) { //if logged in

            // make temp folder and filenames
            char dir_template[] = "/tmp/tmpdir.XXXXXX";
            char *dir_name = mkdtemp(dir_template);
            if (dir_name == nullptr)
                throw std::runtime_error("Unable to create temporary directory");

            std::string tmp_filename = std::string(dir_name) + "/dinner_orders.xlsx";

            DinnerOrderXLS::makeDinnerOrderXLS(tmp_filename, &jlwe);


            FILE *file = fopen(tmp_filename.c_str(), "rb");
            if (file) { //if file exists in filesystem
                //output header
                std::cout << "Content-type:application/vnd.openxmlformats-officedocument.spreadsheetml.sheet\r\n";
                std::cout << "Content-Disposition: attachment; filename=dinner_orders.xlsx\r\n\r\n";

                uint8_t buffer[1024];
                size_t size = 1024;
                while (size == 1024){
                    size = fread(buffer, 1, 1024, file);
                    std::cout.write((const char*)buffer, size);
                }
                fclose(file);
            } else {
                std::cout << "Content-type:text/plain\r\n\r\n";
                std::cout << "Error: unable to read xlsx file\n";
            }

            rmdir(dir_name);

        } else {
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
