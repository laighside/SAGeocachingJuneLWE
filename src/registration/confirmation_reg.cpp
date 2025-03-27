/**
  @file    confirmation_reg.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/registration/confirmation_reg.cgi?userKey=...
  This page is shown to customers once they have completed there registration.
  It marks their registration as saved and sends them an email invoice.
  This page may be requested several times, but the email must only be sent once.

  The registration is marked as deleted if cancel=true is set. This happens if the user clicks cancel/back while on the Stripe page
  confirmation_reg.cgi?cancel=true&userKey=...

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/Encoder.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/KeyValueParser.h"
#include "../core/PaymentUtils.h"
#include "../email/Email.h"
#include "../email/JlweHtmlEmail.h"

int main () {
    try {
        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        std::string userKey = urlQueries.getValue("key");
        std::string session_id = urlQueries.getValue("session_id");
        std::string cancel = urlQueries.getValue("cancel");

        JlweCore jlwe;

        std::string result = "<p>Something has gone wrong. Please let us know at " + std::string(jlwe.config.at("adminEmail")) + "</p>";
        bool livemode = true;

        sql::Statement *stmt;
        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (session_id.size()) {
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT idempotency FROM event_registrations WHERE stripe_session_id = ? UNION SELECT idempotency FROM camping WHERE stripe_session_id = ? UNION SELECT idempotency FROM sat_dinner WHERE stripe_session_id = ?;");
            prep_stmt->setString(1, session_id);
            prep_stmt->setString(2, session_id);
            prep_stmt->setString(3, session_id);
            res = prep_stmt->executeQuery();
            if (res->next()){
                userKey = res->getString(1);
            }
            delete res;
            delete prep_stmt;
        }

        std::string regType = PaymentUtils::getRegistrationType(jlwe.getMysqlCon(), userKey);
        if (regType == "event" || regType == "camping_only" || regType == "dinner_only") {

            if (session_id.size() && cancel == "true") {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setRegistrationStatus(?,?,?,?);");
                prep_stmt->setString(1, userKey);
                prep_stmt->setString(2, "D");
                prep_stmt->setString(3, jlwe.getCurrentUserIP());
                prep_stmt->setString(4, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next()) {
                }
                delete res;
                delete prep_stmt;

                if (regType == "camping_only") {
                    result = "<p>Payment (and camping) has been cancelled. <a href=\"/camping\">Click here to start over and submit another campsite booking.</a></p>\n";
                } else if (regType == "dinner_only") {
                    result = "<p>Payment (and dinner) has been cancelled. <a href=\"/dinner\">Click here to start over and submit another dinner booking.</a></p>\n";
                } else {
                    result = "<p>Payment (and registration) has been cancelled. <a href=\"/register\">Click here to start over and submit another registration.</a></p>\n";
                }
            } else {

                bool validEntry = false;
                std::string email_address;
                std::string gc_username;
                std::string phone_number;
                std::string payment_type;
                std::string past_status = "S";
                if (regType == "camping_only") {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address, gc_username, phone_number, livemode, payment_type, status FROM camping WHERE idempotency = ?;");
                } else if (regType == "dinner_only") {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address, gc_username, phone_number, livemode, payment_type, status FROM sat_dinner WHERE idempotency = ?;");
                } else {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address, gc_username, phone_number, livemode, payment_type, status FROM event_registrations WHERE idempotency = ?;");
                }
                prep_stmt->setString(1, userKey);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    email_address = res->getString(1);
                    gc_username = res->getString(2);
                    phone_number = res->getString(3);
                    livemode = res->getInt(4);
                    payment_type = res->getString(5);
                    past_status = res->getString(6);
                    validEntry = true;
                } else {
                    result = "<p>Invalid registration ID. Please try again or contact " + std::string(jlwe.config.at("adminEmail")) + " for assistance.</p>\n";
                }
                delete res;
                delete prep_stmt;

                if (validEntry) {
                    result = "<p>Thankyou for registering for the June Long Weekend event, we look forward to seeing you there!</p>\n";
                    int payment_total = PaymentUtils::getUserCost(jlwe.getMysqlCon(), userKey);

                    if ((past_status == "P" || past_status == "D") && Email::isValidEmail(email_address)) {
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setRegistrationStatus(?,?,?,?);");
                        prep_stmt->setString(1, userKey);
                        prep_stmt->setString(2, "S");
                        prep_stmt->setString(3, jlwe.getCurrentUserIP());
                        prep_stmt->setString(4, jlwe.getCurrentUsername());
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                        }
                        delete res;
                        delete prep_stmt;

                        JlweHtmlEmail makeEmail;

                        makeEmail.addInvoiceTitle(gc_username);

                        std::string subject = "Event Registration Invoice";
                        if (regType == "camping_only") {
                            makeEmail.addHtml("<p>Thankyou for reserving a camping site at the June Long Weekend event.</p>\n");
                            subject = "Camping Invoice";
                        } else if (regType == "dinner_only") {
                            makeEmail.addHtml("<p>Thankyou for booking a meal at the Saturday night dinner of the June Long Weekend event.</p>\n");
                            subject = "Saturday Dinner Invoice";
                        } else {
                            makeEmail.addHtml("<p>Thankyou for registering for the June Long Weekend event.</p>\n");
                        }

                        makeEmail.addRegistrationDetails(gc_username, email_address, phone_number);

                        makeEmail.addCostTable(userKey, jlwe.getMysqlCon());

                        if (payment_type == "cash")
                            makeEmail.addCashPayment(PaymentUtils::currencyToString(payment_total));
                        if (payment_type == "bank")
                            makeEmail.addBankDetails(PaymentUtils::currencyToString(payment_total), &jlwe, false);
                        if (payment_type == "card") {
                            std::vector<PaymentUtils::paymentEntry> paymentTable = PaymentUtils::getUserPayments(jlwe.getMysqlCon(), userKey);
                            if (paymentTable.size() == 1 && paymentTable.at(0).payment_type == "card") {
                                makeEmail.addCardPayment(PaymentUtils::currencyToString(paymentTable.at(0).payment_amount), paymentTable.at(0).timestamp);
                            } else {
                                int payment_received = PaymentUtils::getTotalPaymentReceived(&paymentTable);
                                if (payment_received > 0) {
                                    makeEmail.addCardPayment(PaymentUtils::currencyToString(payment_received), 0);
                                }
                            }
                        }

                        makeEmail.addHtml("<p>For assistance or changes to your registration, please reply to this email or contact us at contact@jlwe.org</p>\n");

                        makeEmail.sendJlweEmail(email_address, "contact@jlwe.org", subject, jlwe.config.at("mailerAddress"), "June LWE Geocaching");


                        // Send emails to admins that want to be notified of new registrations
                        std::string messageHtml = "<html><body><p>A new event registration has been received from <span style=\"font-weight:bold;\">" + Encoder::htmlEntityEncode(gc_username) + "</span>. View it at <a href=\"" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/cgi-bin/registration/registration.cgi\">" + std::string(jlwe.config.at("websiteDomain")) + "/cgi-bin/registration/registration.cgi</a></p>\n";
                        messageHtml += "<p>To stop receiving these emails, please visit <a href=\"" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/cgi-bin/users/users.cgi\">" + std::string(jlwe.config.at("websiteDomain")) + "/cgi-bin/users/users.cgi</a> or contact " + std::string(jlwe.config.at("adminEmail")) + "</p></body></html>";
                        std::string messagePlain = "A new event registration has been received from " + gc_username + ". View it at " + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/cgi-bin/registration/registration.cgi\n\n";
                        messagePlain += "To stop receiving these emails, please visit " + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/cgi-bin/users/users.cgi or contact " + std::string(jlwe.config.at("adminEmail"));

                        stmt = jlwe.getMysqlCon()->createStatement();
                        res = stmt->executeQuery("SELECT users.email FROM user_preferences INNER JOIN users ON users.user_id=user_preferences.user_id WHERE users.email IS NOT NULL AND user_preferences.email_reg_every != 0;");
                        while (res->next()) {
                            std::string toEmail = res->getString(1);

                            Email notificationEmail;

                            notificationEmail.setPlainText(messagePlain);
                            notificationEmail.setHtml(messageHtml);

                            notificationEmail.sendEmail(toEmail, "noreply@" + std::string(jlwe.config.at("websiteDomain")) , "[June LWE Geocaching] Event registration notification", std::string(jlwe.config.at("mailerAddress")), "June LWE Geocaching");
                        }
                        delete res;
                        delete stmt;
                    }


                    if (regType == "camping_only") {
                        result += "<p>Camping site booking for <span style=\"font-weight:bold;\">" + Encoder::htmlEntityEncode(gc_username) + "</span> has been received.</p>\n";
                    } else if (regType == "dinner_only") {
                        result += "<p>Saturday dinner booking for <span style=\"font-weight:bold;\">" + Encoder::htmlEntityEncode(gc_username) + "</span> has been received.</p>\n";
                    } else {
                        result += "<p>Registration for <span style=\"font-weight:bold;\">" + Encoder::htmlEntityEncode(gc_username) + "</span> has been received.</p>\n";
                    }

                    if (payment_type == "cash")
                        result += "<p>You will need to pay <span style=\"font-weight:bold;\">" + PaymentUtils::currencyToString(payment_total) + "</span> upon arrival at the event.</p>\n";// <span style=\"font-weight:bold;\">Note that there will be no change given so please ensure you bring the exact amount.</span></p>\n";

                    if (payment_type == "bank") {
                        result += "<p>You will need to make a bank transfer of <span style=\"font-weight:bold;\">" + PaymentUtils::currencyToString(payment_total) + "</span> to the account details below. Please place your geocaching username in the reference field.</p>\n";
                        result += "<p style=\"font-weight:bold;text-align:center;\">" + jlwe.getGlobalVar("bank_details") + "</p>\n";
                    }
                    if (payment_type == "card") {
                        int payment_received = PaymentUtils::getTotalPaymentReceived(jlwe.getMysqlCon(), userKey);
                        if (payment_received > 0)
                            result += "<p>Payment of " + PaymentUtils::currencyToString(payment_received) + " for <span style=\"font-weight:bold;\">" + Encoder::htmlEntityEncode(gc_username) + "</span> has been received.</p>\n";
                    }

                    if (regType == "event")
                        result += "<p>If you have signed up to the mailing list, you will need to confirm your email address by clicking the link in the email we have sent you.</p>\n";

                }
            }
        } else {
            result = "<p>Invalid registration ID. Please try again or contact " + std::string(jlwe.config.at("adminEmail")) + " for assistance.</p>\n";
        }



        HtmlTemplate html(true);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "Event Registration - Confirmation", false))
            return 0;

        if (!livemode) {
            std::cout << "<div class=\"note\"><p><span style=\"font-weight:bold;\">Stripe test mode is enabled.</span> See <a href=\"https://stripe.com/docs/testing\">https://stripe.com/docs/testing</a></p></div>\n";
        }

        //content
        std::cout << "<h1>Event Registration</h1>\n";
        std::cout << result << "\n";

        //end of template
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
