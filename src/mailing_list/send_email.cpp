/**
  @file    send_email.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/mailing_list/send_email.cgi
  This sends an email to the mailing list. Submitting the form at /cgi-bin/mailing_list/write_email.cgi ends up here.
  POST requests only

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
#include "../email/Email.h"
#include "../email/EmailTemplates.h"

int main () {
    try {
        JlweCore jlwe;

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << "Content-type:text/html\r\n\r\n";

            std::cout << "<html><head>\n";
            std::cout << "<title>Send email - error</title>\n";
            std::cout << "</head><body>\n";
            std::cout << "<h2>Send email - error</h2>\n";

            std::cout << "<p>" << postData.errorText() << "<br/>\n";
            std::cout << "<a href=/cgi-bin/mailing_list/write_email.cgi>Try again</a></p>\n";

            std::cout << "</body></html>\n";
            return 0;
        }
        postData.parseUrlEncodedForm();

        sql::Statement *stmt;
        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - Mailing List", false))
            return 0;

        std::cout << "<h1>Send email to mailing list</h1>\n";

        if (jlwe.getPermissionValue("perm_email")) { //if logged in
            bool fileError = false;
            int sendCount = 0;

            std::string replyEmail = postData.getValue("reply");
            std::string subject = postData.getValue("subject");
            std::string message = postData.getValue("message");
            bool sendToAll = (postData.getValue("sendToAll") == "true");

            if (!Email::isValidEmail(replyEmail))
                throw std::invalid_argument("Invalid reply to address: " + replyEmail);

            std::string file_dir = jlwe.config.at("files").at("directory");

            int email_count = 0;
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT COUNT(*) FROM email_list WHERE verify = 1 AND unsubscribed = 0;");
            if (res->next()) {
                email_count = res->getInt(1);
            }
            delete res;
            delete stmt;

            Email makeEmail;

            // get attachments
            std::cout << "<p>Attaching files...<br />\n";
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT CONCAT(directory,filename), filename FROM files;");
            while (res->next()) {
                std::string filename = res->getString(1);
                if (postData.getValue(filename) == "true") {
                    std::string full_filename = file_dir + filename;

                    if (makeEmail.addAttachmentFile(full_filename, res->getString(2))) {
                        std::cout << Encoder::htmlEntityEncode(filename) << ": <span style=\"color:red;\">failed to attach</span><br />\n";
                        fileError = true;
                    } else {
                        std::cout << Encoder::htmlEntityEncode(filename) << ": attached<br />\n";
                    }
                }
            }
            delete res;
            delete stmt;
            std::cout << "Total attachments: " + std::to_string(makeEmail.numberOfAttachments()) + "</p>\n";

            if (fileError == false) {

                // send to everyone on list
                std::cout << "<p>Sending emails...<br />\n";
                stmt = jlwe.getMysqlCon()->createStatement();
                res = stmt->executeQuery("SELECT email, unsub_token FROM email_list WHERE verify = 1 AND unsubscribed = 0;");
                while (res->next()) {
                    std::string toEmail = res->getString(1);

                    bool emailChecked = (postData.getValue("checkbox_" + Encoder::htmlAttributeEncode(toEmail)) == "true");

                    if (sendToAll || emailChecked) {
                        // add unsubscribe link to email
                        std::string emailHtml = "<html><body><p>" + JlweUtils::replaceString(Encoder::htmlEntityEncode(message), "\n", "<br />\n") + "</p>\n";
                        emailHtml += "<p>To unsubscribe from this mailing list, please <a href=\"" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(MANAGE_SUBSCRIPTION_URL) + "?unsubscribe=" + res->getString(2) + "\">click here.</a></p></body></html>\r\n\r\n";
                        makeEmail.setHtml(emailHtml);

                        std::string emailPlain = message + "\n\n";
                        emailPlain += "To unsubscribe from this mailing list, please visit " + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(MANAGE_SUBSCRIPTION_URL) + "?unsubscribe=" + res->getString(2) + "\r\n\r\n";
                        makeEmail.setPlainText(emailPlain);

                        if (makeEmail.sendEmail(toEmail, replyEmail, "[June LWE Geocaching] " + subject, std::string(jlwe.config.at("mailerAddress")), "June LWE Geocaching")) {
                            std::cout << Encoder::htmlEntityEncode(toEmail) << ": <span style=\"color:red;\">fail</span><br />\n";
                        } else {
                            std::cout << Encoder::htmlEntityEncode(toEmail) << ": success<br />\n";
                            sendCount++;
                        }
                    }
                }
                delete res;
                delete stmt;

                std::cout << "Emails sent successfully to " + std::to_string(sendCount) + " of " + std::to_string(email_count) + " recipients.</p>\n";

                // write to user log
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT log_user_event(?,?,?);");
                prep_stmt->setString(1, jlwe.getCurrentUserIP());
                prep_stmt->setString(2, jlwe.getCurrentUsername());
                prep_stmt->setString(3, "Email sent to " + std::to_string(sendCount) + " of " + std::to_string(email_count) + " recipients on mailing list.");
                res = prep_stmt->executeQuery();
                res->next();
                delete res;
                delete prep_stmt;

            }

            std::cout << "<p><button onclick=\"location.href='/cgi-bin/admin_index.cgi'\" type=\"button\">Return to admin index</button></p>\n";

        } else {
            std::cout << "<p>You need to be logged in to view this area.</p>";
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
