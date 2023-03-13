/**
  @file    forward_message.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/contact_form/forward_message.cgi
  This forwards a message to an email address
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/Encoder.h"
#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"
#include "../core/PostDataParser.h"
#include "../email/Email.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        if (jlwe.isLoggedIn()) { //if logged in

            sql::PreparedStatement *prep_stmt;
            sql::ResultSet *res;

            PostDataParser postData(jlwe.config.at("maxPostSize"));
            if (postData.hasError()) {
                std::cout << JsonUtils::makeJsonError(postData.errorText());
                return 0;
            }

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            int message_id = jsonDocument.at("message_id");
            std::string forward_email_address = jsonDocument.at("email");

            if (!Email::isValidEmail(forward_email_address)) {
                throw std::runtime_error(forward_email_address + " is not a valid email address.");
            }

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT from_name,email_address,message,status FROM contact_form WHERE id = ?;");
            prep_stmt->setInt(1, message_id);
            res = prep_stmt->executeQuery();
            if (res->next()) {
                std::string from_name = res->getString(1);
                std::string reply_email_address = res->getString(2);
                std::string message = res->getString(3);
                std::string status = res->getString(4);

                if (status.substr(0, 1) == "S") {
                    std::cout << JsonUtils::makeJsonError("Forwarding spam messages is not allowed");
                } else {
                    std::string subject = from_name + " contacting you from " + std::string(jlwe.config.at("websiteDomain"));
                    std::string plainContent = Encoder::htmlEntityEncode(message) + "\n\n";

                    Email makeEmail;
                    makeEmail.setPlainText(plainContent);
                    if (makeEmail.sendEmail(forward_email_address, reply_email_address, subject, jlwe.config.at("mailerAddress"), from_name) == 0) {
                        std::cout << JsonUtils::makeJsonSuccess("Message forwarded to " + forward_email_address);
                    } else {
                        std::cout << JsonUtils::makeJsonError("Error sending email");
                    }
                }
            } else {
                std::cout << JsonUtils::makeJsonError("Invalid message ID");
            }
            delete res;
            delete prep_stmt;
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
