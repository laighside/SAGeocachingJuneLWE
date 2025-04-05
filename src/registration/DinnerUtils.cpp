/**
  @file    DinnerUtils.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions commonly used when working with dinner orders
  All functions are static so there is no need to create instances of the DinnerUtils object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "DinnerUtils.h"

#include "../core/Encoder.h"
#include "../core/JlweUtils.h"

#include <mysql_driver.h>
#include <mysql_connection.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "../ext/nlohmann/json.hpp"

std::vector<DinnerUtils::dinner_form> DinnerUtils::getDinnerFormList(sql::Connection *con) {
    sql::Statement *stmt;
    sql::ResultSet *res;

    std::vector<dinner_form> dinner_forms;
    stmt = con->createStatement();
    res = stmt->executeQuery("SELECT dinner_id,title,unix_timestamp(order_close_time),config FROM dinner_forms WHERE enabled > 0;");
    while (res->next()) {
        dinner_forms.push_back({res->getInt(1), res->getString(2), res->getInt64(3), res->getString(4)});
    }
    delete res;
    delete stmt;
    return dinner_forms;
}

std::vector<DinnerUtils::dinner_menu_item> DinnerUtils::getDinnerMenuItems(sql::Connection *con, int dinner_form_id) {
    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;

    std::vector<dinner_menu_item> menu_items;
    prep_stmt = con->prepareStatement("SELECT id,name,name_plural,price FROM dinner_menu WHERE dinner_form_id = ?;");
    prep_stmt->setInt(1, dinner_form_id);
    res = prep_stmt->executeQuery();
    while (res->next()) {
        menu_items.push_back({res->getInt(1), res->getString(2), res->getString(3), res->getInt(4)});
    }
    delete res;
    delete prep_stmt;

    return menu_items;
}

int DinnerUtils::getDinnerCost(sql::Connection *con, int dinner_form_id, std::string order_json, std::vector<dinner_menu_item> menu_items) {
    if (!menu_items.size())
        menu_items = DinnerUtils::getDinnerMenuItems(con, dinner_form_id);

    int total_cost = 0;

    nlohmann::json jsonDocument = nlohmann::json::parse(order_json);

    // new format items
    if (jsonDocument.contains("categories") && jsonDocument.at("categories").is_object()) {
        for (nlohmann::json::iterator catagory_it = jsonDocument.at("categories").begin(); catagory_it != jsonDocument.at("categories").end(); ++catagory_it) {
            nlohmann::json meal_array = catagory_it.value();

            for (nlohmann::json::iterator meal_it = meal_array.begin(); meal_it != meal_array.end(); ++meal_it) {
                nlohmann::json meal = meal_it.value();

                for (nlohmann::json::iterator course_it = meal.at("courses").begin(); course_it != meal.at("courses").end(); ++course_it) {
                    int item_id = course_it.value();

                    if (item_id > 0) {
                        bool valid_item = false;
                        for (unsigned int i = 0; i < menu_items.size(); i++) {
                            if (menu_items.at(i).id == item_id) {
                                valid_item = true;
                                total_cost += menu_items.at(i).price;
                            }
                        }

                        if (!valid_item)
                            throw std::runtime_error("Menu item " + std::to_string(item_id) + " not found");
                    }

                }
            }
        }
    }

    // old format items
    if (jsonDocument.contains("meals") && jsonDocument.at("meals").is_object()) {
        for (nlohmann::json::iterator meal_it = jsonDocument.at("meals").begin(); meal_it != jsonDocument.at("meals").end(); ++meal_it) {
            nlohmann::json meal_array = meal_it.value();
            int meal_id = 0;
            try {
                meal_id = std::stoi(meal_it.key());
            } catch (...) {}

            bool valid_item = false;
            for (unsigned int i = 0; i < menu_items.size(); i++) {
                if (menu_items.at(i).id == meal_id) {
                    valid_item = true;
                    total_cost += menu_items.at(i).price * static_cast<int>(meal_array.size());
                }
            }

            if (!valid_item)
                throw std::runtime_error("Menu item " + meal_it.key() + " not found");
        }
    }

    return total_cost;
}

std::string DinnerUtils::dinnerOptionsToString(std::string json_str, const std::vector<DinnerUtils::dinner_menu_item> &menu_items) {
    std::string output = "";
    nlohmann::json json_object = nlohmann::json::parse(json_str);
    nlohmann::json category_array = nlohmann::json::array();
    if (json_object.contains("categories"))
        category_array = json_object.at("categories");
    if (json_object.contains("meals"))
        category_array = json_object.at("meals");

    for (nlohmann::json::iterator category_it = category_array.begin(); category_it != category_array.end(); ++category_it) {
        nlohmann::json meal_array = category_it.value();
        for (nlohmann::json::iterator meal_it = meal_array.begin(); meal_it != meal_array.end(); ++meal_it) {
            nlohmann::json meal = meal_it.value();
            std::string line_str = "";
            if (meal.contains("name") && std::string(meal.at("name")).size())
                line_str += std::string(meal.at("name")) + ": ";

            std::vector<std::string> item_list;
            if (meal.contains("courses")) {
                for (nlohmann::json::iterator courses_it = meal.at("courses").begin(); courses_it != meal.at("courses").end(); ++courses_it) {
                    int item_id = courses_it.value();
                    for (unsigned int i = 0; i < menu_items.size(); i++) {
                        if (menu_items.at(i).id == item_id) {
                            item_list.push_back(menu_items.at(i).name);
                        }
                    }
                }
            }
            if (meal.contains("item_options")) {
                for (nlohmann::json::iterator items_it = meal.at("item_options").begin(); items_it != meal.at("item_options").end(); ++items_it) {
                    nlohmann::json item = items_it.value();
                    if (item.is_string())
                        if (!JlweUtils::compareStringsNoCase(item, "none"))
                            item_list.push_back(item);
                }
            }

            for (unsigned int i = 0; i < item_list.size(); i++) {
                if (i != 0)
                    line_str += ", ";
                line_str += item_list.at(i);
            }

            if (line_str.size())
                output += Encoder::htmlEntityEncode(line_str) + "<br/>";
        }
    }

    return output;
}
