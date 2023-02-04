/**
  @file    reset.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  /cgi-bin/password/reset.cgi?reset=abc...
  Return format is always HTML
  Page for entering a new password after clicking the link in a password reset email
  GET requests return the form to enter the new password
  POST requests save the new password

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/KeyValueParser.h"
#include "../core/PostDataParser.h"
#include "../core/HtmlTemplate.h"
#include "../core/Encoder.h"
#include "../core/JlweUtils.h"
#include "../core/JlweCore.h"
#include "Password.h"

int main () {
    try {
        JlweCore jlwe;

        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        std::string reset_token = urlQueries.getValue("reset");
        std::string result = "Something has gone wrong. Please let us know at " + std::string(jlwe.config.at("adminEmail"));

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        // check that the supplied reset token is valid
        std::string reset_username = "";
        int reset_id = -1;
        if (reset_token.size()){
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT user_id,username FROM users WHERE reset_token = ?;");
            prep_stmt->setString(1, reset_token);
            res = prep_stmt->executeQuery();
            if (res->next()){
                reset_id = res->getInt(1);
                reset_username = res->getString(2);
            }
            delete res;
            delete prep_stmt;
        }

        // output the HTML page
        HtmlTemplate html(true);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "Reset Password", false))
            return 0;

        // if valid token
        if (reset_id > 0) {

            // if its a POST request then parse the new password from the POST data and save it
            if (JlweUtils::compareStringsNoCase(CgiEnvironment::getRequestMethod(), "post")) {

                PostDataParser postData(jlwe.config.at("maxPostSize"));
                if (postData.hasError()) {
                    std::cout << "<p>" << postData.errorText() << "</p>\n";
                } else {
                    std::string new_password = KeyValueParser(postData.dataAsString(), true).getValue("new_password");

                    if (new_password.size() >= MIN_PASSWORD_LENGTH) { // Minimum password length
                        std::string hash_new = Password::makeNewHash(new_password);

                        // store new hash in mysql
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setPasswordHash(?, ?, ?, ?);");
                        prep_stmt->setString(1, hash_new);
                        prep_stmt->setInt(2, reset_id);
                        prep_stmt->setString(3, jlwe.getCurrentUserIP());
                        prep_stmt->setString(4, "Reset_email");
                        res = prep_stmt->executeQuery();
                        if (res->next() && res->getInt(1)){
                            std::cout << "<p>Password reset successful! <a href=\"/login.html\">Click here to login</p></p>\n";
                        } else {
                            std::cout << "<p>Error: User not found in database</p>\n";
                        }
                        delete res;
                        delete prep_stmt;

                    }else{
                        std::cout << "<p>Error: New password must be " << MIN_PASSWORD_LENGTH << " characters or longer.</p>\n";
                    }
                }
            } else { // if its not a POST request then display the form to enter a new password
                std::cout << "<form action=\"/cgi-bin/password/reset.cgi?reset=" << Encoder::htmlAttributeEncode(Encoder::urlEncode(reset_token)) << "\" method=\"POST\">\n";
                std::cout << "    <table border=\"0\" align=\"left\" style=\"border:0\">\n";
                std::cout << "            <tr style=\"border:0\">\n";
                std::cout << "              <td style=\"border:0\"><span style=\"float:right\">User:</span></td>\n";
                std::cout << "              <td style=\"border:0\">" << Encoder::htmlEntityEncode(reset_username) << "</td>\n";
                std::cout << "            </tr>\n";
                std::cout << "            <tr style=\"border:0\">\n";
                std::cout << "              <td style=\"border:0\"><span style=\"float:right\">New Password:</span></td>\n";
                std::cout << "              <td style=\"border:0\"><input name=\"new_password\" type=\"password\"></td>\n";
                std::cout << "            </tr>\n";
                std::cout << "            <tr style=\"border:0\">\n";
                std::cout << "              <td colspan=\"2\" align=\"center\" style=\"border:0\"><input type=\"submit\" value=\"Submit\"></td>\n";
                std::cout << "            </tr>\n";
                std::cout << "    </table>\n";
                std::cout << "</form>\n";
            }
        } else {
            std::cout << "<p>Invalid token.</p>";
        }

        // end of template
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
