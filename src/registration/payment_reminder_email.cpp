/**
  @file    payment_reminder_email.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/registration/payment_reminder_email.cgi
  Sends a payment reminder email to a customer (bank payments only)
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/Encoder.h"
#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"
#include "../core/PaymentUtils.h"
#include "../core/PostDataParser.h"
#include "../email/Email.h"
#include "../email/JlweHtmlEmail.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << JsonUtils::makeJsonError(postData.errorText());
            return 0;
        }

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_registrations")) { //if logged in

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            std::string userKey = jsonDocument.at("key");
            std::string email_in = jsonDocument.at("email");

            bool outputDone = false;
            std::string regType = PaymentUtils::getRegistrationType(jlwe.getMysqlCon(), userKey);
            if (regType == "event" || regType == "camping_only" || regType == "dinner_only") {

                bool validEntry = false;
                std::string email_address;
                std::string gc_username;
                std::string phone_number;
                std::string payment_type;
                if (regType == "camping_only") {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address, gc_username, phone_number, payment_type FROM camping WHERE idempotency = ? AND status = 'S';");
                } else if (regType == "dinner_only") {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address, gc_username, phone_number, payment_type FROM sat_dinner WHERE idempotency = ? AND status = 'S';");
                } else {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address, gc_username, phone_number, payment_type FROM event_registrations WHERE idempotency = ? AND status = 'S';");
                }
                prep_stmt->setString(1, userKey);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    email_address = res->getString(1);
                    gc_username = res->getString(2);
                    phone_number = res->getString(3);
                    payment_type = res->getString(4);
                    validEntry = true;
                }
                delete res;
                delete prep_stmt;

                if (validEntry && Email::isValidEmail(email_address) && email_in == email_address) {
                    if (payment_type == "bank") {

                        int payment_total = PaymentUtils::getUserCost(jlwe.getMysqlCon(), userKey);

                        JlweHtmlEmail makeEmail;
                        std::string subject = "Invoice Payment Reminder";
                        makeEmail.addHtml("<p class=\"title\">Payment Reminder for " + Encoder::htmlEntityEncode(gc_username) + "</p>\n");
                        makeEmail.addHtml("<p>A friendly reminder that we have not yet received your payment for the following invoice, which is now overdue. If you believe you have received this message in error, please email us at contact@jlwe.org with proof of payment.</p>\n");

                        makeEmail.addRegistrationDetails(gc_username, email_address, phone_number);
                        makeEmail.addCostTable(userKey, jlwe.getMysqlCon());
                        makeEmail.addBankDetails(PaymentUtils::currencyToString(payment_total), &jlwe, false);
                        makeEmail.addHtml("<p>For assistance or changes to your registration, please reply to this email or contact us at contact@jlwe.org</p>\n");

                        makeEmail.sendJlweEmail(email_address, "contact@jlwe.org", subject, jlwe.config.at("mailerAddress"), "June LWE Geocaching");


                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT log_user_event(?,?,?);");
                        prep_stmt->setString(1, jlwe.getCurrentUserIP());
                        prep_stmt->setString(2, jlwe.getCurrentUsername());
                        prep_stmt->setString(3, "Payment reminder email sent to " + email_address);
                        res = prep_stmt->executeQuery();
                        res->next();
                        delete res;
                        delete prep_stmt;


                        std::cout << JsonUtils::makeJsonSuccess("Reminder email sent to " + email_address);
                    } else {
                        std::cout << JsonUtils::makeJsonError("Reminder emails are only for bank payments - this is not a bank payment");
                    }
                    outputDone = true;
                }
            } else if (regType == "merch") {

                bool validEntry = false;
                int order_id = 0;
                std::string email_address;
                std::string gc_username;
                std::string phone_number;
                std::string payment_type;

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT order_id, email_address, gc_username, phone_number, payment_type FROM merch_orders WHERE idempotency = ? AND status = 'S';");
                prep_stmt->setString(1, userKey);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    order_id = res->getInt(1);
                    email_address = res->getString(2);
                    gc_username = res->getString(3);
                    phone_number = res->getString(4);
                    payment_type = res->getString(5);
                    validEntry = true;
                }
                delete res;
                delete prep_stmt;

                if (validEntry && Email::isValidEmail(email_address) && email_in == email_address) {
                    if (payment_type == "bank") {
                        int payment_total = PaymentUtils::getUserCost(jlwe.getMysqlCon(), userKey);


                        JlweHtmlEmail makeEmail;
                        std::string subject = "Invoice Payment Reminder";
                        std::string emailHTML = "<p class=\"title\">Payment Reminder for " + Encoder::htmlEntityEncode(gc_username) + "</p>\n";

                        makeEmail.addHtml("<p>A friendly reminder that we have not yet received your payment for the following invoice, which is now overdue. If you believe you have received this message in error, please email us at contact@jlwe.org with proof of payment.</p>\n");

                        makeEmail.addRegistrationDetails(gc_username, email_address, phone_number);
                        makeEmail.addMerchOrderTable(userKey, jlwe.getMysqlCon());
                        makeEmail.addBankDetails(PaymentUtils::currencyToString(payment_total), &jlwe, true);

                        makeEmail.addHtml("<p>For assistance or changes to your order, please reply to this email or contact us at contact@jlwe.org</p>\n");

                        makeEmail.sendJlweEmail(email_address, "contact@jlwe.org", subject, jlwe.config.at("mailerAddress"), "June LWE Geocaching");


                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT log_user_event(?,?,?);");
                        prep_stmt->setString(1, jlwe.getCurrentUserIP());
                        prep_stmt->setString(2, jlwe.getCurrentUsername());
                        prep_stmt->setString(3, "Payment reminder email sent to " + email_address);
                        res = prep_stmt->executeQuery();
                        res->next();
                        delete res;
                        delete prep_stmt;

                        std::cout << JsonUtils::makeJsonSuccess("Reminder email sent to " + email_address);

                    } else {
                        std::cout << JsonUtils::makeJsonError("Reminder emails are only of bank payments - this is not a bank payment");
                    }
                    outputDone = true;
                }

                if (!outputDone)
                    std::cout << JsonUtils::makeJsonError("Unable to find registration ID");

            } else {
                std::cout << JsonUtils::makeJsonError("Invalid registration ID/type");
            }
        } else {
            std::cout << JsonUtils::makeJsonError("You do not have permission to view this area");
        }
    } catch (sql::SQLException &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")");
    } catch (const std::exception &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()));
    }

    return 0;
}
