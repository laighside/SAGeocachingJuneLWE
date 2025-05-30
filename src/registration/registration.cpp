/**
  @file    registration.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/registration/registration.cgi
  This page shows the list of registrations to the admins.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/Encoder.h"
#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/PaymentUtils.h"
#include "DinnerUtils.h"

struct team_items {
    std::string username;
    std::string email;
    std::string user_key;
    int event_people;
    int camping_people;
    std::vector<int> dinner_people;
};

team_items * findTeam(std::vector<team_items> * team_list, const std::string &username) {
    for (unsigned int i = 0; i < team_list->size(); i++) {
        if (JlweUtils::compareTeamNames(team_list->at(i).username, username)) {
            return &team_list->at(i);
        }
    }
    return nullptr;
}

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - Event Registrations", false))
            return 0;

        if (jlwe.getPermissionValue("perm_registrations")) { // if logged in

            if (jlwe.config.at("stripe").value("testMode", false)) {
                std::cout << "<div class=\"note\"><p><span style=\"font-weight:bold;\">Stripe test mode is enabled.</span> See <a href=\"https://stripe.com/docs/testing\">https://stripe.com/docs/testing</a></p></div>\n";
            }

            time_t jlwe_date = 0;
            try {
                jlwe_date = std::stoll(jlwe.getGlobalVar("jlwe_date"));
            } catch (...) {}

            html.outputAdminMenu();

            // List of dinner forms
            std::vector<DinnerUtils::dinner_form> dinner_forms = DinnerUtils::getDinnerFormList(jlwe.getMysqlCon());

            // List of what teams have bought so far
            std::vector<team_items> team_list;

            std::cout << "<h2 style=\"text-align:center\">JLWE Event Registrations</h2>\n";
            std::cout << "<p style=\"text-align:center;\">\n";
            std::cout << "<a class=\"admin_button\" style=\"height:95px;\" href=\"/cgi-bin/registration/download_event_registrations.cgi\"><span>Download Spreadsheet<br/><span style=\"font-size:16px;\">(full)</span></span></a>\n";
            std::cout << "<a class=\"admin_button\" style=\"height:95px;\" href=\"/cgi-bin/registration/download_event_registrations.cgi?simple=true\"><span>Download Spreadsheet<br/><span style=\"font-size:16px;\">(simple)</span></span></a><br/>\n";
            std::cout << "<a class=\"admin_button\" style=\"height:95px;\" href=\"/cgi-bin/registration/download_dinner_orders.cgi\"><span>Download Dinner Orders</span></a>\n";
            std::cout << "<a class=\"admin_button\" href=\"/cgi-bin/registration/add_payments.cgi\"><span>Add Payments</span></a>\n";
            std::cout << "</p>\n";


            std::cout << "<h2 style=\"text-align:center\">List of Registrations</h2>\n";
            std::cout << "<p style=\"text-align:right;\">Show Saved registrations only: " << FormElements::htmlSwitch("savedToggleCB", true) << "</p>\n";

            std::cout << "<h3 style=\"text-align:center\">Event registrations</h3>\n";
            std::cout << "<table class=\"reg_table\" align=\"center\" style=\"width: 100%;\"><tr>\n";
            std::cout << "<th>Email</th><th>Username</th><th>Phone number</th><th>#Adults</th><th>#Children</th><th>Total cost</th><th>Paid</th><th>Payment</th><th>Status</th><th></th>\n";
            std::cout << "</tr>\n";

            std::vector<int> username_count_per_day;
            std::vector<int> people_count_per_day;

            int rowId = 0;

            int adult_count = 0;
            int child_count = 0;
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT registration_id,idempotency,email_address,gc_username,phone_number,number_adults,number_children,payment_type,status,UNIX_TIMESTAMP(timestamp) FROM event_registrations;");
            while (res->next()) {
                std::string userKey = res->getString(2);
                bool saved = (res->getString(9) == "S");

                std::cout << "<tr" << (saved ? "" : " class=\"not_s\" style=\"background-color: #ba8866;\"") <<">\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(3)) << "</td>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(4)) << "</td>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(5)) << "</td>\n";
                std::cout << "<td>" << res->getInt(6) << "</td>\n";
                std::cout << "<td>" << res->getInt(7) << "</td>\n";
                int user_cost = PaymentUtils::getUserCost(jlwe.getMysqlCon(), userKey);
                int payment_recived = PaymentUtils::getTotalPaymentReceived(jlwe.getMysqlCon(), userKey);
                bool needToPay = ((user_cost - payment_recived) > 0);
                bool overpaid = ((user_cost - payment_recived) < 0);
                std::string background_color = ((needToPay && saved) ? "#FFC0C0" : ((overpaid && saved) ? "#80FFFF" : ""));
                std::cout << "<td" << (background_color.size() > 0 ? (" style=\"background-color:" + background_color + ";\"") : "") << ">" << PaymentUtils::currencyToString(user_cost) << "</td>\n";
                std::cout << "<td" << (background_color.size() > 0 ? (" style=\"background-color:" + background_color + ";\"") : "") << ">" << PaymentUtils::currencyToString(payment_recived) << "</td>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(8)) << "</td>\n";
                std::string status = "Unknown";
                if (res->getString(9) == "S") status = "Saved";
                if (res->getString(9) == "C") status = "Cancelled";
                if (res->getString(9) == "D") status = "Deleted";
                if (res->getString(9) == "P") status = "Pending";
                std::cout << "<td>" << status << "</td>\n";

                std::vector<FormElements::dropDownMenuItem> menuItems;
                menuItems.push_back({"location.href='/cgi-bin/registration/payment_history.cgi?key=" + Encoder::javascriptAttributeEncode(Encoder::urlEncode(userKey)) + "'", "View details", true});
                menuItems.push_back({"sendReminderEmail('" + Encoder::javascriptAttributeEncode(userKey) + "', '" + Encoder::javascriptAttributeEncode(res->getString(3)) + "', 'payment_reminder_email.cgi')", "Send Payment Reminder", needToPay && saved && (res->getString(8) == "bank")});
                menuItems.push_back({"cancelRegistration('" + Encoder::javascriptAttributeEncode(userKey) + "', '" + Encoder::javascriptAttributeEncode(res->getString(3)) + "')", "Cancel Registration", saved});
                std::cout << "<td>" << FormElements::dropDownMenu(rowId, menuItems) << "</td></tr>\n";

                rowId++;
                if (saved) {
                    adult_count += res->getInt(6);
                    child_count += res->getInt(7);

                    team_items * this_team = findTeam(&team_list, res->getString(4));
                    if (this_team) {
                        if (this_team->event_people >= 0) {
                            this_team->event_people += res->getInt(6) + res->getInt(7);
                        } else {
                            this_team->event_people = res->getInt(6) + res->getInt(7);
                        }
                    } else {
                        team_list.push_back({res->getString(4), res->getString(3), userKey, res->getInt(6) + res->getInt(7), -1, std::vector<int>(dinner_forms.size(), -1)});
                    }

                    time_t reg_time = res->getInt64(10);
                    int days_before = (jlwe_date - reg_time) / 86400;
                    if (days_before >= 0 && days_before < 365) { // ignore registrations that happen after the event
                        if (days_before >= username_count_per_day.size())
                            username_count_per_day = std::vector<int>(days_before + 1, 0);
                        username_count_per_day[days_before]++;
                        if (days_before >= people_count_per_day.size())
                            people_count_per_day = std::vector<int>(days_before + 1, 0);
                        people_count_per_day[days_before] += res->getInt(6) + res->getInt(7);
                    }
                }
            }
            delete res;
            delete stmt;
            std::cout << "<td colspan=\"3\" style=\"font-weight:bold;text-align:right;\">Total</td><td style=\"font-weight:bold;\">" << adult_count << "</td><td style=\"font-weight:bold;\">" << child_count << "</td>\n";
            std::cout << "</table>\n";

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT COUNT(*),SUM(number_adults) + SUM(number_children) FROM event_registrations WHERE status = 'S';");
            if (res->next()) {
                std::cout << "<p>Total: " << res->getInt(1) << " usernames, " << res->getInt(2) << " people</p>\n";
            }
            delete res;
            delete stmt;

            std::cout << "<h3 style=\"text-align:center\">Camping</h3>\n";
            std::cout << "<table class=\"reg_table\" align=\"center\" style=\"width: 100%;\"><tr>\n";
            std::cout << "<th>Email</th><th>Username</th><th>Phone number</th><th>Type</th><th>#People</th><th>Dates</th><th>Total cost</th><th>Paid</th><th>Payment</th><th>Status</th><th></th>\n";
            std::cout << "</tr>\n";

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT registration_id,idempotency,email_address,gc_username,phone_number,camping_type,number_people,arrive_date,leave_date,payment_type,status FROM camping;");
            while (res->next()) {
                std::string userKey = res->getString(2);
                bool saved = (res->getString(11) == "S");
                bool isEventInc = (res->getString(10) == "event");

                std::cout << "<tr" << (saved ? "" : " class=\"not_s\" style=\"background-color: #ba8866;\"") <<">\n";
                std::cout << "<td" << (isEventInc ? " style=\"font-style:italic;\"" : "") << ">" << Encoder::htmlEntityEncode(res->getString(3)) << "</td>\n";
                std::cout << "<td" << (isEventInc ? " style=\"font-style:italic;\"" : "") << ">" << Encoder::htmlEntityEncode(res->getString(4)) << "</td>\n";
                std::cout << "<td" << (isEventInc ? " style=\"font-style:italic;\"" : "") << ">" << Encoder::htmlEntityEncode(res->getString(5)) << "</td>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(6)) << "</td>\n";
                std::cout << "<td>" << res->getInt(7) << "</td>\n";
                std::cout << "<td>" << JlweUtils::numberToOrdinal(res->getInt(8)) << "&nbsp;-&nbsp;" << JlweUtils::numberToOrdinal(res->getInt(9)) << "</td>\n";
                bool needToPay = false;
                if (isEventInc) {
                    std::cout << "<td colspan=\"3\" style=\"font-style:italic;\">Included in main event</td>\n";
                } else {
                    int user_cost = PaymentUtils::getUserCost(jlwe.getMysqlCon(), userKey);
                    int payment_recived = PaymentUtils::getTotalPaymentReceived(jlwe.getMysqlCon(), userKey);
                    needToPay = ((user_cost - payment_recived) > 0);
                    bool overpaid = ((user_cost - payment_recived) < 0);
                    std::string background_color = ((needToPay && saved) ? "#FFC0C0" : ((overpaid && saved) ? "#80FFFF" : ""));
                    std::cout << "<td" << (background_color.size() > 0 ? (" style=\"background-color:" + background_color + ";\"") : "") << ">" << PaymentUtils::currencyToString(user_cost) << "</td>\n";
                    std::cout << "<td" << (background_color.size() > 0 ? (" style=\"background-color:" + background_color + ";\"") : "") << ">" << PaymentUtils::currencyToString(payment_recived) << "</td>\n";
                    std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(10)) << "</td>\n";
                }
                std::string status = "Unknown";
                if (res->getString(11) == "S") status = "Saved";
                if (res->getString(11) == "C") status = "Cancelled";
                if (res->getString(11) == "D") status = "Deleted";
                if (res->getString(11) == "P") status = "Pending";
                std::cout << "<td>" << status << "</td>\n";

                std::vector<FormElements::dropDownMenuItem> menuItems;
                menuItems.push_back({"location.href='/cgi-bin/registration/payment_history.cgi?key=" + Encoder::javascriptAttributeEncode(Encoder::urlEncode(userKey)) + "'", "View details", true});
                menuItems.push_back({"sendReminderEmail('" + Encoder::javascriptAttributeEncode(userKey) + "', '" + Encoder::javascriptAttributeEncode(res->getString(3)) + "', 'payment_reminder_email.cgi')", "Send Payment Reminder", needToPay && saved && !isEventInc && (res->getString(10) == "bank")});
                menuItems.push_back({"cancelRegistration('" + Encoder::javascriptAttributeEncode(userKey) + "', '" + Encoder::javascriptAttributeEncode(res->getString(3)) + "')", "Cancel Registration", saved && !isEventInc});
                std::cout << "<td>" << FormElements::dropDownMenu(rowId, menuItems) << "</td></tr>\n";

                rowId++;

                if (saved) {
                    team_items * this_team = findTeam(&team_list, res->getString(4));
                    if (this_team) {
                        if (this_team->camping_people >= 0) {
                            this_team->camping_people += res->getInt(7);
                        } else {
                            this_team->camping_people = res->getInt(7);
                        }
                    } else {
                        team_list.push_back({res->getString(4), res->getString(3), userKey, -1, res->getInt(7), std::vector<int>(dinner_forms.size(), -1)});
                    }
                }
            }
            delete res;
            delete stmt;
            std::cout << "</table>\n";

            // Totals
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT camping.camping_type,camping_options.display_name,COUNT(*),SUM(camping.number_people) FROM camping INNER JOIN camping_options ON camping.camping_type=camping_options.id_string WHERE status = 'S' GROUP BY camping.camping_type ORDER BY camping.camping_type;");
            while (res->next()) {
                std::cout << "<p>Total " << Encoder::htmlEntityEncode(res->getString(2)) << ": " << res->getInt(3) << " sites, " << res->getInt(4) << " people</p>\n";
            }
            delete res;
            delete stmt;


            for (unsigned int i = 0; i < dinner_forms.size(); i++) {
                std::cout << "<h3 style=\"text-align:center\">" + Encoder::htmlEntityEncode(dinner_forms.at(i).title) + "</h3>\n";
                std::cout << "<table class=\"reg_table\" align=\"center\" style=\"width: 100%;\"><tr>\n";
                std::cout << "<th>Email</th><th>Username</th><th>Phone number</th><th>#Adults</th><th>#Children</th><th>Total cost</th><th>Paid</th><th>Payment</th><th>Status</th><th></th>\n";
                std::cout << "</tr>\n";

                int dinner_adult_count = 0;
                int dinner_child_count = 0;
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT registration_id,idempotency,email_address,gc_username,phone_number,number_adults,number_children,payment_type,status FROM sat_dinner WHERE dinner_form_id = ?;");
                prep_stmt->setInt(1, dinner_forms.at(i).dinner_id);
                res = prep_stmt->executeQuery();
                while (res->next()) {
                    std::string userKey = res->getString(2);
                    bool saved = (res->getString(9) == "S");
                    bool isEventInc = (res->getString(8) == "event");

                    std::cout << "<tr" << (saved ? "" : " class=\"not_s\" style=\"background-color: #ba8866;\"") <<">\n";
                    std::cout << "<td" << (isEventInc ? " style=\"font-style:italic;\"" : "") << ">" << Encoder::htmlEntityEncode(res->getString(3)) << "</td>\n";
                    std::cout << "<td" << (isEventInc ? " style=\"font-style:italic;\"" : "") << ">" << Encoder::htmlEntityEncode(res->getString(4)) << "</td>\n";
                    std::cout << "<td" << (isEventInc ? " style=\"font-style:italic;\"" : "") << ">" << Encoder::htmlEntityEncode(res->getString(5)) << "</td>\n";
                    std::cout << "<td>" << res->getInt(6) << "</td>\n";
                    std::cout << "<td>" << res->getInt(7) << "</td>\n";
                    bool needToPay = false;
                    if (isEventInc) {
                        std::cout << "<td colspan=\"3\" style=\"font-style:italic;\">Included in main event</td>\n";
                    } else {
                        int user_cost = PaymentUtils::getUserCost(jlwe.getMysqlCon(), userKey);
                        int payment_recived = PaymentUtils::getTotalPaymentReceived(jlwe.getMysqlCon(), userKey);
                        needToPay = ((user_cost - payment_recived) > 0);
                        bool overpaid = ((user_cost - payment_recived) < 0);
                        std::string background_color = ((needToPay && saved) ? "#FFC0C0" : ((overpaid && saved) ? "#80FFFF" : ""));
                        std::cout << "<td" << (background_color.size() > 0 ? (" style=\"background-color:" + background_color + ";\"") : "") << ">" << PaymentUtils::currencyToString(user_cost) << "</td>\n";
                        std::cout << "<td" << (background_color.size() > 0 ? (" style=\"background-color:" + background_color + ";\"") : "") << ">" << PaymentUtils::currencyToString(payment_recived) << "</td>\n";
                        std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(8)) << "</td>\n";
                    }
                    std::string status = "Unknown";
                    if (res->getString(9) == "S") status = "Saved";
                    if (res->getString(9) == "C") status = "Cancelled";
                    if (res->getString(9) == "D") status = "Deleted";
                    if (res->getString(9) == "P") status = "Pending";
                    std::cout << "<td>" << status << "</td>\n";

                    std::vector<FormElements::dropDownMenuItem> menuItems;
                    menuItems.push_back({"location.href='/cgi-bin/registration/payment_history.cgi?key=" + Encoder::javascriptAttributeEncode(Encoder::urlEncode(userKey)) + "'", "View details", true});
                    menuItems.push_back({"sendReminderEmail('" + Encoder::javascriptAttributeEncode(userKey) + "', '" + Encoder::javascriptAttributeEncode(res->getString(3)) + "', 'payment_reminder_email.cgi')", "Send Payment Reminder", needToPay && saved && !isEventInc && (res->getString(8) == "bank")});
                    menuItems.push_back({"cancelRegistration('" + Encoder::javascriptAttributeEncode(userKey) + "', '" + Encoder::javascriptAttributeEncode(res->getString(3)) + "')", "Cancel Registration", saved && !isEventInc});
                    std::cout << "<td>" << FormElements::dropDownMenu(rowId, menuItems) << "</td></tr>\n";

                    rowId++;
                    if (saved) {
                        dinner_adult_count += res->getInt(6);
                        dinner_child_count += res->getInt(7);

                        team_items * this_team = findTeam(&team_list, res->getString(4));
                        if (this_team) {
                            if (this_team->dinner_people[i] >= 0) {
                                this_team->dinner_people[i] += res->getInt(6) + res->getInt(7);
                            } else {
                                this_team->dinner_people[i] = res->getInt(6) + res->getInt(7);
                            }
                        } else {
                            std::vector<int> new_dinner_people(dinner_forms.size(), -1);
                            new_dinner_people[i] = res->getInt(6) + res->getInt(7);
                            team_list.push_back({res->getString(4), res->getString(3), userKey, -1, -1, new_dinner_people});
                        }
                    }

                }
                delete res;
                delete prep_stmt;
                std::cout << "<td colspan=\"3\" style=\"font-weight:bold;text-align:right;\">Total</td><td style=\"font-weight:bold;\">" << dinner_adult_count << "</td><td style=\"font-weight:bold;\">" << dinner_child_count << "</td>\n";
                std::cout << "</table>\n";
            }


            std::sort(team_list.begin(), team_list.end(), [](const team_items &a, const team_items &b)
            {
                for (size_t c = 0; c < a.username.size() and c < b.username.size(); c++) {
                    if (std::tolower(a.username[c]) != std::tolower(b.username[c]))
                        return (std::tolower(a.username[c]) < std::tolower(b.username[c]));
                }
                return a.username.size() < b.username.size();
            });

            std::cout << "<h2 style=\"text-align:center;margin-top:50px;margin-bottom:0px;\">Who hasn't bought what</h2>\n";
            std::cout << "<table class=\"reg_table\" align=\"center\"><tr>\n";
            std::cout << "<th>Username</th><th>Event</th><th>Camping</th>";
            for (unsigned int j = 0; j < dinner_forms.size(); j++)
                std::cout << "<th>" << Encoder::htmlEntityEncode(dinner_forms.at(j).title) << "</th>";
            std::cout << "<th></th>\n";
            std::cout << "</tr>\n";
            for (unsigned int i = 0; i < team_list.size(); i++) {
                std::cout << "<tr>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(team_list.at(i).username) << "</td>\n";
                if (team_list.at(i).event_people >= 0) {
                    std::cout << "<td style=\"text-align:center;\">" << team_list.at(i).event_people << "</td>\n";
                } else {
                    std::cout << "<td style=\"text-align:center;background-color:#FFC0C0;\">-</td>\n";
                }
                if (team_list.at(i).camping_people >= 0) {
                    std::cout << "<td style=\"text-align:center;\">" << team_list.at(i).camping_people << "</td>\n";
                } else {
                    std::cout << "<td></td>\n";
                }
                for (unsigned int j = 0; j < dinner_forms.size(); j++) {
                    if (team_list.at(i).dinner_people.at(j) >= 0) {
                        std::cout << "<td style=\"text-align:center;\">" << team_list.at(i).dinner_people.at(j) << "</td>\n";
                    } else {
                        std::cout << "<td></td>\n";
                    }
                }

                std::vector<FormElements::dropDownMenuItem> menuItems;
                menuItems.push_back({"sendReminderEmail('" + Encoder::javascriptAttributeEncode(team_list.at(i).user_key) + "', '" + Encoder::javascriptAttributeEncode(team_list.at(i).email) + "', 'registration_reminder_email.cgi')", "Send Event Registration Reminder", team_list.at(i).event_people < 0});
                std::cout << "<td>" << FormElements::dropDownMenu(rowId++, menuItems) << "</td>\n";

                std::cout << "</tr>\n";
            }

            std::cout << "</table>\n";


            std::cout << FormElements::includeJavascript("/js/utils.js");
            std::cout << FormElements::includeJavascript("/js/menu.js");
            std::cout << "<script>\n";
            std::cout << "function toggleSaved(e) {\n";
            std::cout << "    var new_display = e.currentTarget.checked ? \"none\" : \"table-row\";\n";
            std::cout << "    var rows = document.getElementsByClassName(\"not_s\");\n";
            std::cout << "    for(var i = 0; i < rows.length; i++) {\n";
            std::cout << "        rows[i].style.display = new_display;\n";
            std::cout << "    }\n";
            std::cout << "}\n";
            std::cout << "toggleSaved({currentTarget:document.getElementById(\"savedToggleCB\")});\n";
            std::cout << "document.getElementById(\"savedToggleCB\").addEventListener('change', toggleSaved);\n";

            std::cout << "function sendReminderEmail(userKey, email_address, api_endpoint) {\n";
            std::cout << "    if (confirm(\"Are you sure you wish to send a reminder email to \" + email_address + \" ?\") == true) {\n";
            std::cout << "        var jsonObj = {\n";
            std::cout << "            \"key\":userKey,\n";
            std::cout << "            \"email\":email_address\n";
            std::cout << "        };\n";
            std::cout << "        postUrl(api_endpoint, JSON.stringify(jsonObj), null,\n";
            std::cout << "            function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, null, null);\n";
            std::cout << "         }, httpErrorResponseHandler);\n";
            std::cout << "    }\n";
            std::cout << "}\n";

            std::cout << "function cancelRegistration(userKey, email_address) {\n";
            std::cout << "    if (confirm(\"Are you sure you to cancel the registration for \" + email_address + \" ?\") == true) {\n";
            std::cout << "        var jsonObj = {\n";
            std::cout << "            \"key\":userKey,\n";
            std::cout << "            \"email\":email_address\n";
            std::cout << "        };\n";
            std::cout << "        postUrl('cancel.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "            function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, null, null);\n";
            std::cout << "         }, httpErrorResponseHandler);\n";
            std::cout << "    }\n";
            std::cout << "}\n";

            std::cout << "</script>\n";


            std::cout << "<h2 style=\"text-align:center;margin-top:50px;margin-bottom:0px;\">Registration Stats</h2>\n";
            std::cout << "<canvas id=\"users_per_day_plot\"></canvas>\n";

            std::cout << "<script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>\n";
            std::cout << "<script>\n";

            int current_days_to_event = (jlwe_date - time(nullptr)) / 86400;
            std::cout << "var usernames_per_day = [0";
            int total = 0;
            for (unsigned int i = username_count_per_day.size(); i > 0; i--) {
                total += username_count_per_day.at(i - 1);
                if (static_cast<int>(i) - 1 > current_days_to_event) {
                    std::cout << "," << total;
                } else {
                    std::cout << ",null";
                }
            }
            std::cout << "];\n";
            std::cout << "var people_per_day = [0";
            total = 0;
            for (unsigned int i = people_count_per_day.size(); i > 0; i--) {
                total += people_count_per_day.at(i - 1);
                if (static_cast<int>(i) - 1 > current_days_to_event) {
                    std::cout << "," << total;
                } else {
                    std::cout << ",null";
                }
            }
            std::cout << "];\n";

            std::cout << "(async function main () {\n";

            std::cout << "var current_year = " << JlweUtils::getCurrentYearString() << ";\n";
            std::cout << "var usernames_per_day_plot_data = [];\n";
            std::cout << "var start_day = 1 - usernames_per_day.length;\n";
            std::cout << "for (var i = 0; i < usernames_per_day.length; i++)\n";
            std::cout << "  usernames_per_day_plot_data.push({x: start_day + i, y: usernames_per_day[i]});\n";
            std::cout << "var current_dataset = {label: current_year.toString(), data: usernames_per_day_plot_data};\n";

            std::cout << "try {\n";
            std::cout << "  var past_year_data = [];\n";
            std::cout << "  const response = await fetch(\"/reg_stats.json\");\n";
            std::cout << "  if (response.ok)\n";
            std::cout << "    past_year_data = await response.json();\n";
            std::cout << "} catch (error) {}\n";

            std::cout << "var past_datasets = past_year_data.map(item => {\n";
            std::cout << "  var start_day = 1 - item.usernames.length;\n";
            std::cout << "  var usernames_per_day_plot_data = [];\n";
            std::cout << "  for (var i = 0; i < item.usernames.length; i++)\n";
            std::cout << "    usernames_per_day_plot_data.push({x: start_day + i, y: item.usernames[i]});\n";
            std::cout << "  return {label: item.year.toString(), data: usernames_per_day_plot_data};\n";
            std::cout << "});\n";

            std::cout << "var all_datasets = [current_dataset].concat(past_datasets);\n";

            std::cout << "new Chart(document.getElementById('users_per_day_plot'), {\n";
            std::cout << "  type: 'line',\n";
            std::cout << "  data: {\n";
            std::cout << "    datasets: all_datasets\n";
            std::cout << "  },\n";
            std::cout << "  options: {\n";
            std::cout << "    plugins: {\n";
            std::cout << "      title: {display: true, text: 'Total usernames registered over time'}\n";
            std::cout << "    },\n";
            std::cout << "    scales: {\n";
            std::cout << "      x: {\n";
            std::cout << "        type: 'linear',\n";
            std::cout << "        title: {display: true, text: 'Days before event'}\n";
            std::cout << "      },\n";
            std::cout << "      y: {\n";
            std::cout << "        type: 'linear',\n";
            std::cout << "        title: {display: true, text: 'No. of usernames'}\n";
            std::cout << "      }\n";
            std::cout << "    }\n";
            std::cout << "  }\n";
            std::cout << "});\n";

            std::cout << "})();\n";

            std::cout << "</script>\n";


        } else {
            if (jlwe.isLoggedIn()) {
                std::cout << "<p>You don't have permission to view this area.</p>";
            } else {
                std::cout << "<p>You need to be logged in to view this area.</p>";
            }
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
