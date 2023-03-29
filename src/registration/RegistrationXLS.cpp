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
#include "RegistrationXLS.h"
#include <ctime>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/JlweUtils.h"
#include "../core/KeyValueParser.h"
#include "../core/PaymentUtils.h"

#include "../ext/pugixml/pugixml.hpp"

// this will return true for years 2018 or later
bool RegistrationXLS::hasLanyardYear(std::vector<int> &years) {
    for (unsigned int i = 0; i < years.size(); i++) {
        if (years.at(i) >= 2018)
            return true;
    }
    return false;
}

std::string RegistrationXLS::wdayToName(int wday) {
    if (wday == 0) return "Sunday";
    if (wday == 1) return "Monday";
    if (wday == 2) return "Tuesday";
    if (wday == 3) return "Wednesday";
    if (wday == 4) return "Thursday";
    if (wday == 5) return "Friday";
    if (wday == 6) return "Saturday";
    return "";
}

std::vector<RegistrationXLS::cacheLog> RegistrationXLS::readLogsFromGPXfile(const std::string &filename) {
    pugi::xml_document doc;
    pugi::xml_parse_result parse_result = doc.load_file(filename.c_str());
    if (!parse_result)
        return {};

    std::vector<cacheLog> result;

    pugi::xml_node wpts = doc.child("gpx");
    for (pugi::xml_named_node_iterator cache_it = wpts.children("wpt").begin(); cache_it != wpts.children("wpt").end(); ++cache_it) {

        std::string time_str(cache_it->child_value("time"));
        int year = 0;
        try{
            year = std::stoi(time_str.substr(0, 4));
        }catch (...) {}

        pugi::xml_node cache_logs = cache_it->child("groundspeak:cache").child("groundspeak:logs");
        for (pugi::xml_named_node_iterator log_it = cache_logs.children("groundspeak:log").begin(); log_it != cache_logs.children("groundspeak:log").end(); ++log_it) {

            std::string log_type(log_it->child_value("groundspeak:type"));
            std::string finder(log_it->child_value("groundspeak:finder"));

            if (JlweUtils::compareStringsNoCase(log_type, "attended") || JlweUtils::compareStringsNoCase(log_type, "found it"))
                result.push_back({year, log_type, finder});

        }
    }

    return result;
}

std::vector<int> RegistrationXLS::searchEventLogsForName(const std::vector<cacheLog> &logs, const std::string &username) {
    std::vector<int> result;
    for (unsigned int i = 0; i < logs.size(); i++) {
        if (JlweUtils::compareTeamNames(logs.at(i).finder, username)) {
            result.push_back(logs.at(i).year);
        }
    }
    return result;
}

