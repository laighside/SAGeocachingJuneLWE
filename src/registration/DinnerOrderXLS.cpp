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
#include "DinnerOrderXLS.h"
#include <ctime>
#include <vector>

#include "../core/JlweUtils.h"
#include "DinnerUtils.h"

#include "../ext/nlohmann/json.hpp"

struct menu_option {
    int id;
    std::string name;
    std::string type;
    std::vector<std::string> values;
    std::string meal_type;
    std::vector<int> value_counts;
};

void DinnerOrderXLS::makeOrdersSheet(OpenXLSX::XLWorksheet &sheet, sql::Connection *con, int dinner_form_id, int category_id, nlohmann::json configJson) {

    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;

    std::vector<menu_option> meal_options;
    prep_stmt = con->prepareStatement("SELECT id,name,option_type,option_values FROM dinner_menu_options WHERE menu_item_id = ? OR category_id = ? ORDER BY display_order;");
    prep_stmt->setInt(1, category_id);
    prep_stmt->setInt(2, category_id);
    res = prep_stmt->executeQuery();
    while (res->next()) {
        meal_options.push_back({res->getInt(1), res->getString(2), res->getString(3), JlweUtils::splitString(res->getString(4), ';'), ""});
    }
    delete res;
    delete prep_stmt;

    std::vector<DinnerUtils::dinner_menu_item> items = DinnerUtils::getDinnerMenuItems(con, dinner_form_id);

    int colID = 1;
    sheet.cell(1, colID++).value() = "Username";
    sheet.cell(1, colID++).value() = "Phone";
    sheet.cell(1, colID++).value() = "Name";
    sheet.cell(1, colID++).value() = "Comment";

    if (configJson.contains("courses")) {
        for (nlohmann::json::iterator it = configJson.at("courses").begin(); it != configJson.at("courses").end(); ++it) {
            if (it.value().value("category_id", 0) == category_id) {
                int course_id = it.value().at("id");
                sheet.cell(1, colID++).value() = it.value().value("name", std::to_string(course_id));
            }
        }
    }

    for (unsigned int i = 0; i < meal_options.size(); i++) {
        if (meal_options.at(i).type == "checkbox") {
            std::string option_name = meal_options.at(i).name;
            std::vector<std::string> option_values = meal_options.at(i).values;
            for (unsigned int j = 0; j < option_values.size(); j++) {
                sheet.cell(1, colID++).value() = option_name + " - " + option_values.at(j);
            }
        } else {
            sheet.cell(1, colID++).value() = meal_options.at(i).name;
        }
    }

    int rowID = 2;

    prep_stmt = con->prepareStatement("SELECT gc_username,phone_number,dinner_comment,dinner_options_adults FROM sat_dinner WHERE status = 'S' AND dinner_form_id = ?;");
    prep_stmt->setInt(1, dinner_form_id);
    res = prep_stmt->executeQuery();
    while (res->next()) {
        std::string username = res->getString(1);
        std::string phone = res->getString(2);
        std::string comment = res->getString(3);

        nlohmann::json json_object = nlohmann::json::parse(std::string(res->getString(4)));

        nlohmann::json meals_array = nlohmann::json::array();
        if (json_object.contains("meals") && json_object.at("meals").contains(std::to_string(category_id)))
            meals_array = json_object.at("meals").at(std::to_string(category_id));

        if (json_object.contains("categories") && json_object.at("categories").contains(std::to_string(category_id)))
            meals_array = json_object.at("categories").at(std::to_string(category_id));

        for (nlohmann::json::iterator it = meals_array.begin(); it != meals_array.end(); ++it) {
            nlohmann::json meal = it.value();

            colID = 1;

            sheet.cell(rowID, colID++).value() = username;
            sheet.cell(rowID, colID++).value() = phone;
            sheet.cell(rowID, colID++).value() = meal.value("name", "");
            sheet.cell(rowID, colID++).value() = comment;

            if (meal.contains("courses") && configJson.contains("courses")) {

                for (nlohmann::json::iterator course_it = configJson.at("courses").begin(); course_it != configJson.at("courses").end(); ++course_it) {
                    if (course_it.value().value("category_id", 0) == category_id) {

                        int course_id = course_it.value().at("id");
                        int item_id = meal.at("courses").value(std::to_string(course_id), -1);

                        bool valid_item = false;
                        if (item_id == 0) {
                            valid_item = true;
                            sheet.cell(rowID, colID++).value() = "none";
                        } else {
                            for (unsigned int i = 0; i < items.size(); i++) {
                                if (items.at(i).id == item_id) {
                                    valid_item = true;
                                    sheet.cell(rowID, colID++).value() = items.at(i).name;
                                }
                            }
                        }

                        if (!valid_item)
                            colID++;
                    }
                }

            }

            if (meal.contains("item_options")) {
                for (unsigned int i = 0; i < meal_options.size(); i++) {
                    nlohmann::json jsonValue = meal.at("item_options")[std::to_string(meal_options.at(i).id)];
                    if (meal_options.at(i).type == "checkbox") {
                        std::vector<std::string> option_values = meal_options.at(i).values;
                        if (jsonValue.is_array() && jsonValue.size() == option_values.size()) {
                            for (unsigned int j = 0; j < option_values.size(); j++) {
                                bool val = jsonValue.at(j);
                                sheet.cell(rowID, colID++).value() = val ? "Yes" : "No";
                            }
                        } else {
                            colID += option_values.size();
                        }
                    } else {
                        if (jsonValue.is_null()) {
                            sheet.cell(rowID, colID++).value() = "null";
                        } else if (jsonValue.is_string()) {
                            sheet.cell(rowID, colID++).value() = std::string(jsonValue);
                        } else {
                            colID++;
                        }
                    }
                }
            }

            rowID++;
        }

    }
    delete res;
    delete prep_stmt;
}

