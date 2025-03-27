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

#include "../ext/nlohmann/json.hpp"

struct menu_option {
    int id;
    std::string name;
    std::string type;
    std::vector<std::string> values;
    std::string meal_type;
    std::vector<int> value_counts;
};

void DinnerOrderXLS::makeOrdersSheet(OpenXLSX::XLWorksheet &sheet, sql::Connection *con, bool adult) {

    sql::Statement *stmt;
    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;

    std::vector<menu_option> meal_options;
    prep_stmt = con->prepareStatement("SELECT id,name,option_type,option_values FROM dinner_menu_options WHERE menu_item_id = ? ORDER BY display_order;");
    prep_stmt->setInt(1, adult ? 1 : 2);
    res = prep_stmt->executeQuery();
    while (res->next()) {
        meal_options.push_back({res->getInt(1), res->getString(2), res->getString(3), JlweUtils::splitString(res->getString(4), ';'), ""});
    }
    delete res;
    delete prep_stmt;

    int colID = 1;
    sheet.cell(1, colID++).value() = "Username";
    sheet.cell(1, colID++).value() = "Phone";
    sheet.cell(1, colID++).value() = "Meal type";
    sheet.cell(1, colID++).value() = "Comment";
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
    stmt = con->createStatement();
    if (adult) {
        res = stmt->executeQuery("SELECT gc_username,phone_number,dinner_comment,dinner_options_adults FROM sat_dinner WHERE status = 'S';");
    } else {
        res = stmt->executeQuery("SELECT gc_username,phone_number,dinner_comment,dinner_options_children FROM sat_dinner WHERE status = 'S';");
    }
    while (res->next()) {
        std::string username = res->getString(1);
        std::string phone = res->getString(2);
        std::string comment = res->getString(3);

        nlohmann::json jsonMeals = nlohmann::json::parse(std::string(res->getString(4)));
        for (nlohmann::json::iterator it = jsonMeals.begin(); it != jsonMeals.end(); ++it) {
            nlohmann::json meal = it.value();

            colID = 1;

            sheet.cell(rowID, colID++).value() = username;
            sheet.cell(rowID, colID++).value() = phone;
            sheet.cell(rowID, colID++).value() = adult ? "Adult" : "Child";
            sheet.cell(rowID, colID++).value() = comment;
            for (unsigned int i = 0; i < meal_options.size(); i++) {
                nlohmann::json jsonValue = meal[std::to_string(meal_options.at(i).id)];
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

            rowID++;
        }

    }
    delete res;
    delete stmt;

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

    wbk.addWorksheet("Adult Orders");
    wbk.addWorksheet("Child Orders");
    wbk.addWorksheet("Totals");
    wbk.deleteSheet("Sheet1");

    OpenXLSX::XLWorksheet adult_orders_sheet = wbk.sheet("Adult Orders");
    makeOrdersSheet(adult_orders_sheet, jlwe->getMysqlCon(), true);
    OpenXLSX::XLWorksheet child_orders_sheet = wbk.sheet("Child Orders");
    makeOrdersSheet(child_orders_sheet, jlwe->getMysqlCon(), false);
    OpenXLSX::XLWorksheet totals_sheet = wbk.sheet("Totals");
    makeTotalsSheet(totals_sheet, jlwe->getMysqlCon());

    doc.save();
}
