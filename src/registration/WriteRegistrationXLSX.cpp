/**
  @file    WriteRegistrationXLSX.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Class for creating a XLSX (Excel) file containing the registration data
  The XLSX file is built by modifying a template XLSX file, which is located in the "templates" directory
  The templates directory is set by the config file "ooxmlTemplatePath" setting

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "WriteRegistrationXLSX.h"
#include <stdexcept>

#include "../core/Encoder.h"
#include "../core/JlweUtils.h"
#include "../core/PaymentUtils.h"

#include "../ext/pugixml/pugixml.hpp"

WriteRegistrationXLSX::WriteRegistrationXLSX(const std::string &template_dir) :
    WriteXLSX(template_dir + "/registration")
{
    // do nothing
}

WriteRegistrationXLSX::~WriteRegistrationXLSX() {
    // do nothing
}

std::vector<WriteRegistrationXLSX::cacheLog> WriteRegistrationXLSX::readLogsFromGPXfile(const std::string &filename) {
    pugi::xml_document doc;
    pugi::xml_parse_result parse_result = doc.load_file(filename.c_str());
    if (!parse_result)
        return {};

    std::vector<WriteRegistrationXLSX::cacheLog> result;

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

// this will return true for years 2018 or later
bool WriteRegistrationXLSX::hasLanyardYear(std::vector<int> &years) {
    for (unsigned int i = 0; i < years.size(); i++) {
        if (years.at(i) >= 2018)
            return true;
    }
    return false;
}

std::vector<int> WriteRegistrationXLSX::searchEventLogsForName(const std::vector<cacheLog> &logs, const std::string &username) {
    std::vector<int> result;
    for (unsigned int i = 0; i < logs.size(); i++) {
        if (JlweUtils::compareTeamNames(logs.at(i).finder, username)) {
            result.push_back(logs.at(i).year);
        }
    }
    return result;
}

std::string WriteRegistrationXLSX::wdayToName(int wday) {
    if (wday == 0) return "Sunday";
    if (wday == 1) return "Monday";
    if (wday == 2) return "Tuesday";
    if (wday == 3) return "Wednesday";
    if (wday == 4) return "Thursday";
    if (wday == 5) return "Friday";
    if (wday == 6) return "Saturday";
    return "";
}

void WriteRegistrationXLSX::addEventRegistrationsSheet(sql::Connection *con, bool full_mode, const std::vector<cacheLog> &cache_logs) {
    std::string sheetData = "";
    sheetData += "<sheetViews>\n";
    sheetData += "  <sheetView workbookViewId=\"0\">\n";
    sheetData += "    <pane ySplit=\"1\" topLeftCell=\"A2\" activePane=\"bottomLeft\" state=\"frozen\"/>\n";
    sheetData += "  </sheetView>\n";
    sheetData += "</sheetViews>\n";
    sheetData += "<cols>\n";
    if (full_mode) {
        sheetData += "  <col min=\"1\" max=\"1\" width=\"20\" customWidth=\"1\"/>\n";
        sheetData += "  <col min=\"5\" max=\"6\" width=\"20\" customWidth=\"1\"/>\n";
        sheetData += "  <col min=\"7\" max=\"7\" width=\"12\" customWidth=\"1\"/>\n";
    } else {
        sheetData += "  <col min=\"1\" max=\"2\" width=\"20\" customWidth=\"1\"/>\n";
        sheetData += "  <col min=\"3\" max=\"3\" width=\"12\" customWidth=\"1\"/>\n";
    }
    sheetData += "</cols>\n";

    sheetData += "<sheetData>\n";
    unsigned int rowId = 0;

    // Title row
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";

    unsigned int colId = 0;

    if (full_mode) {
        sheetData += makeStringCell(++colId, rowId, "Timestamp (UTC)", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "IP", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "ID", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "Key", TITLE_BOLD);
    }
    sheetData += makeStringCell(++colId, rowId, "Email", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Username", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Phone number", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Names (Adults)", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Names (Children)", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Number of Adults", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Number of Children", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Been to JLWE before?", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Have Lanyard?", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Logged 2018 or later", TITLE_BOLD);
    if (full_mode) {
        sheetData += makeStringCell(++colId, rowId, "Camping?", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "Dinner?", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "Payment type", TITLE_BOLD);
    }
    sheetData += makeStringCell(++colId, rowId, "Cost", TITLE_BOLD);
    if (full_mode) {
        sheetData += makeStringCell(++colId, rowId, "Payment received", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "cs_id", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "payment_intent", TITLE_BOLD);
    } else {
        sheetData += makeStringCell(++colId, rowId, "Paid", TITLE_BOLD);
    }

    sheetData += "</row>\n";

    sql::Statement *stmt = con->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT UNIX_TIMESTAMP(timestamp),IP_address,registration_id,idempotency,email_address,gc_username,phone_number,real_names_adults,real_names_children,number_adults,number_children,past_jlwe,have_lanyard,camping,dinner,payment_type,stripe_session_id FROM event_registrations WHERE status = 'S';");
    while (res->next()) {
        std::string userKey = res->getString(4);
        int cost_total = PaymentUtils::getUserCost(con, userKey);
        int payment_received = PaymentUtils::getTotalPaymentReceived(con, userKey);
        std::string gc_username = res->getString(6);
        std::vector<int> years = searchEventLogsForName(cache_logs, gc_username);

        sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
        colId = 0;

        if (full_mode) {
            sheetData += makeDateTimeCell(++colId, rowId, res->getInt64(1), DATE_TIME); // Date/time
            sheetData += makeStringCell(++colId, rowId, res->getString(2), NO_STYLE);   // IP
            sheetData += makeNumberCell(++colId, rowId, res->getInt(3), NO_STYLE);      // ID
            sheetData += makeStringCell(++colId, rowId, userKey, NO_STYLE);             // userKey
        }
        sheetData += makeStringCell(++colId, rowId, res->getString(5), NO_STYLE);    // email
        sheetData += makeStringCell(++colId, rowId, gc_username, NO_STYLE);          // username
        sheetData += makeStringCell(++colId, rowId, res->getString(7), NO_STYLE);    // phone
        sheetData += makeStringCell(++colId, rowId, res->getString(8), NO_STYLE);    // names (adult)
        sheetData += makeStringCell(++colId, rowId, res->getString(9), NO_STYLE);    // names (children)
        sheetData += makeNumberCell(++colId, rowId, res->getInt(10), NO_STYLE);      // number of adults
        sheetData += makeNumberCell(++colId, rowId, res->getInt(11), NO_STYLE);      // number of children
        sheetData += makeStringCell(++colId, rowId, (res->getInt(12) ? "Yes" : "No"), NO_STYLE);  // past jlwe
        sheetData += makeStringCell(++colId, rowId, (res->getInt(13) ? "Yes" : "No"), NO_STYLE);  // have lanyard
        if (cache_logs.size()) {
            if (years.size()) {
                if (this->hasLanyardYear(years)) {
                    sheetData += makeStringCell(++colId, rowId, "Yes", NO_STYLE);
                } else {
                    sheetData += makeStringCell(++colId, rowId, "No", NO_STYLE);
                }
            } else {
                sheetData += makeStringCell(++colId, rowId, "Newbie", NO_STYLE);
            }
        } else {
            sheetData += makeStringCell(++colId, rowId, "N/A", NO_STYLE);
        }

        if (full_mode) {
            sheetData += makeStringCell(++colId, rowId, res->getString(14), NO_STYLE);         // camping
            sheetData += makeStringCell(++colId, rowId, res->getString(15), NO_STYLE);         // dinner
            sheetData += makeStringCell(++colId, rowId, res->getString(16), NO_STYLE);         // payment type
        }
        sheetData += makeNumberCell(++colId, rowId, static_cast<double>(cost_total) / 100, CURRENCY);  // cost
        bool hasPaid = (payment_received >= cost_total);
        if (full_mode) {
            sheetData += makeNumberCell(++colId, rowId, static_cast<double>(payment_received) / 100, (hasPaid ? CURRENCY : CURRENCY_RED));  // payment recived

            std::string cs_id = res->getString(17);
            sheetData += makeStringCell(++colId, rowId, cs_id, NO_STYLE);
            if (cs_id.size()) {
                sql::PreparedStatement *prep_stmt = con->prepareStatement("SELECT payment_intent FROM stripe_event_log WHERE cs_id = ?;");
                prep_stmt->setString(1, cs_id);
                sql::ResultSet *res2 = prep_stmt->executeQuery();
                if (res2->next()){
                    sheetData += makeStringCell(++colId, rowId, res2->getString(1), NO_STYLE);
                }
                delete res2;
                delete prep_stmt;
            }
        } else {
            sheetData += makeStringCell(++colId, rowId, (hasPaid ? "Yes" : "No"), (hasPaid ? NO_STYLE : RED_BACKGROUND));  // has paid
        }

        sheetData += "</row>\n";
    }
    delete res;
    delete stmt;

    sheetData += "</sheetData>\n";

    this->addWorksheetFromXML(sheetData, "Event Registrations");
}

void WriteRegistrationXLSX::addCampingSheet(sql::Connection *con, bool full_mode, time_t jlwe_date) {
    std::string sheetData = "";
    sheetData += "<sheetViews>\n";
    sheetData += "  <sheetView workbookViewId=\"0\">\n";
    sheetData += "    <pane ySplit=\"1\" topLeftCell=\"A2\" activePane=\"bottomLeft\" state=\"frozen\"/>\n";
    sheetData += "  </sheetView>\n";
    sheetData += "</sheetViews>\n";
    sheetData += "<cols>\n";
    if (full_mode) {
        sheetData += "  <col min=\"1\" max=\"1\" width=\"20\" customWidth=\"1\"/>\n";
        sheetData += "  <col min=\"5\" max=\"6\" width=\"20\" customWidth=\"1\"/>\n";
        sheetData += "  <col min=\"7\" max=\"7\" width=\"12\" customWidth=\"1\"/>\n";
    } else {
        sheetData += "  <col min=\"1\" max=\"2\" width=\"20\" customWidth=\"1\"/>\n";
        sheetData += "  <col min=\"3\" max=\"3\" width=\"12\" customWidth=\"1\"/>\n";
    }
    sheetData += "</cols>\n";

    sheetData += "<sheetData>\n";
    unsigned int rowId = 0;

    // Title row
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";

    unsigned int colId = 0;

    if (full_mode) {
        sheetData += makeStringCell(++colId, rowId, "Timestamp (UTC)", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "IP", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "ID", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "Key", TITLE_BOLD);
    }
    sheetData += makeStringCell(++colId, rowId, "Email", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Username", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Phone number", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Type", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Number of People", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Arrive Date", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Leave Date", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Comments", TITLE_BOLD);
    if (full_mode) {
        sheetData += makeStringCell(++colId, rowId, "Payment type", TITLE_BOLD);
    }
    sheetData += makeStringCell(++colId, rowId, "Cost", TITLE_BOLD);
    if (full_mode) {
        sheetData += makeStringCell(++colId, rowId, "Payment received", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "cs_id", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "payment_intent", TITLE_BOLD);
    } else {
        sheetData += makeStringCell(++colId, rowId, "Paid", TITLE_BOLD);
    }

    sheetData += "</row>\n";

    sql::Statement *stmt = con->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT UNIX_TIMESTAMP(timestamp),IP_address,registration_id,idempotency,email_address,gc_username,phone_number,camping_type,number_people,arrive_date,leave_date,camping_comment,payment_type,stripe_session_id FROM camping WHERE status = 'S';");
    while (res->next()) {
        std::string userKey = res->getString(4);
        int cost_total = PaymentUtils::getUserCost(con, userKey);
        int payment_received = PaymentUtils::getTotalPaymentReceived(con, userKey);

        sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
        colId = 0;

        if (full_mode) {
            sheetData += makeDateTimeCell(++colId, rowId, res->getInt64(1), DATE_TIME); // Date/time
            sheetData += makeStringCell(++colId, rowId, res->getString(2), NO_STYLE);   // IP
            sheetData += makeNumberCell(++colId, rowId, res->getInt(3), NO_STYLE);      // ID
            sheetData += makeStringCell(++colId, rowId, userKey, NO_STYLE);             // userKey
        }
        sheetData += makeStringCell(++colId, rowId, res->getString(5), NO_STYLE);    // email
        sheetData += makeStringCell(++colId, rowId, res->getString(6), NO_STYLE);    // username
        sheetData += makeStringCell(++colId, rowId, res->getString(7), NO_STYLE);    // phone
        sheetData += makeStringCell(++colId, rowId, res->getString(8), NO_STYLE);    // type
        sheetData += makeNumberCell(++colId, rowId, res->getInt(9), NO_STYLE);       // number of people

        struct tm * jlwe_date_tm = gmtime(&jlwe_date);
        jlwe_date_tm->tm_mday = res->getInt(10);
        std::mktime(jlwe_date_tm);
        sheetData += makeStringCell(++colId, rowId, wdayToName(jlwe_date_tm->tm_wday) + " " + JlweUtils::numberToOrdinal(res->getInt(10)), NO_STYLE);  // arrive

        jlwe_date_tm->tm_mday = res->getInt(11);
        std::mktime(jlwe_date_tm);
        sheetData += makeStringCell(++colId, rowId, wdayToName(jlwe_date_tm->tm_wday) + " " + JlweUtils::numberToOrdinal(res->getInt(11)), NO_STYLE);  // leave

        sheetData += makeStringCell(++colId, rowId, res->getString(12), NO_STYLE);    // comments

        if (full_mode) {
            sheetData += makeStringCell(++colId, rowId, res->getString(13), NO_STYLE);    // payment type
        }
        if (res->getString(13) == "event") {
            sheetData += makeStringCell(++colId, rowId, "Inc. in rego", NO_STYLE);
        } else {
            sheetData += makeNumberCell(++colId, rowId, static_cast<double>(cost_total) / 100, CURRENCY);  // cost
        }
        bool hasPaid = (payment_received >= cost_total);
        if (full_mode) {
            sheetData += makeNumberCell(++colId, rowId, static_cast<double>(payment_received) / 100, (hasPaid ? CURRENCY : CURRENCY_RED));  // payment recived

            std::string cs_id = res->getString(14);
            sheetData += makeStringCell(++colId, rowId, cs_id, NO_STYLE);
            if (cs_id.size()) {
                sql::PreparedStatement *prep_stmt = con->prepareStatement("SELECT payment_intent FROM stripe_event_log WHERE cs_id = ?;");
                prep_stmt->setString(1, cs_id);
                sql::ResultSet *res2 = prep_stmt->executeQuery();
                if (res2->next()){
                    sheetData += makeStringCell(++colId, rowId, res2->getString(1), NO_STYLE);
                }
                delete res2;
                delete prep_stmt;
            }
        } else {
            sheetData += makeStringCell(++colId, rowId, (hasPaid ? "Yes" : "No"), (hasPaid ? NO_STYLE : RED_BACKGROUND));  // has paid
        }

        sheetData += "</row>\n";
    }
    delete res;
    delete stmt;

    sheetData += "</sheetData>\n";

    this->addWorksheetFromXML(sheetData, "Camping");
}

void WriteRegistrationXLSX::addDinnerSheet(sql::Connection *con, int dinner_form_id, const std::string &dinner_title, bool full_mode) {
    std::string sheetData = "";
    sheetData += "<sheetViews>\n";
    sheetData += "  <sheetView workbookViewId=\"0\">\n";
    sheetData += "    <pane ySplit=\"1\" topLeftCell=\"A2\" activePane=\"bottomLeft\" state=\"frozen\"/>\n";
    sheetData += "  </sheetView>\n";
    sheetData += "</sheetViews>\n";
    sheetData += "<cols>\n";
    if (full_mode) {
        sheetData += "  <col min=\"1\" max=\"1\" width=\"20\" customWidth=\"1\"/>\n";
        sheetData += "  <col min=\"5\" max=\"6\" width=\"20\" customWidth=\"1\"/>\n";
        sheetData += "  <col min=\"7\" max=\"7\" width=\"12\" customWidth=\"1\"/>\n";
    } else {
        sheetData += "  <col min=\"1\" max=\"2\" width=\"20\" customWidth=\"1\"/>\n";
        sheetData += "  <col min=\"3\" max=\"3\" width=\"12\" customWidth=\"1\"/>\n";
    }
    sheetData += "</cols>\n";

    sheetData += "<sheetData>\n";
    unsigned int rowId = 0;

    // Title row
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";

    unsigned int colId = 0;

    if (full_mode) {
        sheetData += makeStringCell(++colId, rowId, "Timestamp (UTC)", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "IP", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "ID", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "Key", TITLE_BOLD);
    }
    sheetData += makeStringCell(++colId, rowId, "Email", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Username", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Phone number", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Number of Adult meals", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Number of Child meals", TITLE_BOLD);
    sheetData += makeStringCell(++colId, rowId, "Comments", TITLE_BOLD);
    if (full_mode) {
        sheetData += makeStringCell(++colId, rowId, "Payment type", TITLE_BOLD);
    }
    sheetData += makeStringCell(++colId, rowId, "Cost", TITLE_BOLD);
    if (full_mode) {
        sheetData += makeStringCell(++colId, rowId, "Payment received", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "cs_id", TITLE_BOLD);
        sheetData += makeStringCell(++colId, rowId, "payment_intent", TITLE_BOLD);
    } else {
        sheetData += makeStringCell(++colId, rowId, "Paid", TITLE_BOLD);
    }

    sheetData += "</row>\n";

    sql::PreparedStatement *prep_stmt = con->prepareStatement("SELECT UNIX_TIMESTAMP(timestamp),IP_address,registration_id,idempotency,email_address,gc_username,phone_number,number_adults,number_children,dinner_comment,payment_type,stripe_session_id, dinner_options_adults, dinner_options_children FROM sat_dinner WHERE status = 'S' AND dinner_form_id = ?;");
    prep_stmt->setInt(1, dinner_form_id);
    sql::ResultSet *res = prep_stmt->executeQuery();
    while (res->next()) {
        std::string userKey = res->getString(4);
        int cost_total = PaymentUtils::getUserCost(con, userKey);
        int payment_received = PaymentUtils::getTotalPaymentReceived(con, userKey);

        sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
        colId = 0;

        if (full_mode) {
            sheetData += makeDateTimeCell(++colId, rowId, res->getInt64(1), DATE_TIME); // Date/time
            sheetData += makeStringCell(++colId, rowId, res->getString(2), NO_STYLE);   // IP
            sheetData += makeNumberCell(++colId, rowId, res->getInt(3), NO_STYLE);      // ID
            sheetData += makeStringCell(++colId, rowId, userKey, NO_STYLE);             // userKey
        }
        sheetData += makeStringCell(++colId, rowId, res->getString(5), NO_STYLE);     // email
        sheetData += makeStringCell(++colId, rowId, res->getString(6), NO_STYLE);     // username
        sheetData += makeStringCell(++colId, rowId, res->getString(7), NO_STYLE);     // phone
        sheetData += makeNumberCell(++colId, rowId, res->getInt(8), NO_STYLE);        // number of adults
        sheetData += makeNumberCell(++colId, rowId, res->getInt(9), NO_STYLE);        // number of children
        sheetData += makeStringCell(++colId, rowId, res->getString(10), NO_STYLE);    // comments

        if (full_mode) {
            sheetData += makeStringCell(++colId, rowId, res->getString(11), NO_STYLE);    // payment type
        }
        if (res->getString(11) == "event") {
            sheetData += makeStringCell(++colId, rowId, "Inc. in rego", NO_STYLE);
        } else {
            sheetData += makeNumberCell(++colId, rowId, static_cast<double>(cost_total) / 100, CURRENCY);  // cost
        }
        bool hasPaid = (payment_received >= cost_total);
        if (full_mode) {
            sheetData += makeNumberCell(++colId, rowId, static_cast<double>(payment_received) / 100, (hasPaid ? CURRENCY : CURRENCY_RED));  // payment recived

            std::string cs_id = res->getString(12);
            sheetData += makeStringCell(++colId, rowId, cs_id, NO_STYLE);
            if (cs_id.size()) {
                sql::PreparedStatement *prep_stmt = con->prepareStatement("SELECT payment_intent FROM stripe_event_log WHERE cs_id = ?;");
                prep_stmt->setString(1, cs_id);
                sql::ResultSet *res2 = prep_stmt->executeQuery();
                if (res2->next()){
                    sheetData += makeStringCell(++colId, rowId, res2->getString(1), NO_STYLE);
                }
                delete res2;
                delete prep_stmt;
            }
        } else {
            sheetData += makeStringCell(++colId, rowId, (hasPaid ? "Yes" : "No"), (hasPaid ? NO_STYLE : RED_BACKGROUND));  // has paid
        }

        sheetData += "</row>\n";
    }
    delete res;
    delete prep_stmt;

    sheetData += "</sheetData>\n";

    this->addWorksheetFromXML(sheetData, dinner_title);
}
