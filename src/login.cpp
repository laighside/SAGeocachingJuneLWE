/**
  @file    login.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This handles login requests, which are POST requests with username/password

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <cstdlib>

#include "core/CgiEnvironment.h"
#include "core/Encoder.h"
#include "core/HtmlTemplate.h"
#include "core/JlweCore.h"
#include "core/JlweUtils.h"
#include "core/KeyValueParser.h"
#include "core/PostDataParser.h"
#include "password/Password.h"

int main () {
    try{
        JlweCore jlwe;

        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            HtmlTemplate::outputHttpHtmlHeader();

            std::cout << "<html><head>\n";
            std::cout << "<title>Login - error</title>\n";
            std::cout << "</head><body>\n";
            std::cout << "<h2>Login - error</h2>\n";

            std::cout << "<p>" << postData.errorText() << "<br/>\n";
            std::cout << "<a href=/login.html>Try again</a></p>\n";

            std::cout << "</body></html>\n";
            return 0;
        }
        postData.parseUrlEncodedForm();

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        std::string username = postData.getValue("username");

        int attempt_count = 0;
        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT COUNT(*) FROM login_attempts WHERE attempt_time > DATE_SUB(NOW(), INTERVAL 1 DAY) AND username = ?;");
        prep_stmt->setString(1, username);
        res = prep_stmt->executeQuery();
        if (res->next()){
            attempt_count = res->getInt(1);
        }
        delete res;
        delete prep_stmt;

        // max of 100 login attempts per day
        if (attempt_count > 100) {
            HtmlTemplate::outputHttpHtmlHeader();
            std::cout << "<html>\n";
            std::cout << "<head><title>JLWE - Login</title></head>\n";
            std::cout << "<body>\n";

            std::cout << "<p>Too many login attempts. Please wait before trying again.</p>\n";

            std::cout << "</body>\n";
            std::cout << "</html>\n";

            return 0;
        }


        std::string pass_hash = "";
        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT pass_hash FROM users WHERE username = ? AND active = 1;");
        prep_stmt->setString(1, username);
        res = prep_stmt->executeQuery();
        if (res->next()){
            pass_hash = res->getString(1);
        }
        delete res;
        delete prep_stmt;

        bool password_correct = pass_hash.size() && username.size() && Password::checkPassword(postData.getValue("password"), pass_hash);

        // log attempt
        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT loginAttempt(?, ?, ?);");
        prep_stmt->setString(1, username);
        prep_stmt->setString(2, jlwe.getCurrentUserIP());
        prep_stmt->setInt(3, password_correct);
        res = prep_stmt->executeQuery();
        delete res;
        delete prep_stmt;

        if (password_correct){ //password correct

            //create random token
            std::string token = JlweUtils::makeRandomToken(96);

            //set token in database
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setToken(?, ?, ?);");
            prep_stmt->setString(1, username);
            prep_stmt->setString(2, jlwe.getCurrentUserIP());
            prep_stmt->setString(3, token);
            res = prep_stmt->executeQuery();
            delete res;
            delete prep_stmt;

            //success html
            std::string redirect = urlQueries.getValue("redirect");
            if (redirect.size() == 0){
                redirect = "/cgi-bin/admin_index.cgi";
            }
            std::cout << "Set-Cookie:accessToken=" << token << ";Domain=" << std::string(jlwe.config.at("websiteDomain")) << "; Max-Age=10000000; Path=/; Secure; HttpOnly;\r\n";
            std::cout << "Content-type:text/html\r\n\r\n";

            std::cout << "<html>\n";
            std::cout << "<head>\n";
            std::cout << "<title>JLWE - Login</title>\n";
            std::cout << "<META HTTP-EQUIV=REFRESH CONTENT=1;url=\"" + redirect + "\">\n";
            std::cout << "</head>\n";
            std::cout << "<body>\n";
            std::cout << "<h2>JLWE - Login</h2>\n";

            std::cout << "<p>Login successful!</p>\n";
            std::cout << "<p><a href=" + redirect + ">Click here to return</a></p>\n";

            std::cout << "</body>\n";
            std::cout << "</html>\n";

        } else { // password incorrect
            HtmlTemplate::outputHttpHtmlHeader();

            std::cout << "<html>\n";
            std::cout << "<head>\n";
            std::cout << "<title>JLWE - Login</title>\n";
            std::cout << "<META HTTP-EQUIV=REFRESH CONTENT=1;url=/login.html>\n";
            std::cout << "</head>\n";
            std::cout << "<body>\n";
            std::cout << "<h2>JLWE - Login</h2>\n";

            std::cout << "<p>Incorrect username/password<br/>\n";
            std::cout << "<a href=/login.html>Try again</a></p>\n";

            std::cout << "</body>\n";
            std::cout << "</html>\n";
        }

    } catch (const sql::SQLException &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << e.what() << " (MySQL error code: " << std::to_string(e.getErrorCode()) << ")\n";
    } catch (const std::exception &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << e.what();
    }

    return 0;
}