void DinnerOrderXLS::makeTotalsSheet(OpenXLSX::XLWorksheet &sheet, sql::Connection *con) {

    sql::Statement *stmt;
    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;

    std::vector<menu_option> meal_options;
    prep_stmt = con->prepareStatement("SELECT dinner_menu_options.id,dinner_menu_options.name,dinner_menu_options.option_type,dinner_menu_options.option_values,dinner_menu.name FROM dinner_menu_options INNER JOIN dinner_menu ON dinner_menu_options.menu_item_id=dinner_menu.id ORDER BY dinner_menu.id, dinner_menu_options.display_order;");
    res = prep_stmt->executeQuery();
    while (res->next()) {
        menu_option opt = {res->getInt(1), res->getString(2), res->getString(3), JlweUtils::splitString(res->getString(4), ';'), res->getString(5)};
        opt.value_counts = std::vector<int>(opt.values.size(), 0);
        if (opt.type == "select" || opt.type == "checkbox") // ignore inputs that don't have options
            meal_options.push_back(opt);
    }
    delete res;
    delete prep_stmt;


    stmt = con->createStatement();
    res = stmt->executeQuery("SELECT dinner_options_adults,dinner_options_children FROM sat_dinner WHERE status = 'S';");
    while (res->next()) {

        nlohmann::json jsonMealsAdults = nlohmann::json::parse(std::string(res->getString(1)));
        for (nlohmann::json::iterator it = jsonMealsAdults.begin(); it != jsonMealsAdults.end(); ++it) {
            nlohmann::json meal = it.value();

            for (unsigned int i = 0; i < meal_options.size(); i++) {
                nlohmann::json jsonValue = meal[std::to_string(meal_options.at(i).id)];
                if (meal_options.at(i).type == "checkbox") {
                    std::vector<std::string> option_values = meal_options.at(i).values;
                    if (jsonValue.is_array() && jsonValue.size() == option_values.size()) {
                        for (unsigned int j = 0; j < option_values.size(); j++) {
                            bool val = jsonValue.at(j);
                            if (val)
                                meal_options.at(i).value_counts.at(j)++;
                        }
                    }
                } else if (meal_options.at(i).type == "select") {
                    if (jsonValue.is_string()) {
                        std::string val = jsonValue;
                        std::vector<std::string> option_values = meal_options.at(i).values;
                        for (unsigned int j = 0; j < option_values.size(); j++) {
                            if (val == option_values.at(j))
                                meal_options.at(i).value_counts.at(j)++;
                        }
                    }
                }
            }
        }

        nlohmann::json jsonMealsChildren = nlohmann::json::parse(std::string(res->getString(2)));
        for (nlohmann::json::iterator it = jsonMealsChildren.begin(); it != jsonMealsChildren.end(); ++it) {
            nlohmann::json meal = it.value();

            for (unsigned int i = 0; i < meal_options.size(); i++) {
                nlohmann::json jsonValue = meal[std::to_string(meal_options.at(i).id)];
                if (meal_options.at(i).type == "checkbox") {
                    std::vector<std::string> option_values = meal_options.at(i).values;
                    if (jsonValue.is_array() && jsonValue.size() == option_values.size()) {
                        for (unsigned int j = 0; j < option_values.size(); j++) {
                            bool val = jsonValue.at(j);
                            if (val)
                                meal_options.at(i).value_counts.at(j)++;
                        }
                    }
                } else if (meal_options.at(i).type == "select") {
                    if (jsonValue.is_string()) {
                        std::string val = jsonValue;
                        std::vector<std::string> option_values = meal_options.at(i).values;
                        for (unsigned int j = 0; j < option_values.size(); j++) {
                            if (val == option_values.at(j))
                                meal_options.at(i).value_counts.at(j)++;
                        }
                    }
                }
            }
        }

    }
    delete res;
    delete stmt;

    int colID = 1;
    sheet.cell(1, colID++).value() = "Meal type";
    sheet.cell(1, colID++).value() = "Option";
    sheet.cell(1, colID++).value() = "Value";
    sheet.cell(1, colID++).value() = "Count";

    int rowID = 2;
    for (unsigned int i = 0; i < meal_options.size(); i++) {
        menu_option opt = meal_options.at(i);
        for (unsigned int j = 0; j < opt.values.size(); j++) {
            colID = 1;
            sheet.cell(rowID, colID++).value() = opt.meal_type;
            sheet.cell(rowID, colID++).value() = opt.name;
            sheet.cell(rowID, colID++).value() = opt.values.at(j);
            sheet.cell(rowID, colID++).value() = opt.value_counts.at(j);
            rowID++;
        }
    }
}

