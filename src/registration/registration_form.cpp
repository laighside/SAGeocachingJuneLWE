/**
  @file    registration_form.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the script at /cgi-bin/registration/registration_form.cgi
  This creates the registration form that customers fill in, including the camping and dinner order forms
  Apache mod_rewrite is used to direct the following URLs to this script:
   - https://jlwe.org/register
   - https://jlwe.org/camping
   - https://jlwe.org/dinner

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <cmath>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"

void outputEventTab(const std::string &event_registration_html, time_t camping_cutoff, time_t dinner_cutoff, time_t time_now) {
    std::cout << "<div class=\"formTab\" id=\"eventTab\">\n";

    //std::cout << "<p>This form allows to register for the event, book camping and order Saturday night's dinner in one transaction. However, camping and dinner can still be booked separately at a later date if you are unsure of your plans at this stage. <a href=\"/camping\">Click here to book <span style=\"font-weight:bold;\">camping</span> separately.</a> <a href=\"/dinner\">Click here to book <span style=\"font-weight:bold;\">dinner</span> separately.</a><br />\n";
    std::cout << "<p>This form allows to register for the event and book camping in one transaction. However, camping can still be booked separately at a later date if you are unsure of your plans at this stage. <a href=\"/camping\">Click here to book <span style=\"font-weight:bold;\">camping</span> separately.</a><br />\n";
    std::cout << "Camping must be booked by " << FormElements::timeToDateString(camping_cutoff) << " at the latest.</p>\n";
    //std::cout << "Dinner bookings close on " << FormElements::timeToDateString(dinner_cutoff) << ".</p>\n";

    if (camping_cutoff < time_now)
        std::cout << "<p style=\"color:red;font-weight:bold;\">Camping bookings have now closed.</p>\n";
    //if (dinner_cutoff < time_now)
    //    std::cout << "<p style=\"color:red;font-weight:bold;\">Dinner bookings have now closed.</p>\n";

    std::cout << "<p>The cost is $<span id=\"display_price_event_adult\"></span> per player. 10 years and under can participate for free.<br/>\n";
    //std::cout << "<p>The cost is $<span id=\"display_price_event_adult\"></span> per adult and $<span id=\"display_price_event_child\"></span> per child (children under 5 years are free).<br/>\n";
    std::cout << "&nbsp;- This includes participation in all games and a bacon and egg breakfast on Sunday morning<br/>\n";
    std::cout << "On-site camping is an extra fee.</p>\n";

    std::cout << event_registration_html << "\n";

    std::cout << FormElements::emailUsernamePhoneBoxes(true);
    std::cout << FormElements::numberInput("number_adults", "Number of Players: (over 10 years)", 1, 0, 20);
    std::cout << FormElements::numberInput("number_children", "Number of Children: (10 and under years)", 0, 0, 20);
    std::cout << FormElements::textInput("real_names_adults", "text", "Name(s) of all the adults in your team:", "Names...");
    std::cout << FormElements::textInput("real_names_children", "text", "Name(s) of all the children in your team:", "Names...");

    std::cout << FormElements::radioButtons("past_jlwe", "Have you attended a June LWE event before?", {{"past_jlwe_yes", "Yes", "yes", "setRadioClass(this.name, '');", false, false}, {"past_jlwe_no", "No", "no", "setRadioClass(this.name, '');", false, false}});
    //std::cout << FormElements::radioButtons("lanyard", "Do you have a June LWE lanyard? (you should if you were at the 2018 or 2019 events)", {{"lanyard_yes", "Yes (please bring it with you to this year's event)", "yes", "setRadioClass(this.name, '');", false, false}, {"lanyard_no", "No", "no", "setRadioClass(this.name, '');", false, false}});

    std::cout << FormElements::radioButtons("camping", "Would you like to book a camping site?", {{"camping_yes", "Yes, I wish to reserve a camping site", "yes", "setRadioClass(this.name, '');", false, false}, {"camping_no", "No, I will be saying offsite or sharing with someone else", "no", "setRadioClass(this.name, '');", false, false}});
    //std::cout << FormElements::radioButtons("dinner", "Will you be attending the dinner event on Saturday night?", {{"dinner_yes", "Yes, I would like to order a meal", "yes", "setRadioClass(this.name, '');", false, false}, {"dinner_no", "No/Unsure", "no", "setRadioClass(this.name, '');", false, false}});

    std::cout << "</div>\n";
}

void outputCampingTab(const std::string &camping_registration_html, bool camping_only, time_t camping_cutoff, time_t time_now, int saturday_date) {
    std::cout << "<div class=\"formTab\" id=\"campingTab\">\n";

    std::cout << "<p><span style=\"font-weight:bold;\">Camping fees:</span><br/>\n";
    std::cout << "<span id=\"display_price_camping\"></span></p>\n";
    std::cout << "<p id=\"remaining_camping_label\" style=\"font-weight:bold;\"></p>\n";

    std::cout << "<p>Camping must be booked by " << FormElements::timeToDateString(camping_cutoff) << " at the latest.</p>\n";
    if (camping_cutoff < time_now)
        std::cout << "<p style=\"color:red;font-weight:bold;\">Camping bookings have now closed.</p>\n";

    std::cout << camping_registration_html << "\n";

    if (camping_only)
        std::cout << FormElements::emailUsernamePhoneBoxes(false);

    std::cout << FormElements::numberInput("number_people_camping", "Number of people sharing your campsite:", 0, 0, 20);
    std::cout << FormElements::radioButtons("camping_type", "What camping site would you like?", {{"camping_powered", "Powered Site", "powered", "setRadioClass(this.name, '');", false, false}, {"camping_unpowered", "Unpowered Site", "unpowered", "setRadioClass(this.name, '');", false, false}});

    std::vector<FormElements::dropDownOption> arrive_dates;
    arrive_dates.push_back({std::to_string(saturday_date - 1), "Friday " + JlweUtils::numberToOrdinal(saturday_date - 1), true});
    arrive_dates.push_back({std::to_string(saturday_date), "Saturday " + JlweUtils::numberToOrdinal(saturday_date), false});
    arrive_dates.push_back({std::to_string(saturday_date + 1), "Sunday " + JlweUtils::numberToOrdinal(saturday_date + 1), false});
    std::cout << FormElements::dropDownList("camping_arrive", "What day will you be arriving?", arrive_dates);

    std::vector<FormElements::dropDownOption> leave_dates;
    leave_dates.push_back({std::to_string(saturday_date), "Saturday " + JlweUtils::numberToOrdinal(saturday_date), false});
    leave_dates.push_back({std::to_string(saturday_date + 1), "Sunday " + JlweUtils::numberToOrdinal(saturday_date + 1), false});
    leave_dates.push_back({std::to_string(saturday_date + 2), "Monday " + JlweUtils::numberToOrdinal(saturday_date + 2), true});
    std::cout << FormElements::dropDownList("camping_leave", "What day will you be leaving?", leave_dates);

    std::cout << FormElements::textArea("camping_comments", "Comments:");

    std::cout << "</div>\n";
}

void outputDinnerTab(const std::string &dinner_registration_html, bool dinner_only, time_t dinner_cutoff, time_t time_now) {
    std::cout << "<div class=\"formTab\" id=\"dinnerTab\">\n";

    std::cout << "<p>Adult dinners are $<span id=\"display_price_dinner_adult\"></span> per meal.<br/>\n";
    std::cout << "Child dinners are $<span id=\"display_price_dinner_child\"></span> per meal. (up to 12 years old)</p>\n";

    std::cout << "<p>Dinner bookings close on " << FormElements::timeToDateString(dinner_cutoff) << ".</p>\n";
    if (dinner_cutoff < time_now)
        std::cout << "<p style=\"color:red;font-weight:bold;\">Dinner bookings have now closed.</p>\n";

    std::cout << dinner_registration_html << "\n";

    if (dinner_only)
        std::cout << FormElements::emailUsernamePhoneBoxes(false);

    std::cout << "<p style=\"font-weight:bold;\">Adult dinner orders:</p>\n";
    std::cout << FormElements::numberInput("dinner_number_adults_op1", "How many Chicken Schnitzels:", 0, 0, 20);
    std::cout << FormElements::numberInput("dinner_number_adults_op2", "How many Beef Schnitzels:", 0, 0, 20);
    std::cout << FormElements::numberInput("dinner_number_adults_op3", "How many Beer Battered fish:", 0, 0, 20);
    std::cout << "<p><span style=\"font-weight:bold;\">Kids dinner orders:</span> (12 years and under)</p>\n";
    std::cout << FormElements::numberInput("dinner_number_children_op1", "How many 1/2 Chicken Schnitzels:", 0, 0, 20);
    std::cout << FormElements::numberInput("dinner_number_children_op2", "How many 1/2 Beef Schnitzels:", 0, 0, 20);
    std::cout << FormElements::numberInput("dinner_number_children_op3", "How many 6 Nugget meals:", 0, 0, 20);
    std::cout << "<p><span style=\"font-weight:bold;\">Dessert orders:</span></p>\n";
    std::cout << FormElements::numberInput("dinner_number_dessert_op1", "How many chocolate mousses:", 0, 0, 20);
    std::cout << FormElements::numberInput("dinner_number_dessert_op2", "How many Eton Messes:", 0, 0, 20);
    std::cout << FormElements::numberInput("dinner_number_dessert_op3", "How many Fruit salads:", 0, 0, 20);

    std::cout << FormElements::textArea("dinner_comments", "Comments/Questions:");

    std::cout << "</div>\n";
}

void outputSummaryTab() {
    std::cout << "<div class=\"formTab\" id=\"summaryTab\">\n";

    std::cout << "<h2>Summary:</h2>\n";
    std::cout << "<div style=\"font-size:16px;\">\n";
    std::cout << "<p>Username: <span id=\"summary_username\" style=\"font-weight:bold;\"></span></p>\n";
    std::cout << "<p>Email: <span id=\"summary_email\" style=\"font-weight:bold;\"></span></p>\n";
    std::cout << "<p>Phone: <span id=\"summary_phone\" style=\"font-weight:bold;\"></span></p>\n";
    std::cout << "<table id=\"cost_table\">\n";
    std::cout << "<tr><th colspan=\"2\">Costs</th></tr>\n";
    std::cout << "<tr id=\"event_row\">\n";
    std::cout << "<td id=\"summary_event_desc\">Event</td>\n";
    std::cout << "<td id=\"summary_event_cost\" class=\"currency_cell\">$0.00</td>\n";
    std::cout << "</tr>\n";
    std::cout << "<tr id=\"camping_row\">\n";
    std::cout << "<td id=\"summary_camping_desc\">Camping</td>\n";
    std::cout << "<td id=\"summary_camping_cost\" class=\"currency_cell\">$0.00</td>\n";
    std::cout << "</tr>\n";
    std::cout << "<tr id=\"dinner_row\">\n";
    std::cout << "<td id=\"summary_dinner_desc\">Saturday dinner</td>\n";
    std::cout << "<td id=\"summary_dinner_cost\" class=\"currency_cell\">$0.00</td>\n";
    std::cout << "</tr>\n";
    std::cout << "<tr id=\"card_surcharge_row\">\n";
    std::cout << "<td id=\"summary_card_surcharge_desc\">Card processing fee</td>\n";
    std::cout << "<td id=\"summary_card_surcharge_cost\" class=\"currency_cell\">$0.00</td>\n";
    std::cout << "</tr>\n";
    std::cout << "<tr style=\"font-weight:bold;\">\n";
    std::cout << "<td style=\"border-top-width:1px;border-bottom-width:1px;\">Total</td>\n";
    std::cout << "<td id=\"summary_total_cost\" class=\"currency_cell\" style=\"border-top-width:1px;border-bottom-width:1px;\">$0.00</td>\n";
    std::cout << "</tr>\n";
    std::cout << "</table>\n";

    std::cout << FormElements::radioButtons("payment", "Select payment method:", {{"payment_card", "Credit/debit card", "card", "setRadioClass(this.name, '');", false, false}, {"payment_bank", "Bank transfer", "bank", "setRadioClass(this.name, '');", false, false}}); //, {"payment_cash", "Cash at the event", "cash", "setRadioClass(this.name, '');", false, false}

    std::cout << "<div id=\"cash_payment_note\"><p style=\"margin-bottom: 0px;\">We prefer bank or card payment. However payment by cash at the event is an option where bank and card are not suitable. Please contact us to discuss at contact@jlwe.org<br/>\n";
    std::cout << "<span style=\"font-weight:bold;\">Please note the following conditions apply if you choose to pay with cash:</span></p>\n";
    std::cout << "<ul style=\"font-weight:bold;margin-top: 0px;margin-bottom: 0px;\"><li>You must bring the exact amount. No change will be given.</li>\n";
    std::cout << "<li>You may not receive a registration bag. Supplies are limited.</li></ul>\n";
    std::cout << "</div>\n";

    std::cout << "<p style=\"line-height:2em;margin-top:0px;\">\n";
    std::cout << FormElements::radioButton("payment_cash", "payment", "Cash at the event", "cash", "setRadioClass(this.name, '');", false, false);
    std::cout << "</p>\n";

    std::cout << "<p id=\"card_surcharge_note\" style=\"font-size:16px;font-weight:bold;\">Please note that there is a surcharge of 1.75% + $0.30 on card payments to cover the card payment processing fee.</p>\n";
    std::cout << "<p id=\"payment_note\" style=\"font-size:16px;font-weight:bold;\">Note that camping is not reserved until payment is received. Hence paying by cash at the event is not possible for camping reservations.<br/>\n";
    //std::cout << "<p id=\"payment_note\" style=\"font-size:16px;font-weight:bold;\">Note that dinner and camping is not reserved until payment is received. Hence paying by cash at the event is not possible for dinner and camping reservations.<br/>\n";
    std::cout << "Camping fees are non-refundable if you don't attend the event.</p>\n";
    //std::cout << "Dinner and camping fees are non-refundable if you don't attend the event.</p>\n";

    std::cout << "</div>\n";

    std::cout << "</div>\n";
}

int main () {
    try {
        // work out if the user has requested the event, camping or dinner form
        std::string page_request = CgiEnvironment::getRequestUri();
        bool camping_form = false;
        bool dinner_form = false;
        if (page_request.substr(0, 8) == "/camping")
            camping_form = true;
        if (page_request.substr(0, 7) == "/dinner")
            dinner_form = true;

        JlweCore jlwe;

        sql::Statement *stmt;
        sql::ResultSet *res;

        HtmlTemplate html(true);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Event Registrations", false))
            return 0;

        time_t time_now = time(nullptr);
        time_t registration_open_date = 0;
        try {
            registration_open_date = std::stoll(jlwe.getGlobalVar("registration_open_date"));
        } catch (...) {}

        if (registration_open_date == 0) {
            std::cout << "<p style=\"color:red;text-align:center;font-weight:bold;\">Error: registration_open_date has not been set. Please contact us on " << std::string(jlwe.config.at("adminEmail")) << "</p>\n";
            html.outputFooter();
            return 0;
        }

        // check if registration is currently open
        if (registration_open_date > time_now) {
            if (jlwe.getPermissionValue("perm_registrations")) { //if logged in
                std::cout << "<div class=\"note\"><p><span style=\"font-weight:bold;\">The registration form is currently closed.</span> This is a preview available to admins. It will open to everyone on <span style=\"font-weight:bold;\">" << FormElements::timeToDateString(registration_open_date) << "</span></p></div>\n";
            } else {
                std::cout << "<p>The registration form will be live on " << FormElements::timeToDateString(registration_open_date) << "</p>";
                html.outputFooter();
                return 0;
            }
        }


        std::cout << FormElements::includeJavascript("https://js.stripe.com/v3/");
        std::cout << FormElements::includeJavascript("/cgi-bin/stripe_keys.cgi");
        std::cout << FormElements::includeJavascript("/js/utils.js");
        std::cout << FormElements::includeJavascript("/js/form_tools.js");
        std::cout << FormElements::includeJavascript("/js/registration_form.js");
        std::cout << FormElements::includeJavascript("/js/uuid.js");

        if (jlwe.config.at("stripe").value("testMode", false)) {
            std::cout << "<div class=\"note\"><p><span style=\"font-weight:bold;\">Stripe test mode is enabled.</span> You will not be charged. See <a href=\"https://stripe.com/docs/testing\">https://stripe.com/docs/testing</a></p></div>\n";
        }

        std::cout << "<div id=\"loader\" class=\"loader\" style=\"display:none;\"><div></div></div>\n";

        if (camping_form) {
            std::cout << "<h1>Camping Registration</h1>\n";
            std::cout << "<p style=\"font-weight:bold;\">This form is for booking camping only. If you have already booked camping as part of your event registration, you do not need to fill out this form.</p>\n";
        } else if (dinner_form) {
            std::cout << "<p>Dinner form is inactive</p>";
            html.outputFooter();
            return 0;

            std::cout << "<h1>Saturday Dinner Registration</h1>\n";
            std::cout << "<p style=\"font-weight:bold;\">This form is for dinner orders only. If you have already ordered dinner as part of your event registration, you do not need to fill out this form.</p>\n";
        } else {
            std::cout << "<h1>Event Registration</h1>\n";
        }

        std::cout << "<noscript><p style=\"color:red;text-align:center;font-weight:bold;\">You need to have JavaScript enabled.</p></noscript>\n";

        // get dates
        time_t camping_cutoff = 0;
        try {
            camping_cutoff = std::stoll(jlwe.getGlobalVar("camping_cutoff_date"));
        } catch (...) {}
        time_t dinner_cutoff = 0;
        try {
            dinner_cutoff = std::stoll(jlwe.getGlobalVar("dinner_cutoff_date"));
        } catch (...) {}
        time_t jlwe_date = 0;
        try {
            jlwe_date = std::stoll(jlwe.getGlobalVar("jlwe_date"));
        } catch (...) {}

        // Work out the date of the Saturday of the JLWE
        // This won't work if the weekend spans across two months. This should never happen since the SA JLWE is defined as the 2nd Monday in June.
        struct tm * jlwe_date_tm = gmtime(&jlwe_date);
        int saturday_date = 0;
        if (jlwe_date_tm->tm_wday == 5) { // it's a friday
            saturday_date = jlwe_date_tm->tm_mday + 1;
        } else if (jlwe_date_tm->tm_wday == 6) { // it's a saturday
            saturday_date = jlwe_date_tm->tm_mday;
        } else if (jlwe_date_tm->tm_wday == 0) { // it's a sunday
            saturday_date = jlwe_date_tm->tm_mday - 1;
        } else if (jlwe_date_tm->tm_wday == 1) { // it's a monday
            saturday_date = jlwe_date_tm->tm_mday - 2;
        } else { // jlwe_date is not a weekend
            std::cout << "<p style=\"color:red;text-align:center;font-weight:bold;\">Error: jlwe_date is not set to a weekend. Please contact us on " << std::string(jlwe.config.at("adminEmail")) << "</p>\n";
        }


        std::string event_registration_html, camping_registration_html, dinner_registration_html;
        stmt = jlwe.getMysqlCon()->createStatement();
        res = stmt->executeQuery("SELECT html FROM webpages WHERE path = '*event_registration';");
        while (res->next()){
            event_registration_html = res->getString(1);
        }
        delete res;
        delete stmt;
        stmt = jlwe.getMysqlCon()->createStatement();
        res = stmt->executeQuery("SELECT html FROM webpages WHERE path = '*camping_registration';");
        while (res->next()){
            camping_registration_html = res->getString(1);
        }
        delete res;
        delete stmt;
        stmt = jlwe.getMysqlCon()->createStatement();
        res = stmt->executeQuery("SELECT html FROM webpages WHERE path = '*dinner_registration';");
        while (res->next()){
            dinner_registration_html = res->getString(1);
        }
        delete res;
        delete stmt;


        std::cout << "<form id=\"regForm\">\n";

        if (!camping_form && !dinner_form)
            outputEventTab(event_registration_html, camping_cutoff, dinner_cutoff, time_now);
        if (!dinner_form)
            outputCampingTab(camping_registration_html, camping_form, camping_cutoff, time_now, saturday_date);
        //if (!camping_form)
        //    outputDinnerTab(dinner_registration_html, dinner_form, dinner_cutoff, time_now);
        outputSummaryTab();

        std::cout << FormElements::formMessages();
        std::cout << FormElements::formButtons();

        std::cout << "</form>\n";

        std::cout << "<script type=\"text/javascript\">\n";
        std::cout << "var event_form = " << ((camping_form == false && dinner_form == false) ? "true" : "false") << ";\n";

        std::cout << "var camping = " << (camping_form ? "true" : "false") << ";\n";
        std::cout << "var dinner = " << (dinner_form ? "true" : "false") << ";\n";
        if (camping_form || dinner_form) {
            std::cout << "var summary_page = 1;\n";
        } else {
            std::cout << "var summary_page = 2;\n"; // index of the summary of costs page (3 with dinner, 2 dinner disabled)
        }

        if (!camping_form && !dinner_form)
            std::cout << "var formPostURL = stripeSessionURL + '?type=event';\n";
        if (camping_form)
            std::cout << "var formPostURL = stripeSessionURL + '?type=camping_only';\n";
        if (dinner_form)
            std::cout << "var formPostURL = stripeSessionURL + '?type=dinner_only';\n";

        std::cout << "showTab(currentTab); // Display the current tab\n";
        std::cout << "loadRegForm();\n";
        std::cout << "</script>\n";

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
