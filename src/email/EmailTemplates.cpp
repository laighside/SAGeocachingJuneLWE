/**
  @file    EmailTemplates.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions that are used for email related things
  All functions are static so there is no need to create instances of the EmailTemplates object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "EmailTemplates.h"

#include "Email.h"
#include "../core/Encoder.h"
#include "../core/JlweUtils.h"

int EmailTemplates::sendMailingListSignupEmail(const std::string &email_address, const std::string &token_verify, const std::string &token_unsub, JlweCore *jlwe) {
    Email makeEmail;

    std::string htmlMessage = "<html><body><p>Your email has been added to the mailing list for the June LWE Geocaching event.</p>\n\n";
    htmlMessage += "<p>Please verify you email address by clicking here: <a href=\"" + std::string(jlwe->config.at("http")) + std::string(jlwe->config.at("websiteDomain")) + std::string(MANAGE_SUBSCRIPTION_URL) + "?verify=" + token_verify + "\">" + std::string(jlwe->config.at("websiteDomain")) + std::string(MANAGE_SUBSCRIPTION_URL) + "?verify=" + token_verify + "</a><br/>\n";
    htmlMessage += "If you do not verify your email address you may not receive the GPX file on game day.</p>\n\n";
    htmlMessage += "<p>To unsubscribe from this mailing list, please <a href=\"" + std::string(jlwe->config.at("http")) + std::string(jlwe->config.at("websiteDomain")) + std::string(MANAGE_SUBSCRIPTION_URL) + "?unsubscribe=" + token_unsub + "\">click here.</a></p></body></html>\n";
    makeEmail.setHtml(htmlMessage);

    std::string plainMessage = "Your email has been added to the mailing list for the June LWE Geocaching event.\n\n";
    plainMessage += "Please verify you email address at this URL: " + std::string(jlwe->config.at("http")) + std::string(jlwe->config.at("websiteDomain")) + std::string(MANAGE_SUBSCRIPTION_URL) + "?verify=" + token_verify + "\n";
    plainMessage += "If you do not verify your email address you may not receive the GPX file on game day.\n\n";
    plainMessage += "To unsubscribe from this mailing list, visit this URL: " + std::string(jlwe->config.at("http")) + std::string(jlwe->config.at("websiteDomain")) + std::string(MANAGE_SUBSCRIPTION_URL) + "?unsubscribe=" + token_unsub + "\n";
    makeEmail.setPlainText(plainMessage);

    return makeEmail.sendEmail(email_address, "noreply@" + std::string(jlwe->config.at("websiteDomain")), "[June LWE Geocaching] Email signup confirmation", std::string(jlwe->config.at("mailerAddress")), "June LWE Geocaching");
}

int EmailTemplates::addEmailtoMailingList(const std::string &email, JlweCore *jlwe, std::string *error) {
    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;
    int result = 1;

    //check for invalid email
    if (!Email::isValidEmail(email)) {
        if (error)
            *error = email + " is not a valid email address.";
        return result;
    }

    std::string token_verify = JlweUtils::makeRandomToken(64);
    std::string token_unsub  = JlweUtils::makeRandomToken(64);

    prep_stmt = jlwe->getMysqlCon()->prepareStatement("SELECT addEmailAddress(?,?,?,?,?)");
    prep_stmt->setString(1, email);
    prep_stmt->setString(2, token_verify);
    prep_stmt->setString(3, token_unsub);
    prep_stmt->setString(4, jlwe->getCurrentUserIP());
    prep_stmt->setString(5, jlwe->getCurrentUsername());
    res = prep_stmt->executeQuery();
    if (res->next()) {
        int r = res->getInt(1);
        if (r == 1)
            if (error)
                *error = "The email \"" + email + "\" has already been registered.";
        if (r == 0) {
            if (sendMailingListSignupEmail(email, token_verify, token_unsub, jlwe) == 0) {
                result = 0;
            } else {
                if (error)
                    *error = "An error has occured sending mail to \"" + email + "\". Please let us know at " + std::string(jlwe->config.at("adminEmail"));
            }
        }
    }
    delete res;
    delete prep_stmt;

    return result;
}
