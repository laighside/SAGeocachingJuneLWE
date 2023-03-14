/**
  @file    payment_history.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/registration/payment_history.cgi?key=...
  This page shows the details and payment for a given registration

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/Encoder.h"
#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/KeyValueParser.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/PaymentUtils.h"

bool paymentSortByTime (PaymentUtils::paymentEntry i, PaymentUtils::paymentEntry j) { return (i.timestamp<j.timestamp); }

int main () {
    try {
        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        JlweCore jlwe;

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - Payment History", false))
            return 0;

        if (jlwe.getPermissionValue("perm_registrations")) { //if logged in
            std::string message = "";

            std::string userKey = urlQueries.getValue("key");
            std::string email_address;
            std::string gc_username;
            std::string phone_number;
            bool livemode;
            std::string payment_type;
            std::string status;
            time_t timestamp = 0;
            std::string ip_address;

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address,gc_username,phone_number,livemode,payment_type,status,UNIX_TIMESTAMP(timestamp),ip_address FROM event_registrations WHERE idempotency = ?;");
            prep_stmt->setString(1, userKey);
            res = prep_stmt->executeQuery();
            if (res->next()) {
                email_address = res->getString(1);
                gc_username = res->getString(2);
                phone_number = res->getString(3);
                livemode = res->getInt(4);
                payment_type = res->getString(5);
                status = res->getString(6);
                timestamp = res->getInt64(7);
                ip_address = res->getString(8);
            }
            delete res;
            delete prep_stmt;

            if (!gc_username.size()) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address,gc_username,phone_number,livemode,payment_type,status,UNIX_TIMESTAMP(timestamp),ip_address FROM camping WHERE idempotency = ?;");
                prep_stmt->setString(1, userKey);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    email_address = res->getString(1);
                    gc_username = res->getString(2) + " (Camping)";
                    phone_number = res->getString(3);
                    livemode = res->getInt(4);
                    payment_type = res->getString(5);
                    status = res->getString(6);
                    timestamp = res->getInt64(7);
                    ip_address = res->getString(8);
                }
                delete res;
                delete prep_stmt;
            }

            if (!gc_username.size()) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address,gc_username,phone_number,livemode,payment_type,status,UNIX_TIMESTAMP(timestamp),ip_address FROM sat_dinner WHERE idempotency = ?;");
                prep_stmt->setString(1, userKey);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    email_address = res->getString(1);
                    gc_username = res->getString(2) + " (Dinner)";
                    phone_number = res->getString(3);
                    livemode = res->getInt(4);
                    payment_type = res->getString(5);
                    status = res->getString(6);
                    timestamp = res->getInt64(7);
                    ip_address = res->getString(8);
                }
                delete res;
                delete prep_stmt;
            }

            if (!gc_username.size()) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address,gc_username,phone_number,livemode,payment_type,status,UNIX_TIMESTAMP(timestamp),ip_address FROM merch_orders WHERE idempotency = ?;");
                prep_stmt->setString(1, userKey);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    email_address = res->getString(1);
                    gc_username = res->getString(2) + " (Merchandise)";
                    phone_number = res->getString(3);
                    livemode = res->getInt(4);
                    payment_type = res->getString(5);
                    status = res->getString(6);
                    timestamp = res->getInt64(7);
                    ip_address = res->getString(8);
                }
                delete res;
                delete prep_stmt;
            }

            if (gc_username.size()) {

                if (!livemode) {
                    std::cout << "<div class=\"note\"><p><span style=\"font-weight:bold;\">This is a Stripe test mode order.</span> See <a href=\"https://stripe.com/docs/testing\">https://stripe.com/docs/testing</a></p></div>\n";
                }

                std::cout << "<h2 style=\"text-align:center\">Details for " << Encoder::htmlEntityEncode(gc_username) << "</h2>\n";

                if (status == "S") status = "Saved";
                if (status == "C") status = "Cancelled";
                if (status == "D") status = "Deleted";
                if (status == "P") status = "Pending";

                std::string ip_country = JlweUtils::getGeoIPCountry(ip_address, jlwe.config.value("mmdbFilename", ""));

                std::cout << "<p>Email: " << Encoder::htmlEntityEncode(email_address) << "<br />\n";
                std::cout << "Phone: " << Encoder::htmlEntityEncode(phone_number) << "<br />\n";
                std::cout << "Payment type: " << Encoder::htmlEntityEncode(payment_type) << "<br />\n";
                std::cout << "Time received: <span class=\"date_time\" data-value=\"" << timestamp << "\"></span><br />\n";
                if (ip_country.size())
                    std::cout << "IP address country: " << Encoder::htmlEntityEncode(ip_country) << "<br />\n";
                std::cout << "Order status: " << Encoder::htmlEntityEncode(status) << "</p>\n";


                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT number_adults,number_children,past_jlwe,have_lanyard,real_names_adults,real_names_children FROM event_registrations WHERE idempotency = ?;");
                prep_stmt->setString(1, userKey);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    std::cout << "<p><span style=\"font-weight:bold;\">Event Registration</span><br />\n";
                    std::cout << "Number of Adults: " << res->getInt(1) << "<br />\n";
                    std::cout << "Number of Children: " << res->getInt(2)  << "<br />\n";
                    std::cout << "Have been to past JLWE: " << (res->getInt(3) ? "Yes" : "No") << "<br />\n";
                    std::cout << "Have Lanyard: " << (res->getInt(4) ? "Yes" : "No") << "<br />\n";
                    std::cout << "Real names (adults): " << Encoder::htmlEntityEncode(res->getString(5)) << "<br />\n";
                    std::cout << "Real names (children): " << Encoder::htmlEntityEncode(res->getString(6)) << "</p>\n";
                }
                delete res;
                delete prep_stmt;

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT number_people,camping_type,arrive_date,leave_date,camping_comment FROM camping WHERE idempotency = ?;");
                prep_stmt->setString(1, userKey);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    std::cout << "<p><span style=\"font-weight:bold;\">Camping</span><br />\n";
                    std::cout << "Number of People: " << res->getInt(1) << "<br />\n";
                    std::cout << "Type: " << Encoder::htmlEntityEncode(res->getString(2)) << "<br />\n";
                    std::cout << "Dates: June " << JlweUtils::numberToOrdinal(res->getInt(3)) << " - " << JlweUtils::numberToOrdinal(res->getInt(4)) << "<br />\n";
                    std::cout << "Comment: " << Encoder::htmlEntityEncode(res->getString(5)) << "</p>\n";
                }
                delete res;
                delete prep_stmt;

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT number_adults,number_children,dinner_comment FROM sat_dinner WHERE idempotency = ?;");
                prep_stmt->setString(1, userKey);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    std::cout << "<p><span style=\"font-weight:bold;\">Saturday Dinner</span><br />\n";
                    std::cout << "Number of Adults: " << res->getInt(1) << "<br />\n";
                    std::cout << "Number of Children: " << res->getInt(2)  << "<br />\n";
                    std::cout << "Comment: " << Encoder::htmlEntityEncode(res->getString(3)) << "</p>\n";
                }
                delete res;
                delete prep_stmt;


                std::cout << "<p><span style=\"font-weight:bold;\">Payment History</span><br />\n";
                std::cout << "The time for bank and cash payments is only approximate.</p>\n";

                std::cout << "<table class=\"reg_table\" align=\"center\"><tr>\n";
                std::cout << "<th>ID</th><th>Date/Time</th><th>Payment amount</th><th>Payment type</th>\n";
                std::cout << "</tr>\n";

                std::vector<PaymentUtils::paymentEntry> table = PaymentUtils::getUserPayments(jlwe.getMysqlCon(), userKey);

                std::sort(table.begin(), table.end(), paymentSortByTime);

                for (unsigned int i = 0; i < table.size(); i++) {

                    std::cout << "<tr>\n";
                    std::cout << "<td>" << Encoder::htmlEntityEncode(table.at(i).id) << "</td>\n";
                    std::cout << "<td class=\"date_time\" data-value=\"" << table.at(i).timestamp << "\"></td>\n";
                    std::cout << "<td>" << PaymentUtils::currencyToString(table.at(i).payment_amount) << "</td>\n";
                    std::cout << "<td>" << Encoder::htmlEntityEncode(table.at(i).payment_type) << "</td>\n";
                    std::cout << "</tr>\n";
                }
                int total = PaymentUtils::getTotalPaymentReceived(&table);
                std::cout << "<tr>\n";
                std::cout << "<td colspan=\"2\" style=\"font-weight:bold;\">Total</td>\n";
                std::cout << "<td style=\"font-weight:bold;\">" << PaymentUtils::currencyToString(total) << "</td>\n";
                std::cout << "</tr>\n";

                std::cout << "</table>\n";
            } else {
                std::cout << "<h2 style=\"text-align:center\">Payment History</h2>\n";
                std::cout << "<p>User not found.</p>\n";
            }

            std::cout << FormElements::includeJavascript("/js/format_date_time.js");

        } else {
            if (jlwe.isLoggedIn()) {
                std::cout << "<p>You don't have permission to view this area.</p>";
            } else {
                std::cout << "<p>You need to be logged in to view this area.</p>";
            }
        }

        html.outputFooter();

    } catch (const sql::SQLException &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << e.what() << " (MySQL error code: " << std::to_string(e.getErrorCode()) << ")\n";
    } catch (const std::exception &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << e.what();
    }

    return 0;
}
