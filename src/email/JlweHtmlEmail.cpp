/**
  @file    JlweHtmlEmail.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A subclass of Email.h, used for building and sending emails with "JLWE theme" formatting
  These emails are HTML with the JLWE logo, green border and content style that matches the website

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "JlweHtmlEmail.h"

#include "../core/Encoder.h"
#include "../core/PaymentUtils.h"
#include "../core/JlweUtils.h"
#include "../prices.h"
#include "../registration/DinnerUtils.h"

//base 64 encoded version of the logo to put in email headers
#define LOGO_IMAGE  "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"
#define LOGO_IMAGE_NAME "jlwe_logo.png"
#define LOGO_IMAGE_TYPE "image/png"

#include "../ext/nlohmann/json.hpp"

JlweHtmlEmail::JlweHtmlEmail() {
    // do nothing
}

JlweHtmlEmail::~JlweHtmlEmail() {
    // do nothing
}

void JlweHtmlEmail::addHtml(const std::string &html) {
    this->inner_html += html;
}

void JlweHtmlEmail::addMerchTitle(const std::string &username) {
    this->inner_html += "<p class=\"title\">Merchandise order for " + Encoder::htmlEntityEncode(username) + "</p>\n";
}

void JlweHtmlEmail::addInvoiceTitle(const std::string &username) {
    this->inner_html += "<p class=\"title\">Invoice for " + Encoder::htmlEntityEncode(username) + "</p>\n";
}

void JlweHtmlEmail::addRegistrationDetails(const std::string &username, const std::string &email, const std::string &phone) {
    this->inner_html += "<p style=\"text-align:left;\">\n";
    this->inner_html += "Username: " + Encoder::htmlEntityEncode(username) + "<br/>\n";
    this->inner_html += "Email: " + Encoder::htmlEntityEncode(email) + "<br/>\n";
    this->inner_html += "Phone: " + Encoder::htmlEntityEncode(phone) + "\n";
    this->inner_html += "</p>\n";
}

void JlweHtmlEmail::addCostTable(std::string userKey, sql::Connection *con) {
    this->inner_html += "<table id=\"cost_table\">\n";
    this->inner_html += "      <tbody><tr>\n";
    this->inner_html += "        <th colspan=\"2\">Registration details</th>\n";
    this->inner_html += "      </tr>\n";

    int event_cost = 0;
    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;
    prep_stmt = con->prepareStatement("SELECT number_adults, number_children FROM event_registrations WHERE idempotency = ?;");
    prep_stmt->setString(1, userKey);
    res = prep_stmt->executeQuery();
    if (res->next()){
        int number_adults = res->getInt(1);
        int number_children = res->getInt(2);
        event_cost = number_adults * PRICE_EVENT_ADULT + number_children * PRICE_EVENT_CHILD;

        this->inner_html += "      <tr>\n";
        this->inner_html += "        <td><p>Event</p>\n";
        this->inner_html += "        <p class=\"subline\" style=\"padding-left:15px;\">" + std::to_string(number_adults) + " Adult" + (number_adults == 1 ? "" : "s") + ", " + std::to_string(number_children) + (number_children == 1 ? " Child" : " Children") + "</p></td>\n";
        this->inner_html += "        <td class=\"currency_cell\">" + PaymentUtils::currencyToString(event_cost) + "</td>\n";
        this->inner_html += "      </tr>\n";
    }
    delete res;
    delete prep_stmt;

    int camping_cost = 0;
    prep_stmt = con->prepareStatement("SELECT camping.camping_type, camping_options.display_name, camping_options.price_code, camping.number_people, camping.arrive_date, camping.leave_date FROM camping INNER JOIN camping_options ON camping.camping_type=camping_options.id_string WHERE camping.idempotency = ?;");
    prep_stmt->setString(1, userKey);
    res = prep_stmt->executeQuery();
    if (res->next()){
        std::string description = "";
        std::string display_name = res->getString(2);
        int number_people = res->getInt(4);
        int arrive_date = res->getInt(5);
        int leave_date = res->getInt(6);
        int camping_nights = leave_date - arrive_date;
        camping_cost = getCampingPrice(res->getString(3), number_people, camping_nights);
        description = display_name + ", " + std::to_string(number_people) + " " + (number_people > 1 ? "people" : "person") +", " + std::to_string(camping_nights) + " night" + (camping_nights > 1 ? "s" : "") + " (June " + JlweUtils::numberToOrdinal(arrive_date) + " to " + JlweUtils::numberToOrdinal(leave_date) + ")";

        this->inner_html += "      <tr>\n";
        this->inner_html += "        <td style=\"border-bottom:0px;\"><p>Camping</p>\n";
        this->inner_html += "        <p class=\"subline\" style=\"padding-left:15px;\">" + Encoder::htmlEntityEncode(description) + "</p></td>\n";
        this->inner_html += "        <td class=\"currency_cell\">" + PaymentUtils::currencyToString(camping_cost) + "</td>\n";
        this->inner_html += "      </tr>\n";
    }
    delete res;
    delete prep_stmt;

    std::vector<DinnerUtils::dinner_form> dinner_forms = DinnerUtils::getDinnerFormList(con);

    int total_dinner_cost = 0;
    prep_stmt = con->prepareStatement("SELECT dinner_form_id, number_adults, number_children, dinner_options_adults, dinner_options_children FROM sat_dinner WHERE idempotency = ?;");
    prep_stmt->setString(1, userKey);
    res = prep_stmt->executeQuery();
    while (res->next()) {
        int dinner_form_id = res->getInt(1);

        for (unsigned int i = 0; i < dinner_forms.size(); i++) {
            if (dinner_form_id == dinner_forms.at(i).dinner_id) {

                std::vector<DinnerUtils::dinner_menu_item> menu_items = DinnerUtils::getDinnerMenuItems(con, dinner_form_id);

                int dinner_number_adults = res->getInt(2);
                int dinner_number_children = res->getInt(3);
                int dinner_cost = DinnerUtils::getDinnerCost(con, dinner_forms.at(i).dinner_id, res->getString(4), menu_items);
                total_dinner_cost += dinner_cost;

                this->inner_html += "      <tr>\n";
                this->inner_html += "        <td><p>" + Encoder::htmlEntityEncode(dinner_forms.at(i).title) + "</p>\n";
                this->inner_html += "        <p class=\"subline\" style=\"padding-left:15px;\">" + std::to_string(dinner_number_adults) + " Adult meal" + (dinner_number_adults == 1 ? "" : "s") + ", "  + std::to_string(dinner_number_children) + " Child meal" + (dinner_number_children == 1 ? "" : "s") + "</p>\n";
                this->inner_html += "        <p class=\"subline\" style=\"padding-left:30px;\">" + DinnerUtils::dinnerOptionsToString(res->getString(4), menu_items) + "</p></td>\n";
                this->inner_html += "        <td class=\"currency_cell\">" + PaymentUtils::currencyToString(dinner_cost) + "</td>\n";
                this->inner_html += "      </tr>\n";
            }
        }
    }
    delete res;
    delete prep_stmt;

    int card_surcharge_cost = 0;
    prep_stmt = con->prepareStatement("SELECT fee FROM stripe_card_fees WHERE idempotency = ?;");
    prep_stmt->setString(1, userKey);
    res = prep_stmt->executeQuery();
    if (res->next()){
        card_surcharge_cost = res->getInt(1);

        this->inner_html += "      <tr>\n";
        this->inner_html += "        <td>Card processing fee</td>\n";
        this->inner_html += "        <td class=\"currency_cell\">" + PaymentUtils::currencyToString(card_surcharge_cost) + "</td>\n";
        this->inner_html += "      </tr>\n";
    }
    delete res;
    delete prep_stmt;


    int total_cost = event_cost + camping_cost + total_dinner_cost + card_surcharge_cost;
    this->inner_html += "      <tr style=\"font-weight:bold;\">\n";
    this->inner_html += "        <td style=\"border-top-width:1px;border-bottom-width:1px;\">Total</td>\n";
    this->inner_html += "        <td class=\"currency_cell\" style=\"border-top-width:1px;border-bottom-width:1px;\">" + PaymentUtils::currencyToString(total_cost) + "</td>\n";
    this->inner_html += "      </tr>\n";
    this->inner_html += "</tbody></table>\n";
}

void JlweHtmlEmail::addMerchOrderTable(std::string userKey, sql::Connection *con) {
    this->inner_html += "<table id=\"cost_table\">\n";
    this->inner_html += "      <tbody><tr>\n";
    this->inner_html += "        <th colspan=\"2\">Order details</th>\n";
    this->inner_html += "      </tr>\n";

    int total_cost = 0;
    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;
    prep_stmt = con->prepareStatement("SELECT merch_items.name, merch_items.cost, merch_order_items.item_options_str FROM merch_items INNER JOIN merch_order_items ON merch_items.id=merch_order_items.item_id INNER JOIN merch_orders ON merch_orders.order_id=merch_order_items.order_id WHERE merch_orders.idempotency = ?;");
    prep_stmt->setString(1, userKey);
    res = prep_stmt->executeQuery();
    while (res->next()){
        int item_cost = res->getInt(2);
        total_cost += item_cost;

        this->inner_html += "      <tr>\n";
        this->inner_html += "        <td>" + Encoder::htmlEntityEncode(res->getString(1)) + "<br />\n";
        this->inner_html += "        <span>&nbsp;&nbsp;&nbsp;" + Encoder::htmlEntityEncode(res->getString(3)) + "</span></td>\n";
        this->inner_html += "        <td class=\"currency_cell\">" + PaymentUtils::currencyToString(item_cost) + "</td>\n";
        this->inner_html += "      </tr>\n";
    }
    delete res;
    delete prep_stmt;

    int card_surcharge_cost = 0;
    prep_stmt = con->prepareStatement("SELECT fee FROM stripe_card_fees WHERE idempotency = ?;");
    prep_stmt->setString(1, userKey);
    res = prep_stmt->executeQuery();
    if (res->next()){
        card_surcharge_cost = res->getInt(1);
        total_cost += card_surcharge_cost;

        this->inner_html += "      <tr>\n";
        this->inner_html += "        <td>Card processing fee</td>\n";
        this->inner_html += "        <td class=\"currency_cell\">" + PaymentUtils::currencyToString(card_surcharge_cost) + "</td>\n";
        this->inner_html += "      </tr>\n";
    }
    delete res;
    delete prep_stmt;

    this->inner_html += "      <tr style=\"font-weight:bold;\">\n";
    this->inner_html += "        <td style=\"border-top-width:1px;border-bottom-width:1px;\">Total</td>\n";
    this->inner_html += "        <td class=\"currency_cell\" style=\"border-top-width:1px;border-bottom-width:1px;\">" + PaymentUtils::currencyToString(total_cost) + "</td>\n";
    this->inner_html += "      </tr>\n";
    this->inner_html += "</tbody></table>\n";
}

void JlweHtmlEmail::addBankDetails(std::string payment_total, JlweCore *jlwe, bool merch_order) {
    this->inner_html += "<p>Please make a bank transfer of " + Encoder::htmlEntityEncode(payment_total) + " to the account listed below with your geocaching username in the reference field.";
    if (merch_order) {
        this->inner_html += " Your order will not be processed until payment is received.</p>\n";
    } else {
        this->inner_html += " Your camping and/or dinner is not reserved until payment is received.</p>\n";
    }
    this->inner_html += "<p style=\"font-weight:bold;\">" + jlwe->getGlobalVar("bank_details") + "</p>\n";
}

void JlweHtmlEmail::addCashPayment(std::string payment_total) {
    this->inner_html += "<p>You will need to pay " + Encoder::htmlEntityEncode(payment_total) + " upon arrival at the event. <span style=\"font-weight:bold;\">Note that this will need to be paid via eftpos/paywave.</span></p>\n";
}

void JlweHtmlEmail::addCardPayment(std::string payment_received, time_t timestamp) {
    if (timestamp) {
        timestamp += 9 * 60 * 60 + 30 * 60; // hack to get Adelaide time from server in UTC timezone
        struct tm * timeinfo;
        timeinfo = gmtime(&timestamp);
        char buffer[100];
        strftime (buffer, 100, "%d %B %Y",timeinfo);

        this->inner_html += "<p>Payment of " + Encoder::htmlEntityEncode(payment_received) + " has been received via card payment on " + std::string(buffer) + ".</p>\n";

    } else {
        this->inner_html += "<p>Payment of " + Encoder::htmlEntityEncode(payment_received) + " has been received.</p>\n";
    }
}

int JlweHtmlEmail::sendJlweEmail(std::string to_address, std::string reply_address, std::string subject, std::string mailer_address, std::string from_name) {
    this->addInlineBase64(LOGO_IMAGE, LOGO_IMAGE_NAME, LOGO_IMAGE_TYPE);
    this->setHtmlInTemplate("cid:" + Encoder::filterSafeCharsOnly(LOGO_IMAGE_NAME));

    return this->sendEmail(to_address, reply_address, "[June LWE Geocaching] " + subject, mailer_address, from_name);
}

void JlweHtmlEmail::setHtmlInTemplate(std::string logo_url) {
    std::string result = "";

    result += "<!DOCTYPE html>\n";
    result += "<html>\n";
    result += "<head>\n";
    result += "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n";
    result += "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n";
    result += "    <title>JLWE Invoice</title>\n";
    result += "    <style>\n";
    // Take care of image borders and formatting, client hacks
    result += "    img { max-width: 600px; outline: none; text-decoration: none; -ms-interpolation-mode: bicubic;}\n";
    result += "    a img { border: none; }\n";
    result += "    table { border-collapse: collapse !important; width: 100%; }\n";
    result += "    table td { border-collapse: collapse; width: 100%; }\n";
    result += "    .container-for-gmail-android { min-width: 540px; }\n";

    result += "    body {\n";
    result += "      text-align: center;\n";
    result += "      color: #000000;\n";
    result += "      font-family: Arial, Helvetica, sans-serif;\n";
    result += "      font-size: 16px;\n";
    result += "      margin: 0;\n";
    result += "    }\n";
    result += "    p {\n";
    result += "      margin: 0px;\n";
    result += "      text-align: center;\n";
    result += "    }\n";
    result += "    #page {\n";
    result += "      width: 500px;\n";
    result += "      display: block;\n";
    result += "      background: #0B88B4;\n";
    result += "      padding-left: 20px;\n";
    result += "      padding-right: 20px;\n";
    result += "    }\n";
    result += "    #header {\n";
    result += "      text-align: center;\n";
    result += "      background-color: #0B88B4;\n";
    result += "      background-image: linear-gradient(#0B88B4, #FFFFFF);\n";
    result += "      background: #0B88B4;\n";
    result += "      background: linear-gradient(#0B88B4, #FFFFFF);\n";
    result += "    }\n";
    result += "    #header > p {\n";
    result += "      font-size: 26px;\n";
    result += "      font-weight: bold;\n";
    result += "    }\n";
    result += "    #body {\n";
    result += "      background: #FFFFFF;\n";
    result += "      padding-left: 20px;\n";
    result += "      padding-right: 20px;\n";
    result += "      padding-top: 10px;\n";
    result += "      padding-bottom: 10px;\n";
    result += "    }\n";
    result += "    #body > .title {\n";
    result += "      font-size: 20px;\n";
    result += "      font-weight: bold;\n";
    result += "      margin: 0;\n";
    result += "      padding: 10px;\n";
    result += "      mso-line-height-rule: exactly;\n";
    result += "    }\n";
    result += "    #body > p {\n";
    result += "      font-size: 16px;\n";
    result += "      margin-top: 10px;\n";
    result += "      margin-bottom: 10px;\n";
    result += "      margin-left: 0px;\n";
    result += "      margin-right: 0px;\n";
    result += "      line-height: 1.5em;\n";
    result += "    }\n";
    result += "    #footer p {\n";
    result += "      color: #000000;\n";
    result += "      height: 37px;\n";
    result += "      line-height: 37px;\n";
    result += "      text-align: center;\n";
    result += "      margin: 0;\n";
    result += "    }\n";

    result += "    #cost_table {\n";
    result += "      width: 100%;\n";
    result += "      margin-top: 10px;\n";
    result += "    }\n";

    result += "    #cost_table th, #cost_table td {\n";
    result += "      border-style: solid;\n";
    result += "      border-top-width: 0px;\n";
    result += "      border-left-width: 0px;\n";
    result += "      border-right-width: 0px;\n";
    result += "      padding: 5px;\n";
    result += "      font-size: 16px;\n";
    result += "    }\n";
    result += "    #cost_table th {\n";
    result += "      border-bottom-width: 1px;\n";
    result += "    }\n";
    result += "    #cost_table td {\n";
    result += "      text-align: left;\n";
    result += "      border-bottom-width: 0px;\n";
    result += "    }\n";
    result += "    #cost_table td > span {\n";
    result += "      font-size: 14px;\n";
    result += "    }\n";
    result += "    #cost_table td p {\n";
    result += "      text-align: left;\n";
    result += "    }\n";
    result += "    #cost_table td .subline {\n";
    result += "      font-size: 14px;\n";
    result += "    }\n";
    result += "    #cost_table .currency_cell {\n";
    result += "      vertical-align: top;\n";
    result += "      text-align: right;\n";
    result += "    }\n";
    result += "    </style>\n";

    result += "    <style type=\"text/css\" media=\"only screen and (max-width: 520px)\">\n";
          // Mobile styles
    result += "      @media only screen and (max-width: 520px) {\n";

    result += "        table[class*=\"container-for-gmail-android\"] {\n";
    result += "          min-width: 250px !important;\n";
    result += "          width: 100% !important;\n";
    result += "        }\n";

    result += "        #page {\n";
    result += "          width: 100%;\n";
    result += "          display: block;\n";
    result += "          background: #0B88B4;\n";
    result += "          padding-left: 0;\n";
    result += "          padding-right: 0;\n";
    result += "        }\n";
    result += "      }\n";
    result += "    </style>\n";
    result += "</head>\n";
    result += "<body bgcolor=\"#FFFFFF\">\n";
    result += "<table align=\"center\" cellpadding=\"0\" cellspacing=\"0\" class=\"container-for-gmail-android\" width=\"100%\">\n";
    result += "  <tr><td width=\"100%\" style=\"background:#ffffff;\">\n";
    result += "    <table id=\"page\" align=\"center\" cellpadding=\"0\" cellspacing=\"0\" width=\"500px\">\n";
    result += "      <tr><td id=\"header\" width=\"100%\">\n";

    result += "<p style=\"font-size:0;\">\n";
    result += "<img src=\"" + logo_url + "\" alt=\"" + logo_url + "\" /></p>\n";
    result += "<p>June LWE Geocaching Event</p>\n";

    result += "      </td></tr>\n";
    result += "      <tr><td width=\"100%\">\n";

    result += "        <table align=\"center\" cellpadding=\"0\" cellspacing=\"0\">\n";
    result += "          <tr><td id=\"body\" width=\"100%\">\n";

    result += this->inner_html;

    result += "          </td></tr>\n";
    result += "        </table>\n";
    result += "      </td></tr>\n";
    result += "      <tr><td id=\"footer\" width=\"100%\">\n";
    result += "        <p><a href=\"https://jlwe.org\" style=\"color:#000000;\">June Long Weekend Geocaching Event Inc</a> | " + JlweUtils::getCurrentYearString() + "</p>\n";
    result += "      </td></tr>\n";
    result += "    </table>\n";
    result += "  </td></tr>\n";
    result += "</table>\n";
    result += "</body>\n";
    result += "</html>\n";

    this->setHtml(result);
}


