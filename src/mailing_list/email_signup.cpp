/**
  @file    email_signup.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/mailing_list/email_signup.cgi
  This page handles the submission of the /email form where users can signup to the mailing list

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/Encoder.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/PostDataParser.h"
#include "../email/EmailTemplates.h"

int main () {
    try {
        JlweCore jlwe;

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << "Content-type:text/html\r\n\r\n";

            std::cout << "<html><head>\n";
            std::cout << "<title>JLWE - error</title>\n";
            std::cout << "</head><body>\n";
            std::cout << "<h2>JLWE - error</h2>\n";

            std::cout << "<p>" << postData.errorText() << "<br/>\n";
            std::cout << "<a href=/email_signup.html>Try again</a></p>\n";

            std::cout << "</body></html>\n";
            return 0;
        }
        postData.parseUrlEncodedForm();

        std::string email = postData.getValue("email");
        std::string result = "Something has gone wrong. Please let us know at " + std::string(jlwe.config.at("adminEmail"));

        if (EmailTemplates::addEmailtoMailingList(email, &jlwe, &result) == 0) {
            result = "A confirmation email has been sent to " + Encoder::htmlEntityEncode(email) + "</br>Please click the verification link provided in the email. If you do not verify your email, you will not receive the GPX file.";
        } else {
            result = Encoder::htmlEntityEncode(result);
        }

        // HTML output
        HtmlTemplate html(true);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Mailing List Signup", false))
            return 0;

        std::cout << "<h1>June LWE email list signup</h1>\n";
        std::cout << "<p>" << result << "</p>\n";

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
