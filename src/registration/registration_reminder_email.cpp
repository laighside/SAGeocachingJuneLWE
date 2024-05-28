/**
  @file    registration_reminder_email.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/registration/registration_reminder_email.cgi
  Sends a event registration reminder email to a customer (for people who have only booked camping/dinner)
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

            std::string regType = PaymentUtils::getRegistrationType(jlwe.getMysqlCon(), userKey);
            if (regType == "camping_only" || regType == "dinner_only") {

                bool validEntry = false;
                std::string email_address;
                std::string gc_username;
                std::string phone_number;
                if (regType == "camping_only") {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address, gc_username, phone_number, payment_type FROM camping WHERE idempotency = ? AND status = 'S';");
                } else if (regType == "dinner_only") {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address, gc_username, phone_number, payment_type FROM sat_dinner WHERE idempotency = ? AND status = 'S';");
                }
                prep_stmt->setString(1, userKey);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    email_address = res->getString(1);
                    gc_username = res->getString(2);
                    phone_number = res->getString(3);
                    validEntry = true;
                }
                delete res;
                delete prep_stmt;

                if (validEntry && Email::isValidEmail(email_address) && email_in == email_address) {

                    JlweHtmlEmail makeEmail;
                    std::string subject = "Event Registration Reminder";
                    makeEmail.addHtml("<p class=\"title\">Event Registration Reminder</p>\n");
                    makeEmail.addHtml("<p style=\"text-align:left;\">Hi " + Encoder::htmlEntityEncode(gc_username) + ",</p>\n");

                    makeEmail.addHtml("<p style=\"text-align:left;\">We notice that you've booked camping and/or a dinner meal for the June LWE event but haven't yet registered for the event itself.</p>\n");
                    makeEmail.addHtml("<p style=\"font-weight:bold;\">To register for the event, please visit <a href=\"jlwe.org/register\">jlwe.org/register</a></p>\n");
                    makeEmail.addHtml("<p style=\"text-align:left;\">Event registration is required to participate in the games, the bacon &amp; eggs breakfast and to access the tea &amp; coffee facilitates.</p>\n");
                    makeEmail.addHtml("<p style=\"text-align:left;\">Regards,<br />JLWE team</p>\n");

                    makeEmail.addHtml("<p>For assistance please reply to this email or contact us at contact@jlwe.org</p>\n");

                    makeEmail.sendJlweEmail(email_address, "contact@jlwe.org", subject, jlwe.config.at("mailerAddress"), "June LWE Geocaching");

                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT log_user_event(?,?,?);");
                    prep_stmt->setString(1, jlwe.getCurrentUserIP());
                    prep_stmt->setString(2, jlwe.getCurrentUsername());
                    prep_stmt->setString(3, "Event registration reminder email sent to " + email_address);
                    res = prep_stmt->executeQuery();
                    res->next();
                    delete res;
                    delete prep_stmt;

                    std::cout << JsonUtils::makeJsonSuccess("Reminder email sent to " + email_address);
                } else {
                    std::cout << JsonUtils::makeJsonError("Invalid email address");
                }
            } else if (regType == "event") {
                std::cout << JsonUtils::makeJsonError("Reminder emails are for camping/dinner orders only - this is an event registration");
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
