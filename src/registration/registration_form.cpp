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
#include "../core/Encoder.h"
#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/KeyValueParser.h"

struct dinner_form {
    int dinner_id;
    std::string title;
    time_t order_close_time;
    std::string html_path;
};

void outputCampingCutoffTime(time_t camping_cutoff, time_t time_now) {
    std::cout << "<p>Camping must be booked by <span class=\"date_only\" data-value=\"" + std::to_string(camping_cutoff) + "\">" + FormElements::timeToDateString(camping_cutoff) + "</span> at the latest.\n";
    if (camping_cutoff < time_now)
        std::cout << " - <span style=\"color:red;font-weight:bold;\">Bookings have now closed.</span>\n";
    std::cout << "</p>\n";
}

void outputDinnerCutoffTime(time_t dinner_cutoff, time_t time_now, std::string title) {
    std::cout << "<p>Orders for " + Encoder::htmlEntityEncode(title) + " close on <span class=\"date_only\" data-value=\"" + std::to_string(dinner_cutoff) + "\">" + FormElements::timeToDateString(dinner_cutoff) + "</span>";
    if (dinner_cutoff < time_now)
        std::cout << " - <span style=\"color:red;font-weight:bold;\">Orders have now closed.</span>\n";
    std::cout << "</p>\n";
}

void outputEventTab(const std::string &event_registration_html, time_t camping_cutoff, time_t time_now, std::vector<dinner_form> * dinner_forms) { // bool dinner_form_enabled
    std::cout << "<div class=\"formTab\" id=\"eventTab\">\n";

    if (dinner_forms->size()) {
        std::cout << "<p>This form allows to register for the event, book camping and order dinner in one transaction. However, camping and dinner can still be booked separately at a later date if you are unsure of your plans at this stage.</p>\n";
    } else {
        std::cout << "<p>This form allows to register for the event and book camping in one transaction. However, camping can still be booked separately at a later date if you are unsure of your plans at this stage.</p>\n";
    }

    outputCampingCutoffTime(camping_cutoff, time_now);
    for (unsigned int i = 0; i < dinner_forms->size(); i++)
        if (dinner_forms->at(i).order_close_time)
            outputDinnerCutoffTime(dinner_forms->at(i).order_close_time, time_now, dinner_forms->at(i).title);

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

    std::cout << FormElements::radioButtons("past_jlwe", "Have you attended a June LWE event before?", {{"past_jlwe_yes", "Yes", "yes", "setRadioClass(this.name, '');", false, false, ""}, {"past_jlwe_no", "No", "no", "setRadioClass(this.name, '');", false, false, ""}});
    //std::cout << FormElements::radioButtons("lanyard", "Do you have a June LWE lanyard? (you should if you were at the 2018 or 2019 events)", {{"lanyard_yes", "Yes (please bring it with you to this year's event)", "yes", "setRadioClass(this.name, '');", false, false}, {"lanyard_no", "No", "no", "setRadioClass(this.name, '');", false, false}});

    // camping
    bool camping_closed = (camping_cutoff < time_now);
    std::cout << FormElements::radioButtons("camping", "Would you like to book a camping site?", {{"camping_yes", "Yes, I wish to reserve a camping site" + std::string(camping_closed ? " <span style=\"font-weight:bold;color:black;\">(bookings have closed)</span>" : ""), "yes", "setRadioClass(this.name, '');", false, camping_closed, ""}, {"camping_no", "No, I will be saying offsite or sharing with someone else", "no", "setRadioClass(this.name, '');", false, false, ""}});

    // dinner
    for (unsigned int i = 0; i < dinner_forms->size(); i++) {
        std::string id_str = std::to_string(dinner_forms->at(i).dinner_id);
        bool ordering_closed = (dinner_forms->at(i).order_close_time) && (dinner_forms->at(i).order_close_time < time_now);
        std::cout << FormElements::radioButtons("dinner_" + id_str, Encoder::htmlEntityEncode("Will you be attending the " + dinner_forms->at(i).title + "?"), {{"dinner_yes_" + id_str, "Yes, I would like to order a meal" + std::string(ordering_closed ? " <span style=\"font-weight:bold;color:black;\">(orders have closed)</span>" : ""), "yes", "setRadioClass(this.name, '');", false, ordering_closed, ""}, {"dinner_no_" + id_str, "No/Unsure", "no", "setRadioClass(this.name, '');", false, false, ""}});
    }

    std::cout << "</div>\n";
}