void RegistrationXLS::makeEventSheet(OpenXLSX::XLWorksheet &sheet, sql::Connection *con, bool full, std::string events_gpx) {

    std::vector<cacheLog> cache_logs = readLogsFromGPXfile(events_gpx);

    int colID = 1;
    if (full) {
        sheet.cell(1, colID++).value() = "Timestamp (UTC)";
        sheet.cell(1, colID++).value() = "IP";
        sheet.cell(1, colID++).value() = "ID";
        sheet.cell(1, colID++).value() = "Key";
    }
    sheet.cell(1, colID++).value() = "Email";
    sheet.cell(1, colID++).value() = "Username";
    sheet.cell(1, colID++).value() = "Phone number";
    sheet.cell(1, colID++).value() = "Names (Adults)";
    sheet.cell(1, colID++).value() = "Names (Children)";
    sheet.cell(1, colID++).value() = "Number of Adults";
    sheet.cell(1, colID++).value() = "Number of Children";
    sheet.cell(1, colID++).value() = "Been to JLWE before?";
    sheet.cell(1, colID++).value() = "Have Lanyard?";
    sheet.cell(1, colID++).value() = "Logged 2018/2019";
    if (full) {
        sheet.cell(1, colID++).value() = "Camping?";
        sheet.cell(1, colID++).value() = "Dinner?";
        sheet.cell(1, colID++).value() = "Payment type";
    }
    sheet.cell(1, colID++).value() = "Cost";
    if (full) {
        sheet.cell(1, colID++).value() = "Payment received";
        sheet.cell(1, colID++).value() = "cs_id";
        sheet.cell(1, colID++).value() = "payment_intent";
    } else {
        sheet.cell(1, colID++).value() = "Paid";
    }

    int rowID = 2;
    sql::Statement *stmt = con->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT UNIX_TIMESTAMP(timestamp),IP_address,registration_id,idempotency,email_address,gc_username,phone_number,real_names_adults,real_names_children,number_adults,number_children,past_jlwe,have_lanyard,camping,dinner,payment_type,stripe_session_id FROM event_registrations WHERE status = 'S';");
    while (res->next()){
        std::string userKey = res->getString(4);
        int cost_total = PaymentUtils::getUserCost(con, userKey);
        int payment_received = PaymentUtils::getTotalPaymentReceived(con, userKey);
        std::string gc_username = res->getString(6);
        std::vector<int> years = searchEventLogsForName(cache_logs, gc_username);

        colID = 1;

        if (full) {
            //double excelTime = static_cast<double>(res->getInt64(1)) / 86400 + 25569; // convert to excel format
            sheet.cell(rowID, colID++).value() = OpenXLSX::XLDateTime(res->getInt64(1));    // Date/time
            sheet.cell(rowID, colID++).value() = res->getString(2);                  // IP
            sheet.cell(rowID, colID++).value() = res->getInt(3);                     // ID
            sheet.cell(rowID, colID++).value() = userKey;                            // userKey
        }
        sheet.cell(rowID, colID++).value() = res->getString(5);                  // email
        sheet.cell(rowID, colID++).value() = gc_username;                        // username
        sheet.cell(rowID, colID++).value() = res->getString(7);                  // phone
        sheet.cell(rowID, colID++).value() = res->getString(8);                  // names (adult)
        sheet.cell(rowID, colID++).value() = res->getString(9);                  // names (children)
        sheet.cell(rowID, colID++).value() = res->getInt(10);                    // number of adults
        sheet.cell(rowID, colID++).value() = res->getInt(11);                    // number of children
        sheet.cell(rowID, colID++).value() = (res->getInt(12) ? "Yes" : "No");   // past jlwe
        sheet.cell(rowID, colID++).value() = (res->getInt(13) ? "Yes" : "No");   // have lanyard

        if (years.size()) {
            if (hasLanyardYear(years)) {
                sheet.cell(rowID, colID++).value() = "Yes";
            } else {
                sheet.cell(rowID, colID++).value() = "No";
            }
        } else {
            sheet.cell(rowID, colID++).value() = "Newbie";
        }

        if (full) {
            sheet.cell(rowID, colID++).value() = res->getString(14);                           // camping
            sheet.cell(rowID, colID++).value() = res->getString(15);                           // dinner
            sheet.cell(rowID, colID++).value() = res->getString(16);                           // payment type
        }
        sheet.cell(rowID, colID++).value() = static_cast<double>(cost_total) / 100;        // cost
        if (full) {
            sheet.cell(rowID, colID++).value() = static_cast<double>(payment_received) / 100;  // payment recived

            std::string cs_id = res->getString(17);
            sheet.cell(rowID, colID++).value() = cs_id;
            if (cs_id.size()) {
                sql::PreparedStatement *prep_stmt = con->prepareStatement("SELECT payment_intent FROM stripe_event_log WHERE cs_id = ?;");
                prep_stmt->setString(1, cs_id);
                sql::ResultSet *res2 = prep_stmt->executeQuery();
                if (res2->next()){
                    sheet.cell(rowID, colID++).value() = res2->getString(1);
                }
                delete res2;
                delete prep_stmt;
            }
        } else {
            sheet.cell(rowID, colID++).value() = (payment_received >= cost_total ? "Yes" : "No");   // has paid
        }

        rowID++;
    }
    delete res;
    delete stmt;

}

