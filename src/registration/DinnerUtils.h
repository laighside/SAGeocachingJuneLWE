/**
  @file    DinnerUtils.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions commonly used when working with dinner orders
  All functions are static so there is no need to create instances of the DinnerUtils object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef DINNERUTILS_H
#define DINNERUTILS_H

#include <string>
#include <vector>

#include <cppconn/connection.h>

class DinnerUtils {
public:

    /*!
     * \brief Object to represent a single dinner form
     */
    struct dinner_form {
        int dinner_id;
        std::string title;
        time_t order_close_time;
        std::string config;
    };

    /*!
     * \brief Object to represent a single dinner menu item
     */
    struct dinner_menu_item {
        int id;
        std::string name;
        std::string name_plural;
        int price;
    };

    /*!
     * \brief Gets a list of all the enabled dinner forms (the dinner_forms table)
     *
     * \param con MySQL connection object
     * \return A list of form details
     */
    static std::vector<dinner_form> getDinnerFormList(sql::Connection *con);

    /*!
     * \brief Gets a list of all the menu items for a given form (the dinner_menu table)
     *
     * \param con MySQL connection object
     * \param dinner_form_id The ID number of the from to get the items for
     * \return A list of menu item
     */
    static std::vector<dinner_menu_item> getDinnerMenuItems(sql::Connection *con, int dinner_form_id);

    /*!
     * \brief Gets the total cost for a given dinner order
     *
     * \param con MySQL connection object
     * \param dinner_form_id The ID number of the from the order came from
     * \param order_json The JSON (as a string) of the order details
     * \param menu_items List of all the menu items (optional, will call getDinnerMenuItems() if this isn't provided)
     * \return The cost (in cents)
     */
    static int getDinnerCost(sql::Connection *con, int dinner_form_id, std::string order_json, std::vector<dinner_menu_item> menu_items = {});

    /*!
     * \brief Gets the user's dinner order as a string, format is name followed by list of items ordered
     *
     * \param json_str The JSON (as a string) of the order details
     * \param menu_items List of all the menu items
     * \return The name and list of items ordered as a string
     */
    static std::string dinnerOptionsToString(std::string json_str, const std::vector<DinnerUtils::dinner_menu_item> &menu_items);

};

#endif // DINNERUTILS_H