void outputCampingTab(const std::string &camping_registration_html, bool camping_only, time_t camping_cutoff, time_t time_now, int saturday_date, JlweCore * jlwe) {
    std::vector<FormElements::radiobutton> camping_options;
    sql::Statement *stmt = jlwe->getMysqlCon()->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT id_string,display_name,display_comment FROM camping_options WHERE active != 0;");
    while (res->next()){
        std::string id_string = res->getString(1);
        std::string comment = "";
        if (!res->isNull(3))
            comment = res->getString(3);
        camping_options.push_back({"camping_" + id_string, Encoder::htmlEntityEncode(res->getString(2)), id_string, "setRadioClass(this.name, '');", false, false, Encoder::htmlEntityEncode(comment)});
    }
    delete res;
    delete stmt;

    std::cout << "<div class=\"formTab\" id=\"campingTab\">\n";

    std::cout << "<p><span style=\"font-weight:bold;\">Camping fees:</span><br/>\n";
    std::cout << "<span id=\"display_price_camping\"></span></p>\n";
    std::cout << "<p id=\"remaining_camping_label\" style=\"font-weight:bold;\"></p>\n";

    outputCampingCutoffTime(camping_cutoff, time_now);

    std::cout << camping_registration_html << "\n";

    if (camping_only)
        std::cout << FormElements::emailUsernamePhoneBoxes(false);

    std::cout << FormElements::numberInput("number_people_camping", "Number of people (over 10 yrs old) sharing your campsite:", 0, 0, 5);
    std::cout << FormElements::radioButtons("camping_type", "What camping site would you like?", camping_options);

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

    std::cout << FormElements::textArea("camping_comments", "Comments: (let us know if you're bringing a caravan, camper-trailer, tent/swag, etc.)");

    std::cout << "</div>\n";
}

void outputDinnerTab(const std::string &dinner_registration_html, bool dinner_only, dinner_form * form, time_t time_now) {
    std::cout << "<div class=\"formTab\" id=\"dinnerTab_" + std::to_string(form->dinner_id) + "\">\n";

    std::cout << "<h2>" << Encoder::htmlEntityEncode(form->title) << "</h2>\n";

    if (form->order_close_time)
        outputDinnerCutoffTime(form->order_close_time, time_now, form->title);

    std::cout << dinner_registration_html << "\n";

    if (dinner_only)
        std::cout << FormElements::emailUsernamePhoneBoxes(false);

    std::cout << "<div id=\"dinner_items_block_"  + std::to_string(form->dinner_id) + "\" style=\"margin-top:30px;\"></div>\n";

    std::cout << FormElements::textArea("dinner_comments_" + std::to_string(form->dinner_id), "Comments/Questions:");

    std::cout << "<script type=\"text/javascript\">\n";
    std::cout << "loadDinnerItems(\"/cgi-bin/registration/get_form_content.cgi?dinner_id="  + std::to_string(form->dinner_id) + "\", \"dinner_items_block_"  + std::to_string(form->dinner_id) + "\", "  + std::to_string(form->dinner_id) + ");\n";
    std::cout << "</script>\n";

    std::cout << "</div>\n";
}

