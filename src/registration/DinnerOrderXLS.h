/**
  @file    DinnerOrderXLS.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Functions for creating an XLSX (Excel) file containing the list of dinner orders and totals of each menu item
  All functions are static so there is no need to create instances of the DinnerOrderXLS object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef DINNERORDERXLS_H
#define DINNERORDERXLS_H

#include <string>

#include "../core/JlweCore.h"

#include "../ext/OpenXLSX/OpenXLSX.hpp"

class DinnerOrderXLS
{
public:

    /*!
     * \brief Makes the XLSX file
     *
     * No return value, will throw an error if something goes wrong
     *
     * \param filename The file to write the XLSX file to
     * \param jlwe JlweCore object
     */
    static void makeDinnerOrderXLS(const std::string &filename, JlweCore *jlwe);

private:

    // These functions fill in the Excel worksheets with the registration data
    static void makeOrdersSheet(OpenXLSX::XLWorksheet &sheet, sql::Connection *con, int dinner_form_id, int category_id, nlohmann::json configJson);
    static void makeTotalsSheet(OpenXLSX::XLWorksheet &sheet, sql::Connection *con);
};

#endif // DINNERORDERXLS_H
