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

#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/JsonUtils.h"

#include "../ext/nlohmann/json.hpp"

struct menu_item {
    int id;
    std::string name;
    std::string name_plural;
    int price;
};

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        std::vector<menu_item> menu_items;

        stmt = jlwe.getMysqlCon()->createStatement();
        res = stmt->executeQuery("SELECT id,name,name_plural,price FROM dinner_menu ORDER BY id;");
        while (res->next()) {
            menu_items.push_back({res->getInt(1), res->getString(2), res->getString(3), res->getInt(4)});
        }
        delete res;
        delete stmt;

        nlohmann::json jsonDinner;
        jsonDinner["items"] = nlohmann::json::array();

        for (unsigned int i = 0; i < menu_items.size(); i++) {
            menu_item m = menu_items.at(i);

            nlohmann::json jsonItem;

            jsonItem["id"] = m.id;
            jsonItem["name"] = m.name;
            jsonItem["name_plural"] = m.name_plural;
            jsonItem["price"] = m.price;
            jsonItem["options"] = nlohmann::json::array();

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT id,name,question,option_type,option_values FROM dinner_menu_options WHERE menu_item_id = ? ORDER BY display_order;");
            prep_stmt->setInt(1, m.id);
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

                jsonItem["options"].push_back(jsonOption);
            }
            delete res;
            delete prep_stmt;

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