void outputSummaryTab(std::vector<dinner_form> * dinner_forms) {
    std::cout << "<div class=\"formTab\" id=\"summaryTab\">\n";

    std::cout << "<h2>Summary:</h2>\n";
    std::cout << "<div style=\"font-size:16px;\">\n";
    std::cout << "<p>Username: <span id=\"summary_username\" style=\"font-weight:bold;\"></span></p>\n";
    std::cout << "<p>Email: <span id=\"summary_email\" style=\"font-weight:bold;\"></span></p>\n";
    std::cout << "<p>Phone: <span id=\"summary_phone\" style=\"font-weight:bold;\"></span></p>\n";
    std::cout << "<table id=\"cost_table\">\n";
    std::cout << "<tr><th colspan=\"2\">Costs</th></tr>\n";
    std::cout << "<tr id=\"event_row\">\n";
    std::cout << "<td id=\"summary_event_desc\">Event registration<br />\n";
    std::cout << "<span class=\"subtext\" id=\"summary_event_subtext\"></span></td>\n";
    std::cout << "<td id=\"summary_event_cost\" class=\"currency_cell\">$0.00</td>\n";
    std::cout << "</tr>\n";
    std::cout << "<tr id=\"camping_row\">\n";
    std::cout << "<td id=\"summary_camping_desc\">Camping<br />\n";
    std::cout << "<span class=\"subtext\" id=\"summary_camping_subtext\"></span></td>\n";
    std::cout << "<td id=\"summary_camping_cost\" class=\"currency_cell\">$0.00</td>\n";
    std::cout << "</tr>\n";
    for (unsigned int i = 0; i < dinner_forms->size(); i++) {
        std::string id_str = std::to_string(dinner_forms->at(i).dinner_id);
        std::cout << "<tr id=\"dinner_row_" + id_str + "\">\n";
        std::cout << "<td id=\"summary_dinner_desc_" + id_str + "\">" + Encoder::htmlEntityEncode(dinner_forms->at(i).title) + "<br />\n";
        std::cout << "<span class=\"subtext\" id=\"summary_dinner_subtext_" + id_str + "\"></span></td>\n";
        std::cout << "<td id=\"summary_dinner_cost_" + id_str + "\" class=\"currency_cell\">$0.00</td>\n";
        std::cout << "</tr>\n";
    }
    std::cout << "<tr id=\"card_surcharge_row\">\n";
    std::cout << "<td id=\"summary_card_surcharge_desc\">Card processing fee</td>\n";
    std::cout << "<td id=\"summary_card_surcharge_cost\" class=\"currency_cell\">$0.00</td>\n";
    std::cout << "</tr>\n";
    std::cout << "<tr style=\"font-weight:bold;\">\n";
    std::cout << "<td style=\"border-top-width:1px;border-bottom-width:1px;\">Total</td>\n";
    std::cout << "<td id=\"summary_total_cost\" class=\"currency_cell\" style=\"border-top-width:1px;border-bottom-width:1px;\">$0.00</td>\n";
    std::cout << "</tr>\n";
    std::cout << "</table>\n";

    std::cout << FormElements::radioButtons("payment", "Select payment method:", {{"payment_card", "Credit/debit card", "card", "setRadioClass(this.name, '');", false, false, ""}, {"payment_bank", "Bank transfer", "bank", "setRadioClass(this.name, '');", false, false, ""}}); //, {"payment_cash", "Cash at the event", "cash", "setRadioClass(this.name, '');", false, false}

    std::cout << "<div id=\"cash_payment_note\"><p style=\"margin-bottom: 0px;\">We prefer bank or card payment. However payment at the event is an option where bank and card are not suitable. Please contact us to discuss at contact@jlwe.org<br/>\n";
    std::cout << "<span style=\"font-weight:bold;\">Please note the following conditions apply if you choose to pay at the event:</span></p>\n";
    std::cout << "<ul style=\"font-weight:bold;margin-top: 0px;margin-bottom: 0px;\"><li>You may not receive a registration bag. Supplies are limited.</li></ul>\n";
    std::cout << "</div>\n";

    std::cout << "<p style=\"line-height:2em;margin-top:0px;\">\n";
    std::cout << FormElements::radioButton("payment_cash", "payment", "Pay at the event (eftpos/paywave)", "cash", "setRadioClass(this.name, '');", false, false);
    std::cout << "</p>\n";

    std::cout << "<p id=\"card_surcharge_note\" style=\"font-size:16px;font-weight:bold;\">Please note that there is a surcharge of 1.75% + $0.30 on card payments to cover the card payment processing fee.</p>\n";
    //std::cout << "<p id=\"payment_note\" style=\"font-size:16px;font-weight:bold;\">Note that camping is not reserved until payment is received. Hence paying upon arrival at the event is not possible for camping reservations.<br/>\n";
    std::cout << "<p id=\"payment_note\" style=\"font-size:16px;font-weight:bold;\">Note that dinner and camping is not reserved until payment is received. Hence paying upon arrival at the event is not possible for dinner and camping reservations.<br/>\n";
    //std::cout << "Camping fees are non-refundable if you don't attend the event.</p>\n";
    std::cout << "Dinner and camping fees are non-refundable if you don't attend the event.</p>\n";

    std::cout << "</div>\n";

    std::cout << "</div>\n";
}