void RegistrationXLS::makeCampingSheet(OpenXLSX::XLWorksheet &sheet, sql::Connection *con, bool full, time_t jlwe_date) {

    int colID = 1;
    if (full) {
        sheet.cell(1, colID++).value() = "Timestamp (UTC)";
        sheet.cell(1, colID++).value() = "IP";
        sheet.cell(1, colID++).value() = "ID";
        sheet.cell(1, colID++).value() = "Key";
    }
    sheet.cell(1, colID++).value() = "Email";
    sheet.cell(1, colID++).value() = "Username";
    sheet.cell(1, colID++).value() = "Phone number";
    sheet.cell(1, colID++).value() = "Type";
    sheet.cell(1, colID++).value() = "Number of People";
    sheet.cell(1, colID++).value() = "Arrive Date";
    sheet.cell(1, colID++).value() = "Leave Date";
    sheet.cell(1, colID++).value() = "Comments";
    if (full) {
        sheet.cell(1, colID++).value() = "Payment type";
    }
    sheet.cell(1, colID++).value() = "Cost";
    if (full) {
        sheet.cell(1, colID++).value() = "Payment received";
        sheet.cell(1, colID++).value() = "cs_id";
        sheet.cell(1, colID++).value() = "payment_intent";
    } else {
        sheet.cell(1, colID++).value() = "Paid";
    }

    int rowID = 2;
    sql::Statement *stmt = con->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT UNIX_TIMESTAMP(timestamp),IP_address,registration_id,idempotency,email_address,gc_username,phone_number,camping_type,number_people,arrive_date,leave_date,camping_comment,payment_type,stripe_session_id FROM camping WHERE status = 'S';");
    while (res->next()){
        std::string userKey = res->getString(4);
        int cost_total = PaymentUtils::getUserCost(con, userKey);
        int payment_received = PaymentUtils::getTotalPaymentReceived(con, userKey);

        colID = 1;

        if (full) {
            double excelTime = static_cast<double>(res->getInt64(1)) / 86400 + 25569; // convert to excel format
            sheet.cell(rowID, colID++).value() = OpenXLSX::XLDateTime(excelTime);    // Date/time
            sheet.cell(rowID, colID++).value() = res->getString(2);                  // IP
            sheet.cell(rowID, colID++).value() = res->getInt(3);                     // ID
            sheet.cell(rowID, colID++).value() = userKey;                            // userKey
        }
        sheet.cell(rowID, colID++).value() = res->getString(5);                  // email
        sheet.cell(rowID, colID++).value() = res->getString(6);                  // username
        sheet.cell(rowID, colID++).value() = res->getString(7);                  // phone
        sheet.cell(rowID, colID++).value() = res->getString(8);                  // type
        sheet.cell(rowID, colID++).value() = res->getInt(9);                     // number of people

        struct tm * jlwe_date_tm = gmtime(&jlwe_date);
        jlwe_date_tm->tm_mday = res->getInt(10);
        std::mktime(jlwe_date_tm);
        sheet.cell(rowID, colID++).value() = wdayToName(jlwe_date_tm->tm_wday) + " " + JlweUtils::numberToOrdinal(res->getInt(10));  // arrive

        jlwe_date_tm->tm_mday = res->getInt(11);
        std::mktime(jlwe_date_tm);
        sheet.cell(rowID, colID++).value() = wdayToName(jlwe_date_tm->tm_wday) + " " + JlweUtils::numberToOrdinal(res->getInt(11));  // leave

        sheet.cell(rowID, colID++).value() = res->getString(12);                 // comments

        if (full) {
            sheet.cell(rowID, colID++).value() = res->getString(13);                           // payment type
            sheet.cell(rowID, colID++).value() = static_cast<double>(cost_total) / 100;        // cost
        } else {
            if (res->getString(13) == "event") {
                colID++;
            } else {
                sheet.cell(rowID, colID++).value() = static_cast<double>(cost_total) / 100;        // cost
            }
        }
        if (full) {
            sheet.cell(rowID, colID++).value() = static_cast<double>(payment_received) / 100;  // payment recived

            std::string cs_id = res->getString(14);
            sheet.cell(rowID, colID++).value() = cs_id;
            if (cs_id.size()) {
                sql::PreparedStatement *prep_stmt = con->prepareStatement("SELECT payment_intent FROM stripe_event_log WHERE cs_id = ?;");
                prep_stmt->setString(1, cs_id);
                sql::ResultSet *res2 = prep_stmt->executeQuery();
                if (res2->next()){
                    sheet.cell(rowID, colID++).value() = res2->getString(1);
                }
                delete res2;
                delete prep_stmt;
            }
        } else {
            sheet.cell(rowID, colID++).value() = (payment_received >= cost_total ? "Yes" : "No");   // has paid
        }

        rowID++;
    }
    delete res;
    delete stmt;

}

