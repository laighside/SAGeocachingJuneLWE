/**
  @file    RegistrationXLS.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Functions for creating an XLSX (Excel) file containing the list of registrations, camping and dinner orders
  All functions are static so there is no need to create instances of the RegistrationXLS object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef REGISTRATIONXLS_H
#define REGISTRATIONXLS_H

#include <string>

#include "../core/JlweCore.h"

#include "../ext/OpenXLSX/OpenXLSX.hpp"

class RegistrationXLS
{
public:

    /*!
     * \brief Makes the XLSX file
     *
     * No return value, will throw an error if something goes wrong
     *
     * \param filename The file to write the XLSX file to
     * \param jlwe JlweCore object
     * \param full true = full version, false = simple version
     */
    static void makeRegistrationXLS(const std::string &filename, JlweCore *jlwe, bool full = true);

private:

    struct cacheLog {
        int year;
        std::string type;
        std::string finder;
    };

    /*!
     * \brief Converts the day of week from a number to a name
     *
     * eg. 0 -> "Sunday", 1 -> "Monday", etc.
     *
     * \param wday The number of the day, starting at 0=Sunday
     * \return The name of the day as a string
     */
    static std::string wdayToName(int wday);

    // These functions are all for working out if people have been to past events or not
    // Used to work out if they get a lanyard or not
    static bool hasLanyardYear(std::vector<int> &years);
    static std::vector<cacheLog> readLogsFromGPXfile(const std::string &filename);
    static std::vector<int> searchEventLogsForName(const std::vector<cacheLog> &logs, const std::string &username);

    // These functions fill in the Excel worksheets with the registration data
    static void makeEventSheet(OpenXLSX::XLWorksheet &sheet, sql::Connection *con, bool full, std::string events_gpx);
    static void makeCampingSheet(OpenXLSX::XLWorksheet &sheet, sql::Connection *con, bool full, time_t jlwe_date);
    static void makeDinnerSheet(OpenXLSX::XLWorksheet &sheet, sql::Connection *con, bool full = true);
};

#endif // REGISTRATIONXLS_H