int main () {
    try {
        // work out if the user has requested the event, camping or dinner form
        std::string page_request = CgiEnvironment::getRequestUri();
        bool camping_form_only = false;
        bool dinner_form_only = false;
        if (page_request.substr(0, 8) == "/camping")
            camping_form_only = true;
        if (page_request.substr(0, 7) == "/dinner")
            dinner_form_only = true;

        JlweCore jlwe;
        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        int query_dinner_id = 0;
        try {
            query_dinner_id = std::stoi(urlQueries.getValue("id"));
        } catch (...) {}

        sql::Statement *stmt;
        sql::PreparedStatement *prep_stmt;
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
            std::string open_time_html = "<span class=\"date_only\" data-value=\"" + std::to_string(registration_open_date) + "\">" + FormElements::timeToDateString(registration_open_date) + "</span>";
            if (jlwe.getPermissionValue("perm_registrations")) { //if logged in
                std::cout << "<div class=\"note\"><p><span style=\"font-weight:bold;\">The registration form is currently closed.</span> This is a preview available to admins. It will open to everyone on <span style=\"font-weight:bold;\">" << open_time_html << "</span></p></div>\n";
            } else {
                std::cout << "<p>The registration form will be live on " << open_time_html << "</p>";

                std::cout << FormElements::includeJavascript("/js/format_date_time.js?v=2024");
                html.outputFooter();
                return 0;
            }
        }

        // get list of dinner forms
        std::vector<dinner_form> dinner_forms;
        if (!camping_form_only) { // but don't bother if we only want camping
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT dinner_id,title,unix_timestamp(order_close_time),html_path FROM dinner_forms WHERE enabled > 0;");
            while (res->next()) {
                dinner_forms.push_back({res->getInt(1), res->getString(2), res->getInt64(3), res->getString(4)});
            }
            delete res;
            delete stmt;
        }

        bool dinner_form_enabled = (dinner_forms.size() > 0);
        // Can't have a dinner only order if there is no dinner
        if (!dinner_form_enabled && dinner_form_only) {
            std::cout << "<p style=\"color:red;text-align:center;font-weight:bold;\">Error: There are no dinner forms enabled. Please contact us on " << std::string(jlwe.config.at("adminEmail")) << "</p>\n";
            html.outputFooter();
            return 0;
        }
        // If dinner only, then a form id needs to be given
        if (dinner_form_only && (dinner_forms.size() > 1) && (query_dinner_id == 0)) {
            std::cout << "<p style=\"color:red;text-align:center;font-weight:bold;\">Error: dinner form id needs to be set. Please contact us on " << std::string(jlwe.config.at("adminEmail")) << "</p>\n";
            html.outputFooter();
            return 0;
        }

        // No need for id number if there's only one dinner form
        if (dinner_forms.size() == 1)
            query_dinner_id = dinner_forms.at(0).dinner_id;

        std::cout << FormElements::includeJavascript("https://js.stripe.com/v3/");
        std::cout << FormElements::includeJavascript("/cgi-bin/stripe_keys.cgi");
        std::cout << FormElements::includeJavascript("/js/utils.js");
        std::cout << FormElements::includeJavascript("/js/form_tools.js");
        std::cout << FormElements::includeJavascript("/js/form_elements.js");
        std::cout << FormElements::includeJavascript("/js/registration_form.js?v=2024");
        std::cout << FormElements::includeJavascript("/js/dinner.js?v=2024");
        std::cout << FormElements::includeJavascript("/js/uuid.js");

        if (jlwe.config.at("stripe").value("testMode", false)) {
            std::cout << "<div class=\"note\"><p><span style=\"font-weight:bold;\">Stripe test mode is enabled.</span> You will not be charged. See <a href=\"https://stripe.com/docs/testing\">https://stripe.com/docs/testing</a></p></div>\n";
        }

        std::cout << "<div id=\"loader\" class=\"loader\" style=\"display:none;\"><div></div></div>\n";

        if (camping_form_only) {
            std::cout << "<h1>Camping Registration</h1>\n";
            std::cout << "<p style=\"font-weight:bold;\">This form is for booking camping only. If you have already booked camping as part of your event registration, you do not need to fill out this form.</p>\n";
        } else if (dinner_form_only) {
            bool form_found = false;
            for (unsigned int i = 0; i < dinner_forms.size(); i++) {
                if (dinner_forms.at(i).dinner_id == query_dinner_id) {
                    form_found = true;
                    std::cout << "<h1>" << Encoder::htmlEntityEncode(dinner_forms.at(i).title) << "</h1>\n";
                    std::cout << "<p style=\"font-weight:bold;\">This form is for " << Encoder::htmlEntityEncode(dinner_forms.at(i).title) << " orders only. If you have already ordered dinner as part of your event registration, you do not need to fill out this form.</p>\n";
                }
            }
            if (!form_found) {
                std::cout << "<h1>Dinner orders</h1>\n";
                std::cout << "<p style=\"color:red;text-align:center;font-weight:bold;\">Error: Invalid form ID.</p>\n";
                html.outputFooter();
                return 0;
            }
        } else {
            std::cout << "<h1>Event Registration</h1>\n";
        }

        std::cout << "<noscript><p style=\"color:red;text-align:center;font-weight:bold;\">You need to have JavaScript enabled.</p></noscript>\n";

        // get dates
        time_t camping_cutoff = 0;
        try {
            camping_cutoff = std::stoll(jlwe.getGlobalVar("camping_cutoff_date"));
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


        std::string event_registration_html, camping_registration_html;
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

        std::cout << "<form id=\"regForm\">\n";

        if (!camping_form_only && !dinner_form_only)
            outputEventTab(event_registration_html, camping_cutoff, time_now, &dinner_forms);
        if (!dinner_form_only)
            outputCampingTab(camping_registration_html, camping_form_only, camping_cutoff, time_now, saturday_date, &jlwe);
        for (unsigned int i = 0; i < dinner_forms.size(); i++) {
            std::string dinner_registration_html;
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT html FROM webpages WHERE path = ?;");
            prep_stmt->setString(1, dinner_forms.at(i).html_path);
            res = prep_stmt->executeQuery();
            while (res->next()) {
                dinner_registration_html = res->getString(1);
            }
            delete res;
            delete prep_stmt;

            if (dinner_form_only && dinner_forms.at(i).dinner_id == query_dinner_id) {
                outputDinnerTab(dinner_registration_html, true, &dinner_forms.at(i), time_now);
            } else if (!dinner_form_only) {
                outputDinnerTab(dinner_registration_html, false, &dinner_forms.at(i), time_now);
            }
        }
        outputSummaryTab(&dinner_forms);

        std::cout << FormElements::formMessages();
        std::cout << FormElements::formButtons();

        std::cout << "</form>\n";

        std::cout << FormElements::includeJavascript("/js/format_date_time.js?v=2024");

        std::cout << "<script type=\"text/javascript\">\n";
        std::cout << "var event_form = " << ((camping_form_only == false && dinner_form_only == false) ? "true" : "false") << ";\n";

        std::cout << "var camping = " << (camping_form_only ? "true" : "false") << ";\n";
        if (dinner_form_only) {
            std::cout << "var dinner = [";
            for (unsigned int i = 0; i < dinner_forms.size(); i++) {
                if (i != 0)
                    std::cout << ",";
                if (dinner_forms.at(i).dinner_id == query_dinner_id) {
                    std::cout << "true";
                } else {
                    std::cout << "false";
                }
            }
            std::cout << "];\n";
        } else {
            std::cout << "var dinner = [];\n";
        }

        std::cout << "var dinner_forms = [";
        for (int i = 0; i < dinner_forms.size(); i++) {
            if (i != 0)
                std::cout << ",";
            std::cout << "{dinner_id:" + std::to_string(dinner_forms.at(i).dinner_id) + ",title:\"" + Encoder::javascriptAttributeEncode(dinner_forms.at(i).title) + "\"}";
        }
        std::cout << "];\n";

        if (camping_form_only || dinner_form_only) {
            std::cout << "var summary_page = 1;\n";
        } else {
            std::cout << "var summary_page = " << std::to_string(2 + dinner_forms.size()) << ";\n"; // index of the summary of costs page (3 with dinner, 2 dinner disabled)
        }

        if (!camping_form_only && !dinner_form_only)
            std::cout << "var formPostURL = stripeSessionURL + '?type=event';\n";
        if (camping_form_only)
            std::cout << "var formPostURL = stripeSessionURL + '?type=camping_only';\n";
        if (dinner_form_only)
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
