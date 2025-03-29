/**
  @file    add_payments.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/registration/add_payments.cgi
  This page allows admins to enter payments users have made (via bank or cash at the event)
  GET requests just show the form for entering payments
  POST requests saves the payment info in the posted data (then show the form for the next payment)
  Hence pressing submit of the form calls a POST request to this page

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <ctime>

#include "../core/CgiEnvironment.h"
#include "../core/Encoder.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/KeyValueParser.h"
#include "../core/PaymentUtils.h"
#include "../core/PostDataParser.h"
#include "../email/JlweHtmlEmail.h"

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - Add Payments", false))
            return 0;

        if (jlwe.getPermissionValue("perm_registrations")) { //if logged in

            std::string payment_type = "bank"; // default payment type for form
            std::string postMessage = "";
            if (JlweUtils::compareStringsNoCase(CgiEnvironment::getRequestMethod(), "post")) { // if request is POST

                PostDataParser postData(jlwe.config.at("maxPostSize"));
                if (postData.hasError()) {
                   postMessage = postData.errorText();
                } else {

                    postData.parseUrlEncodedForm();

                    std::string userid = postData.getValue("userid");
                    std::string gc_user = postData.getValue("user");
                    payment_type = postData.getValue("payment_type");
                    int payment_amount = 0;
                    try {
                        payment_amount = static_cast<int>(std::stod(postData.getValue("payment_amount")) * 100);
                    } catch (...) {}

                    if (payment_type == "bank" || payment_type == "cash") {
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT insertPayment(?,?,?,?,?,?);");
                        prep_stmt->setString(1, userid);
                        prep_stmt->setInt64(2, time(nullptr));
                        prep_stmt->setInt(3, payment_amount);
                        prep_stmt->setString(4, payment_type);
                        prep_stmt->setString(5, jlwe.getCurrentUserIP());
                        prep_stmt->setInt(6, jlwe.getCurrentUserId());
                        res = prep_stmt->executeQuery();
                        if (res->next() && res->getInt(1) == 1) {
                            postMessage = "Payment of " + PaymentUtils::currencyToString(payment_amount) + " for " + gc_user + " has been successfully recorded.";

                            // send email to thank user for payment
                            std::string email_address = PaymentUtils::getUserEmail(jlwe.getMysqlCon(), userid);
                            if (email_address.size()) {
                                JlweHtmlEmail makeEmail;

                                makeEmail.addHtml("<p class=\"title\">Receipt of Payment</p>\n");
                                makeEmail.addHtml("<p>Hi " + Encoder::htmlEntityEncode(PaymentUtils::getUserName(jlwe.getMysqlCon(), userid)) + ",</p>\n");

                                if (payment_type == "cash")
                                    makeEmail.addHtml("<p>We have received " + PaymentUtils::currencyToString(payment_amount) + "</p>\n");
                                if (payment_type == "bank")
                                    makeEmail.addHtml("<p>We have received " + PaymentUtils::currencyToString(payment_amount) + " via bank transfer.</p>\n");

                                makeEmail.addHtml("<p>Thankyou for your payment. We look forward to seeing you at the event!</p>\n");

                                makeEmail.addHtml("<p>For assistance or changes to your registration, please reply to this email or contact us at contact@jlwe.org</p>\n");

                                makeEmail.sendJlweEmail(email_address, "contact@jlwe.org", "Receipt of Payment", std::string(jlwe.config.at("mailerAddress")), "June LWE Geocaching");
                            }


                        } else {
                            postMessage = "Error while recording payment, please try again. (MySQL)";
                        }
                        delete res;
                        delete prep_stmt;
                    } else {
                        postMessage = "Invalid payment type";
                    }
                }
            }


            std::cout << "<h1>Add Payments</h1>\n";
            std::cout << "<p><button onclick=\"location.href='/cgi-bin/registration/registration.cgi'\" type=\"button\">Return to registration list</button></p>\n";
            std::cout << "<p>This page is for entering payments as the appear on the bank transaction record. Only use this page if you have access to the bank account, or are receiving cash payments at the event.</p>\n";

            if (postMessage.size())
                std::cout << "<p style=\"color:red;\">" << Encoder::htmlEntityEncode(postMessage) << "</p>\n";
            std::cout << "<p id=\"page_note\" style=\"color: red;\"></p>\n";
            std::cout << "<form action=\"/cgi-bin/registration/add_payments.cgi\" method=\"POST\">\n";
            std::cout << "<p>Payment Type:&nbsp;&nbsp;\n";
            std::cout << "<span class=\"checkbox_container\"><label>Bank transfer\n";
            std::cout << "  <input type=\"radio\" name=\"payment_type\" id=\"payment_type_bank\" value=\"bank\" " << ((payment_type == "bank") ? "checked=\"true\" " : "") << "/>\n";
            std::cout << "  <span class=\"radiobox\"></span>\n";
            std::cout << "</label></span>&nbsp;&nbsp;\n";
            std::cout << "<span class=\"checkbox_container\"><label>Cash at the event\n";
            std::cout << "  <input type=\"radio\" name=\"payment_type\" id=\"payment_type_cash\" value=\"cash\" " << ((payment_type == "cash") ? "checked=\"true\" " : "") << "/>\n";
            std::cout << "  <span class=\"radiobox\"></span>\n";
            std::cout << "</label></span></p>\n";

            std::cout << "<p>Select User: <input type=\"text\" list=\"user_list\" name=\"user\" id=\"user\" onchange=\"usernameChanged()\"></p>\n";

            std::cout << "<div style=\"height:220px;\">\n";
            std::cout << "<p>Username: <span id=\"user_info_username\" style=\"font-weight:bold;\"></span><br/>\n";
            std::cout << "Email: <span id=\"user_info_email\"></span><br/>\n";
            std::cout << "Phone: <span id=\"user_info_phone\"></span></p>\n";
            std::cout << "<div id=\"user_info\"></div>\n";
            std::cout << "</div>\n";

            std::cout << "<p>Enter amount received: $<input type=\"number\" name=\"payment_amount\" step=\"0.01\" value=\"0.00\" style=\"width:70px;\"></p>\n";

            std::cout << "<datalist id=\"user_list\">\n";
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT idempotency, gc_username FROM event_registrations WHERE status = 'S';");
            while (res->next()) {
                std::cout << "<option value=\"" << Encoder::htmlAttributeEncode(res->getString(2)) << "\" id=\"" << Encoder::htmlAttributeEncode(res->getString(1))<< "\">\n";
            }
            delete res;
            delete stmt;

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT idempotency, gc_username FROM camping WHERE payment_type != 'event' AND status = 'S';");
            while (res->next()) {
                std::cout << "<option value=\"" << Encoder::htmlAttributeEncode(res->getString(2)) << " (Camping)\" id=\"" << Encoder::htmlAttributeEncode(res->getString(1)) << "\">\n";
            }
            delete res;
            delete stmt;

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT idempotency, gc_username, dinner_form_id FROM sat_dinner WHERE payment_type != 'event' AND status = 'S';");
            while (res->next()) {
                std::cout << "<option value=\"" << Encoder::htmlAttributeEncode(res->getString(2)) << " (Dinner " << res->getInt(3) << ")\" id=\"" << Encoder::htmlAttributeEncode(res->getString(1)) << "\">\n";
            }
            delete res;
            delete stmt;

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT idempotency, gc_username FROM merch_orders WHERE status = 'S';");
            while (res->next()) {
                std::cout << "<option value=\"" << Encoder::htmlAttributeEncode(res->getString(2)) << " (Merchandise)\" id=\"" << Encoder::htmlAttributeEncode(res->getString(1)) << "\">\n";
            }
            delete res;
            delete stmt;
            std::cout << "</datalist>\n";

            std::cout << "<input type=\"hidden\" name=\"userid\" id=\"userid\">\n";
            std::cout << "<p>When you click submit, the user will receive an email thanking them for their payment. Please double check payment details are correct before clicking submit.</p>\n";
            std::cout << "<p><input type=\"submit\"></p>\n";
            std::cout << "</form></p>\n";


            std::cout << "<script>\n";
            std::cout << "function usernameChanged() {\n";
            std::cout << "    var options = document.querySelectorAll('#user_list option');\n";
            std::cout << "    var idInput = document.getElementById(\"userid\");\n";
            std::cout << "    var inputValue = document.getElementById(\"user\").value;\n";
            std::cout << "    var idKey = \"\";\n";

            std::cout << "    for(var i = 0; i < options.length; i++) {\n";
            std::cout << "        var option = options[i];\n";

            std::cout << "        if(option.getAttribute('value') === inputValue) {\n";
            std::cout << "            idKey = option.id;\n";
            std::cout << "            break;\n";
            std::cout << "        }\n";
            std::cout << "    }\n";
            std::cout << "    idInput.value = idKey;\n";

            std::cout << "    downloadUrl('/cgi-bin/registration/get_details.cgi?key=' + encodeURI(idKey), null,\n";
            std::cout << "        function(data, responseCode) {\n";

            std::cout << "            if (responseCode === 200){\n";
            std::cout << "                var jsonObj = JSON.parse(data);\n";

            std::cout << "                var user_html = '';\n";
            std::cout << "                if (jsonObj.error == null){\n";
            std::cout << "                    document.getElementById(\"user_info_username\").innerText = jsonObj.gc_username;\n";
            std::cout << "                    document.getElementById(\"user_info_email\").innerText = jsonObj.email_address;\n";
            std::cout << "                    document.getElementById(\"user_info_phone\").innerText = jsonObj.phone_number;\n";

            std::cout << "                    user_html += '<p>Event: ';\n";
            std::cout << "                    if (jsonObj.event != null && typeof jsonObj.event == 'object') {\n";
            std::cout << "                        user_html += jsonObj.event.number_adult.toString() + ' adults, ' + jsonObj.event.number_child.toString() + ' childern<br/>';\n";
            std::cout << "                    } else {\n";
            std::cout << "                        user_html += 'none (camping/dinner only)<br/>';\n";
            std::cout << "                    }\n";
            std::cout << "                    user_html += 'Camping: ';\n";
            std::cout << "                    if (jsonObj.camping != null && typeof jsonObj.camping == 'object') {\n";
            std::cout << "                        var number_nights = jsonObj.camping.leave_date - jsonObj.camping.arrive_date;\n";
            std::cout << "                        user_html += jsonObj.camping.camping_type + ' site, ' + jsonObj.camping.number_people.toString() + ' people, ' + number_nights.toString() + ' nights (' + jsonObj.camping.arrive_date.toString() + ' to ' + jsonObj.camping.leave_date.toString() + ')<br/>';\n";
            std::cout << "                    } else {\n";
            std::cout << "                        user_html += 'none<br/>';\n";
            std::cout << "                    }\n";
            std::cout << "                    if (jsonObj.dinner != null && typeof jsonObj.dinner == 'object') {\n";
            std::cout << "                        if (jsonObj.dinner.length > 0) {\n";
            std::cout << "                            for (var i = 0; i < jsonObj.dinner.length; i++)\n";
            std::cout << "                                user_html += jsonObj.dinner[i].title + ': ' + jsonObj.dinner[i].meals.toString() + ' meals<br />';\n";
            std::cout << "                            user_html += '</p>';\n";
            std::cout << "                        } else {\n";
            std::cout << "                            user_html += 'Dinner: none</p>';\n";
            std::cout << "                        }\n";
            std::cout << "                    } else {\n";
            std::cout << "                        user_html += 'Dinner: none</p>';\n";
            std::cout << "                    }\n";
            std::cout << "                    user_html += 'Merchandise: ';\n";
            std::cout << "                    if (jsonObj.merch != null && typeof jsonObj.merch == 'object') {\n";
            std::cout << "                        user_html += jsonObj.merch.item_count.toString() + ' merchandise items</p>';\n";
            std::cout << "                    } else {\n";
            std::cout << "                        user_html += 'none</p>';\n";
            std::cout << "                    }\n";

            std::cout << "                    user_html += '<p>Total cost: $' + (jsonObj.payment_total / 100).toFixed(2) + '<br/>';\n";
            std::cout << "                    user_html += 'Payment received: $' + (jsonObj.payment_received / 100).toFixed(2) + '<br/>';\n";
            std::cout << "                    var amount_owing = jsonObj.payment_total - jsonObj.payment_received;\n";
            std::cout << "                    user_html += '<span';\n";
            std::cout << "                    if (amount_owing > 0) {\n";
            std::cout << "                        user_html += ' style=\"color: red;\"';\n";
            std::cout << "                    }\n";
            std::cout << "                    user_html += '>Payment owing: $' + (amount_owing / 100).toFixed(2) + '</span></p>';\n";
            std::cout << "                }else{\n";
            std::cout << "                    document.getElementById(\"user_info_username\").innerText = 'Error';\n";
            std::cout << "                    document.getElementById(\"user_info_email\").innerText = '';\n";
            std::cout << "                    document.getElementById(\"user_info_phone\").innerText = '';\n";
            std::cout << "                    user_html += '<p style=\"color:red;\">Error: ' + jsonObj.error + '</p>';\n";
            std::cout << "                }\n";
            std::cout << "                document.getElementById(\"user_info\").innerHTML = user_html;\n";
            std::cout << "            }\n";
            std::cout << "     });\n";

            std::cout << "}\n";
            std::cout << "</script>\n";

            std::cout << "<script type=\"text/javascript\" src=\"/js/utils.js\"></script>\n";

        } else {
            if (jlwe.isLoggedIn()) {
                std::cout << "<p>You don't have permission to view this area.</p>";
            } else {
                std::cout << "<p>You need to be logged in to view this area.</p>";
            }
        }

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