void RegistrationXLS::makeDinnerSheet(OpenXLSX::XLWorksheet &sheet, sql::Connection *con, bool full) {

    int colID = 1;
    if (full) {
        sheet.cell(1, colID++).value() = "Timestamp (UTC)";
        sheet.cell(1, colID++).value() = "IP";
        sheet.cell(1, colID++).value() = "ID";
        sheet.cell(1, colID++).value() = "Key";
    }
    sheet.cell(1, colID++).value() = "Email";
    sheet.cell(1, colID++).value() = "Username";
    sheet.cell(1, colID++).value() = "Phone number";
    sheet.cell(1, colID++).value() = "Number of Adult meals";
    sheet.cell(1, colID++).value() = "Number of Child meals";
    sheet.cell(1, colID++).value() = "Comments";
    if (full) {
        sheet.cell(1, colID++).value() = "Payment type";
    }
    sheet.cell(1, colID++).value() = "Cost";
    if (full) {
        sheet.cell(1, colID++).value() = "Payment received";
        sheet.cell(1, colID++).value() = "cs_id";
        sheet.cell(1, colID++).value() = "payment_intent";
    } else {
        sheet.cell(1, colID++).value() = "Paid";
    }

    colID++;
    /*sheet.cell(1, colID++).value() = "Adult Op1 (Chicken Schnitzel)";
    sheet.cell(1, colID++).value() = "Adult Op2 (Beef Schnitzel)";
    sheet.cell(1, colID++).value() = "Adult Op3 (Beer Battered fish)";
    sheet.cell(1, colID++).value() = "Child Op1 (Chicken Schnitzel)";
    sheet.cell(1, colID++).value() = "Child Op2 (Beef Schnitzel)";
    sheet.cell(1, colID++).value() = "Child Op3 (6 Nuggets)";
    sheet.cell(1, colID++).value() = "Dessert Op1 (Chocolate mousse)";
    sheet.cell(1, colID++).value() = "Dessert Op2 (Eton Mess)";
    sheet.cell(1, colID++).value() = "Dessert Op3 (Fruit salad)";*/


    int rowID = 2;
    sql::Statement *stmt = con->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT UNIX_TIMESTAMP(timestamp),IP_address,registration_id,idempotency,email_address,gc_username,phone_number,number_adults,number_children,dinner_comment,payment_type,stripe_session_id, dinner_options_adults, dinner_options_children FROM sat_dinner WHERE status = 'S';");
    while (res->next()){
        std::string userKey = res->getString(4);
        int cost_total = PaymentUtils::getUserCost(con, userKey);
        int payment_received = PaymentUtils::getTotalPaymentReceived(con, userKey);

        colID = 1;

        if (full) {
            double excelTime = static_cast<double>(res->getInt64(1)) / 86400 + 25569; // convert to excel format
            sheet.cell(rowID, colID++).value() = OpenXLSX::XLDateTime(excelTime);    // Date/time
            sheet.cell(rowID, colID++).value() = res->getString(2);                  // IP
            sheet.cell(rowID, colID++).value() = res->getInt(3);                     // ID
            sheet.cell(rowID, colID++).value() = userKey;                            // userKey
        }
        sheet.cell(rowID, colID++).value() = res->getString(5);                  // email
        sheet.cell(rowID, colID++).value() = res->getString(6);                  // username
        sheet.cell(rowID, colID++).value() = res->getString(7);                  // phone
        sheet.cell(rowID, colID++).value() = res->getInt(8);                     // number of adults
        sheet.cell(rowID, colID++).value() = res->getInt(9);                     // number of children
        sheet.cell(rowID, colID++).value() = res->getString(10);                 // comment

        if (full) {
            sheet.cell(rowID, colID++).value() = res->getString(11);                           // payment type
            sheet.cell(rowID, colID++).value() = static_cast<double>(cost_total) / 100;        // cost
        } else {
            if (res->getString(11) == "event") {
                colID++;
            } else {
                sheet.cell(rowID, colID++).value() = static_cast<double>(cost_total) / 100;        // cost
            }
        }
        if (full) {
            sheet.cell(rowID, colID++).value() = static_cast<double>(payment_received) / 100;  // payment recived

            std::string cs_id = res->getString(12);
            sheet.cell(rowID, colID++).value() = cs_id;
            bool hasPi = false;
            if (cs_id.size()) {
                sql::PreparedStatement *prep_stmt = con->prepareStatement("SELECT payment_intent FROM stripe_event_log WHERE cs_id = ?;");
                prep_stmt->setString(1, cs_id);
                sql::ResultSet *res2 = prep_stmt->executeQuery();
                if (res2->next()){
                    sheet.cell(rowID, colID++).value() = res2->getString(1);
                    hasPi = true;
                }
                delete res2;
                delete prep_stmt;
            }
            if (!hasPi)
                colID++;
        } else {
            sheet.cell(rowID, colID++).value() = (payment_received >= cost_total ? "Yes" : "No");   // has paid
        }

        colID++;
        /*sheet.cell(rowID, colID++).value() = res->getInt(13);
        sheet.cell(rowID, colID++).value() = res->getInt(14);
        sheet.cell(rowID, colID++).value() = res->getInt(15);
        sheet.cell(rowID, colID++).value() = res->getInt(16);
        sheet.cell(rowID, colID++).value() = res->getInt(17);
        sheet.cell(rowID, colID++).value() = res->getInt(18);
        sheet.cell(rowID, colID++).value() = res->getInt(19);
        sheet.cell(rowID, colID++).value() = res->getInt(20);
        sheet.cell(rowID, colID++).value() = res->getInt(21);*/

        rowID++;
    }
    delete res;
    delete stmt;

}

