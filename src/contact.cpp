/**
  @file    contact.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  When a user fills in the "Contact Us" from on the public website, this handles the response.
  The user's message gets saved in the contact_from table in the database. Due to large amount of spam they don't get sent to an email address.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "core/CgiEnvironment.h"
#include "core/Encoder.h"
#include "core/HtmlTemplate.h"
#include "core/HttpRequest.h"
#include "core/JlweUtils.h"
#include "core/JlweCore.h"
#include "core/KeyValueParser.h"
#include "core/PostDataParser.h"
#include "email/Email.h"

#include "ext/nlohmann/json.hpp"

int main () {
    try {

        JlweCore jlwe;

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << "Content-type:text/html\r\n\r\n";

            std::cout << "<html><head>\n";
            std::cout << "<title>Contact - error</title>\n";
            std::cout << "</head><body>\n";
            std::cout << "<h2>Contact - error</h2>\n";

            std::cout << "<p>" << postData.errorText() << "<br/>\n";
            std::cout << "<a href=/contact.html>Try again</a></p>\n";

            std::cout << "</body></html>\n";
            return 0;
        }
        postData.parseUrlEncodedForm();

        bool blocked = false;
        std::string result = "Something has gone wrong. Please let us know at " + std::string(jlwe.config.at("adminEmail"));

        std::string g_recaptcha_response = postData.getValue("g-recaptcha-response");
        std::string captchaPost = "secret=" + std::string(jlwe.config.at("recaptcha").at("key")) + "&response=" + g_recaptcha_response + "&remoteip=" + jlwe.getCurrentUserIP();

        std::string captchaResponse = "{}";
        HttpRequest request(jlwe.config.at("recaptcha").at("url"));
        if (request.post(captchaPost, "application/x-www-form-urlencoded")) {
            captchaResponse = request.responseAsString();
        } else {
            blocked = true;
            result = "Unable to verify CAPTCHA, please try again later or contact " + std::string(jlwe.config.at("adminEmail"));
        }
        nlohmann::json captchaResponseJson = nlohmann::json::parse(captchaResponse);

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        std::string email = postData.getValue("email");
        std::string from_name = Encoder::removeNewLines(postData.getValue("name"));

        // this probably isn't needed but just to be safe
        from_name = Encoder::htmlEntityEncode(from_name);

        //check for invalid email
        if (!Email::isValidEmail(email)){
            blocked = true;
            result = Encoder::htmlEntityEncode(email) + " is not a valid email address.";
        }

        //check if blocked ip
        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT * FROM blocked_ips WHERE ip = ?;");
        prep_stmt->setString(1, jlwe.getCurrentUserIP());
        res = prep_stmt->executeQuery();
        if (res->next()){
            blocked = true;
            result = "This IP address has been blocked due to spam.";
        }
        delete res;
        delete prep_stmt;

        if (!jlwe.config.value("contactEnable", false)) {
            blocked = true;
            result = "The contact form has been disabled due to spam. Please send your email directly to " + std::string(jlwe.config.at("adminEmail"));
        }

        if (captchaResponseJson.value("success", false) == false) {
            blocked = true;
            result = "CAPTCHA fail, please try again or send your email directly to " + std::string(jlwe.config.at("adminEmail"));
        }

        // just quietly ignore spam from Eric Jones
        if (from_name == "Eric Jones") {
            blocked = true;
            result = "Thankyou for your email. We will get back to you as soon as possible.";
        }

        // send the email if the user isn't blocked
        if (blocked == false) {

            std::string userMessage = postData.getValue("message");

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT insertContactForm(?,?,?,?,?);");
            prep_stmt->setString(1, from_name);
            prep_stmt->setString(2, email);
            prep_stmt->setString(3, userMessage);
            prep_stmt->setString(4, jlwe.getCurrentUserIP());
            prep_stmt->setString(5, jlwe.getCurrentUsername());
            res = prep_stmt->executeQuery();
            if (res->next() && res->getInt(1)) {
                result = "Thankyou for your email. We will get back to you as soon as possible.";
            }
            delete res;
            delete prep_stmt;
        }

        //start of html output
        HtmlTemplate html(true);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "Contact Us", false))
            return 0;

        //content
        std::cout << "<h1>Contact Us</h1>\n";
        std::cout << "<p>" << result << "</p>\n";

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
