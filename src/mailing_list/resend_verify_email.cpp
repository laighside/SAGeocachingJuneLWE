/**
  @file    resend_verify_email.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/mailing_list/resend_verify_email.cgi
  This allows admins to resend the verification email to individual users
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"
#include "../core/PostDataParser.h"
#include "../email/EmailTemplates.h"

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

        if (jlwe.getPermissionValue("perm_email")) { //if logged in

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            std::string email_address = jsonDocument.value("email", "");

            if (email_address.size()) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT verify_token, unsub_token FROM email_list WHERE email = ? AND unsubscribed = 0;");
                prep_stmt->setString(1, email_address);
                res = prep_stmt->executeQuery();
                if (res->next()) {

                    // write to user log
                    sql::PreparedStatement *prep_stmt_2;
                    sql::ResultSet *res_2;
                    prep_stmt_2 = jlwe.getMysqlCon()->prepareStatement("SELECT log_user_event(?,?,?);");
                    prep_stmt_2->setString(1, jlwe.getCurrentUserIP());
                    prep_stmt_2->setString(2, jlwe.getCurrentUsername());
                    prep_stmt_2->setString(3, "Mailing list verification email resent to \"" + email_address + "\"");
                    res_2 = prep_stmt_2->executeQuery();
                    res_2->next();
                    delete res_2;
                    delete prep_stmt_2;

                    if (EmailTemplates::sendMailingListSignupEmail(email_address, res->getString(1), res->getString(2), &jlwe) == 0) {
                        std::cout << JsonUtils::makeJsonSuccess("The verification email has been sent to \"" + email_address + "\" successfully.");
                    } else {
                        std::cout << JsonUtils::makeJsonError("Error re-sending email to \"" + email_address + "\"");
                    }
                } else {
                    std::cout << JsonUtils::makeJsonError("Email address \"" + email_address + "\" not found in mailing list.");
                }
            } else {
                std::cout << JsonUtils::makeJsonError("Invalid email address");
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