void RegistrationXLS::makeRegistrationXLS(const std::string &filename, JlweCore *jlwe, bool full) {

    std::string currentTime = JlweUtils::timeToW3CDTF(time(nullptr));

    std::string events_gpx = std::string(jlwe->config.at("files").at("directory")) + jlwe->getGlobalVar("event_caches_gpx");
    time_t jlwe_date = 0;
    try {
        jlwe_date = std::stoll(jlwe->getGlobalVar("jlwe_date"));
    } catch (...) {}

    OpenXLSX::XLDocument doc;
    doc.create(filename);
    doc.setProperty(OpenXLSX::XLProperty::Creator, jlwe->config.at("websiteDomain"));
    doc.setProperty(OpenXLSX::XLProperty::LastModifiedBy, jlwe->config.at("websiteDomain"));
    doc.setProperty(OpenXLSX::XLProperty::CreationDate, currentTime);
    doc.setProperty(OpenXLSX::XLProperty::ModificationDate, currentTime);

    OpenXLSX::XLWorkbook wbk = doc.workbook();

    wbk.addWorksheet("Event");
    wbk.addWorksheet("Camping");
    wbk.addWorksheet("Dinner");
    wbk.deleteSheet("Sheet1");

    OpenXLSX::XLWorksheet event_sheet = wbk.sheet("Event");
    makeEventSheet(event_sheet, jlwe->getMysqlCon(), full, events_gpx);
    OpenXLSX::XLWorksheet camping_sheet = wbk.sheet("Camping");
    makeCampingSheet(camping_sheet, jlwe->getMysqlCon(), full, jlwe_date);
    OpenXLSX::XLWorksheet dinner_sheet = wbk.sheet("Dinner");
    makeDinnerSheet(dinner_sheet, jlwe->getMysqlCon(), full);

    doc.save();
}
