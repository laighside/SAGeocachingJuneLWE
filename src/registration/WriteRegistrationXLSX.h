/**
  @file    WriteRegistrationXLSX.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Class for creating a XLSX (Excel) file containing the registration data
  The XLSX file is built by modifying a template XLSX file, which is located in the "templates" directory
  The templates directory is set by the config file "ooxmlTemplatePath" setting

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef WRITEREGISTRATIONXLSX_H
#define WRITEREGISTRATIONXLSX_H
#include "../ooxml/WriteXLSX.h"
#include <ctime>

#include "../core/JlweCore.h"

class WriteRegistrationXLSX : public WriteXLSX
{
public:

    struct cacheLog {
        int year;
        std::string type;
        std::string finder;
    };

    /*!
     * \brief WriteRegistrationXLSX Constructor.
     *
     * \param template_dir Path to the template directory (this is copied to the tmp directory)
     */
    WriteRegistrationXLSX(const std::string &template_dir);

    /*!
     * \brief WriteRegistrationXLSX Destructor.
     */
    ~WriteRegistrationXLSX();

    /*!
     * \brief Creates the "Event Registrations" sheet
     *
     * \param con The MySQL connection object
     * \param full_mode Set to true to make a sheet with the full data, false to make a simplified sheet
     * \param cache_logs List of logs from previous JLWE events, used to work out who is a newbie
     */
    void addEventRegistrationsSheet(sql::Connection *con, bool full_mode = true, const std::vector<cacheLog> &cache_logs = {});

    /*!
     * \brief Creates the "Camping" sheet
     *
     * \param con The MySQL connection object
     * \param full_mode Set to true to make a sheet with the full data, false to make a simplified sheet
     * \param jlwe_date Date of the JLWE event
     */
    void addCampingSheet(sql::Connection *con, bool full_mode, time_t jlwe_date);

    /*!
     * \brief Creates a dinner event sheet
     *
     * \param con The MySQL connection object
     * \param dinner_form_id The ID number of the dinner event to make the sheet for
     * \param dinner_title The title of the sheet
     * \param full_mode Set to true to make a sheet with the full data, false to make a simplified sheet
     */
    void addDinnerSheet(sql::Connection *con, int dinner_form_id, const std::string &dinner_title, bool full_mode = true);

    /*!
     * \brief Get a list of logs from a GPX file
     *
     * \param filename Filename of the GPX file
     * \return A list of cache logs
     */
    static std::vector<cacheLog> readLogsFromGPXfile(const std::string &filename);

private:

    // List of the styles in the styles.xml file
    enum xlsx_style {
        NO_STYLE = 0,
        TITLE_BOLD = 1,
        DATE_TIME = 2,
        CURRENCY = 3,
        RED_BACKGROUND = 4,
        CURRENCY_RED = 5,
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
    static std::vector<int> searchEventLogsForName(const std::vector<cacheLog> &logs, const std::string &username);

};

#endif // WRITEREGISTRATIONXLSX_H
