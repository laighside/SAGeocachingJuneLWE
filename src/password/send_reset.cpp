/**
  @file    send_reset.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  /cgi-bin/password/send_reset.cgi
  Return format is always JSON
  Sends a password reset email to the given email
  The email is in the JSON formatted POST data

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/PostDataParser.h"
#include "../core/JlweUtils.h"
#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"
#include "../email/Email.h"

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

        if (jlwe.isLoggedIn()) { // if logged in - only logged in users can send password resets

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            std::string email_in = jsonDocument.value("email", "");

            // check that the email is in the database
            bool validEmail = false;
            int target_user_id = -1;
            if (email_in.size()) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT user_id FROM users WHERE email = ?;");
                prep_stmt->setString(1, email_in);
                res = prep_stmt->executeQuery();
                if (res->next()){
                    validEmail = true;
                    target_user_id = res->getInt(1);
                }
                delete res;
                delete prep_stmt;
            }

            if (validEmail) {

                // only allow admins to send resets to other users
                // not sure why anyone would need to send one themselves if they are logged in?
                if (jlwe.getPermissionValue("perm_admin") || jlwe.getCurrentUserId() == target_user_id) {

                    // make reset token
                    std::string token = JlweUtils::makeRandomToken(64);

                    // save the token in the database
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT resetPasswordRequest(?,?,?,?);");
                    prep_stmt->setString(1, email_in);
                    prep_stmt->setString(2, token);
                    prep_stmt->setString(3, jlwe.getCurrentUserIP());
                    prep_stmt->setString(4, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    if (res->next() && res->getInt(1) == 0){

                        // make the password reset email
                        std::string htmlMessage = "<p>You have requested that your password for " + std::string(jlwe.config.at("websiteDomain")) + " be reset.</p>\n\n";
                        htmlMessage += "<p>Please reset your password by clicking here: <a href=\"" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/cgi-bin/password/reset.cgi?reset=" + token + "\">" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/cgi-bin/password/reset.cgi?reset=" + token + "</a><br/>\n";

                        std::string plainMessage = "You have requested that your password for " + std::string(jlwe.config.at("websiteDomain")) + " be reset.\n\n";
                        plainMessage += "Please reset your password at this link: " + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/cgi-bin/password/reset.cgi?reset=" + token + "\n";

                        Email makeEmail;
                        makeEmail.setHtml(htmlMessage);
                        makeEmail.setPlainText(plainMessage);

                        if (!makeEmail.sendEmail(email_in, "noreply@" + std::string(jlwe.config.at("websiteDomain")), "[June LWE Geocaching] Password Reset Request", std::string(jlwe.config.at("mailerAddress")), "June LWE Geocaching")) {
                            std::cout << JsonUtils::makeJsonSuccess("Password reset email send to " + email_in);
                        } else {
                            std::cout << JsonUtils::makeJsonError("Error sending email to: " + email_in);
                        }
                    } else {
                        std::cout << JsonUtils::makeJsonError("Error sending email to: " + email_in);
                    }
                    delete res;
                    delete prep_stmt;

                } else {
                    std::cout << JsonUtils::makeJsonError("Invalid email: " + email_in);
                }
            } else {
                std::cout << JsonUtils::makeJsonError("Invalid email: " + email_in);
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
