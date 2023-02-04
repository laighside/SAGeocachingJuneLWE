/**
  @file    send_reset_email.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  /cgi-bin/password/send_reset_email.cgi
  Return format is always HTML
  Sends a password reset email to the given email
  The email is in the URL encoded form POST data

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/PostDataParser.h"
#include "../core/JlweUtils.h"
#include "../core/JlweCore.h"
#include "../core/HtmlTemplate.h"
#include "../email/Email.h"

int main () {
    try {
        JlweCore jlwe;

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            HtmlTemplate::outputHttpHtmlHeader();

            std::cout << "<html><head>\n";
            std::cout << "<title>Login - error</title>\n";
            std::cout << "</head><body>\n";
            std::cout << "<h2>Login - error</h2>\n";

            std::cout << "<p>" << postData.errorText() << "<br/>\n";
            std::cout << "<a href=/password_reset.html>Try again</a></p>\n";

            std::cout << "</body></html>\n";
            return 0;
        }
        postData.parseUrlEncodedForm();

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;


        // limit email to 100 characters in length
        std::string email_in = postData.getValue("email").substr(0, 100);


        // check that the email is in the database
        bool validEmail = false;
        int target_user_id = -1;
        std::string email_db = "";
        if (email_in.size()) {
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT user_id,email FROM users WHERE email = ?;");
            prep_stmt->setString(1, email_in);
            res = prep_stmt->executeQuery();
            if (res->next()){
                target_user_id = res->getInt(1);
                email_db = res->getString(2);
                validEmail = true;
            }
            delete res;
            delete prep_stmt;
        }


        // log attempt
        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT loginAttempt(?, ?, ?);");
        prep_stmt->setString(1, "reset_email: " + email_in);
        prep_stmt->setString(2, jlwe.getCurrentUserIP());
        prep_stmt->setInt(3, validEmail);
        res = prep_stmt->executeQuery();
        if (res->next()){
        }
        delete res;
        delete prep_stmt;


        if (validEmail) {

            // make reset token
            std::string token = JlweUtils::makeRandomToken(64);

            // save the token in the database
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT resetPasswordRequest(?,?,?,?);");
            prep_stmt->setString(1, email_db);
            prep_stmt->setString(2, token);
            prep_stmt->setString(3, jlwe.getCurrentUserIP());
            prep_stmt->setString(4, jlwe.getCurrentUsername());
            res = prep_stmt->executeQuery();
            if (res->next() && res->getInt(1) == 0){

                // make the password reset email
                std::string htmlMessage = "<p>You have requested that your password for " + std::string(jlwe.config.at("websiteDomain")) + " be reset.</p>\n";
                htmlMessage += "<p>Please reset your password by clicking here: <a href=\"" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/cgi-bin/password/reset.cgi?reset=" + token + "\">" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/cgi-bin/password/reset.cgi?reset=" + token + "</a></p>\n";
                htmlMessage += "<p>If you did not request this reset, please contact " + std::string(jlwe.config.at("adminEmail")) + "</p>\n";

                std::string plainMessage = "You have requested that your password for " + std::string(jlwe.config.at("websiteDomain")) + " be reset.\n\n";
                plainMessage += "Please reset your password at this link: " + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/cgi-bin/password/reset.cgi?reset=" + token + "\n";
                plainMessage += "If you did not request this reset, please contact " + std::string(jlwe.config.at("adminEmail")) + "\n";

                Email makeEmail;
                makeEmail.setHtml(htmlMessage);
                makeEmail.setPlainText(plainMessage);

                makeEmail.sendEmail(email_db, "noreply@" + std::string(jlwe.config.at("websiteDomain")), "[June LWE Geocaching] Password Reset Request", std::string(jlwe.config.at("mailerAddress")), "June LWE Geocaching");
            }
            delete res;
            delete prep_stmt;
        }

        // output HTML page
        HtmlTemplate html(true);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "Reset Password", false))
            return 0;

        //content
        std::cout << "<h1>Reset Password</h1>\n";
        std::cout << "<p>A password reset link has been sent to your email inbox (if the address provided was found in our systems)</p>\n";
        std::cout << "<p>If you require assistance, please contact " + std::string(jlwe.config.at("adminEmail")) + "</p>\n";

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