void DinnerOrderXLS::makeDinnerOrderXLS(const std::string &filename, JlweCore *jlwe) {

    std::string currentTime = JlweUtils::timeToW3CDTF(time(nullptr));

    OpenXLSX::XLDocument doc;
    doc.create(filename, true);
    doc.setProperty(OpenXLSX::XLProperty::Creator, jlwe->config.at("websiteDomain"));
    doc.setProperty(OpenXLSX::XLProperty::LastModifiedBy, jlwe->config.at("websiteDomain"));
    doc.setProperty(OpenXLSX::XLProperty::CreationDate, currentTime);
    doc.setProperty(OpenXLSX::XLProperty::ModificationDate, currentTime);

    OpenXLSX::XLWorkbook wbk = doc.workbook();

    // List of dinner forms
    std::vector<DinnerUtils::dinner_form> dinner_forms = DinnerUtils::getDinnerFormList(jlwe->getMysqlCon());
    for (unsigned int i = 0; i < dinner_forms.size(); i++) {
        nlohmann::json configJson = nlohmann::json::parse(dinner_forms.at(i).config);
        if (configJson.contains("categories")) {
            for (nlohmann::json::iterator category_it = configJson.at("categories").begin(); category_it != configJson.at("categories").end(); ++category_it) {
                int category_id = category_it.value().at("id");
                std::string category_name = category_it.value().value("name_plural", category_it.value().value("name", ""));

                std::string sheet_name = dinner_forms.at(i).title + ", " + category_name;
                wbk.addWorksheet(sheet_name);
                OpenXLSX::XLWorksheet sheet = wbk.sheet(sheet_name);
                makeOrdersSheet(sheet, jlwe->getMysqlCon(), dinner_forms.at(i).dinner_id, category_id, configJson);
            }
        } else {
            std::vector<DinnerUtils::dinner_menu_item> items = DinnerUtils::getDinnerMenuItems(jlwe->getMysqlCon(), dinner_forms.at(i).dinner_id);
            for (unsigned int j = 0; j < items.size(); j++) {
                int meal_id = items.at(j).id;
                std::string category_name = items.at(j).name_plural.size() ? items.at(j).name_plural : items.at(j).name;

                std::string sheet_name = dinner_forms.at(i).title + ", " + category_name;
                wbk.addWorksheet(sheet_name);
                OpenXLSX::XLWorksheet sheet = wbk.sheet(sheet_name);
                makeOrdersSheet(sheet, jlwe->getMysqlCon(), dinner_forms.at(i).dinner_id, meal_id, configJson);
            }
        }

    }

    //wbk.addWorksheet("Totals");
    //OpenXLSX::XLWorksheet totals_sheet = wbk.sheet("Totals");
    //makeTotalsSheet(totals_sheet, jlwe->getMysqlCon());

    wbk.deleteSheet("Sheet1");
    doc.save();
}
