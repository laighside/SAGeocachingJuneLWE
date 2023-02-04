/**
  @file    change.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This script changes a user's password. It is called from the change_password.html page

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/KeyValueParser.h"
#include "../core/PostDataParser.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "Password.h"

int main () {
    try {
        JlweCore jlwe;

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << "Content-type:text/html\r\n\r\n";

            std::cout << "<html><head>\n";
            std::cout << "<title>Change Password - error</title>\n";
            std::cout << "</head><body>\n";
            std::cout << "<h2>Change Password - error</h2>\n";

            std::cout << "<p>" << postData.errorText() << "<br/>\n";
            std::cout << "<a href=/change_password.html>Try again</a></p>\n";

            std::cout << "</body></html>\n";
            return 0;
        }

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "Change Password", false))
            return 0;

        if (jlwe.isLoggedIn()) { //if logged in

            KeyValueParser postDataValues(postData.dataAsString(), true);

            // get current password hash
            std::string pass_hash = "";
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT pass_hash FROM users WHERE user_id = ?;");
            prep_stmt->setInt(1, jlwe.getCurrentUserId());
            res = prep_stmt->executeQuery();
            if (res->next()){
                pass_hash = res->getString(1);
            }
            delete res;
            delete prep_stmt;

            if (pass_hash.size() > 0 && Password::checkPassword(postDataValues.getValue("old_password"), pass_hash)){ // old password correct

                std::string new_password = postDataValues.getValue("new_password");
                if (new_password.size() >= MIN_PASSWORD_LENGTH) { // Minimum password length
                    std::string new_hash = Password::makeNewHash(new_password);

                    //store new hash in mysql
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setPasswordHash(?, ?, ?, ?);");
                    prep_stmt->setString(1, new_hash);
                    prep_stmt->setInt(2, jlwe.getCurrentUserId());
                    prep_stmt->setString(3, jlwe.getCurrentUserIP());
                    prep_stmt->setString(4, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    if (res->next() && res->getInt(1)){
                        std::cout << "<p>Password changed successful!</p>\n";
                    } else {
                        std::cout << "<p>Error: User not found in database</p>\n";
                    }
                    delete res;
                    delete prep_stmt;


                } else {
                    std::cout << "<p>Error: New password must be " << MIN_PASSWORD_LENGTH << " characters or longer.</p>\n";
                }

            } else { //old password incorrect
                std::cout << "<p>Error: The old password is incorrect.</p>\n";
            }
        } else { //not logged in
            std::cout << "<p>You need to be logged in to access this area.</p>\n";
        }

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
