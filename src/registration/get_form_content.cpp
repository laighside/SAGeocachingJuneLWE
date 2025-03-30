/**
  @file    get_form_content.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/registration/get_form_content.cgi
  Returns the content for the registration form (dinner menu, camping details, etc.)
  GET requests, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/JsonUtils.h"
#include "../core/KeyValueParser.h"

#include "../ext/nlohmann/json.hpp"

struct menu_item {
    int id;
    std::string name;
    std::string name_plural;
    bool null_name_plural; // hack to store null values in name_plural
    int course_id;
    bool null_course_id; // hack to store null values in course
    int meal_category_id;
    bool null_meal_category_id; // hack to store null values in meal_category
    int price;
};

nlohmann::json getDinnerOptions(sql::Connection * con, int meal_id = 0, int category_id = 0) {
    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;

    nlohmann::json options = nlohmann::json::array();

    if (meal_id) {
        prep_stmt = con->prepareStatement("SELECT id,name,question,option_type,option_values FROM dinner_menu_options WHERE menu_item_id = ? ORDER BY display_order;");
        prep_stmt->setInt(1, meal_id);
    } else if (category_id) {
        prep_stmt = con->prepareStatement("SELECT id,name,question,option_type,option_values FROM dinner_menu_options WHERE category_id = ? ORDER BY display_order;");
        prep_stmt->setInt(1, category_id);
    } else {
        return nlohmann::json::array();;
    }
    res = prep_stmt->executeQuery();
    while (res->next()) {
        nlohmann::json jsonOption;

        jsonOption["id"] = res->getInt(1);
        jsonOption["name"] = res->getString(2);
        jsonOption["question"] = res->getString(3);
        jsonOption["type"] = res->getString(4);
        if (res->isNull(5)) {
            jsonOption["values"] = nullptr;
        } else {
            jsonOption["values"] = JlweUtils::splitString(res->getString(5), ';');
        }

        options.push_back(jsonOption);
    }
    delete res;
    delete prep_stmt;

    return options;
}

int main () {
    try {
        JlweCore jlwe;
        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        std::vector<menu_item> menu_items;

        int dinner_id = 0;
        try {
            dinner_id = std::stoi(urlQueries.getValue("dinner_id"));
        } catch (...) {}

        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT id,name,name_plural,course_id,meal_category_id,price FROM dinner_menu WHERE dinner_form_id = ? ORDER BY id;");
        prep_stmt->setInt(1, dinner_id);
        res = prep_stmt->executeQuery();
        while (res->next()) {
            menu_items.push_back({res->getInt(1), res->getString(2), res->getString(3), res->isNull(3), res->getInt(4), res->isNull(4), res->getInt(5), res->isNull(5), res->getInt(6)});
        }
        delete res;
        delete prep_stmt;

        nlohmann::json jsonDinner;
        jsonDinner["items"] = nlohmann::json::array();

        jsonDinner["config"] = nullptr;
        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT config FROM dinner_forms WHERE dinner_id = ?;");
        prep_stmt->setInt(1, dinner_id);
        res = prep_stmt->executeQuery();
        if (res->next()) {
            if (res->isNull(1) == false)
                jsonDinner["config"] = nlohmann::json::parse(std::string(res->getString(1)));
        }
        delete res;
        delete prep_stmt;

        // new format options
        if (jsonDinner.contains("config") && jsonDinner.at("config").is_object() && jsonDinner.at("config").contains("categories") && jsonDinner.at("config").at("categories").is_array()) {
            for (nlohmann::json::iterator it = jsonDinner.at("config").at("categories").begin(); it != jsonDinner.at("config").at("categories").end(); ++it) {
                int category_id = it.value().at("id");
                it.value()["options"] = getDinnerOptions(jlwe.getMysqlCon(), 0, category_id);
            }
        }

        // old format options
        for (unsigned int i = 0; i < menu_items.size(); i++) {
            menu_item m = menu_items.at(i);

            nlohmann::json jsonItem;

            jsonItem["id"] = m.id;
            jsonItem["name"] = m.name;
            if (m.null_name_plural) jsonItem["name_plural"] = nullptr; else jsonItem["name_plural"] = m.name_plural;
            if (m.null_course_id) jsonItem["course_id"] = nullptr; else jsonItem["course_id"] = m.course_id;
            if (m.null_meal_category_id) jsonItem["meal_category_id"] = nullptr; else jsonItem["meal_category_id"] = m.meal_category_id;
            jsonItem["price"] = m.price;

            jsonItem["options"] = getDinnerOptions(jlwe.getMysqlCon(), m.id, 0);

            jsonDinner["items"].push_back(jsonItem);
        }

        nlohmann::json jsonDocument;
        jsonDocument["dinner"] = jsonDinner;
        jsonDocument["camping"] = nlohmann::json::object();

        std::cout << JsonUtils::makeJsonHeader() + jsonDocument.dump();

    } catch (sql::SQLException &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")");
    } catch( const std::exception &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()));
    }

    return 0;
}
