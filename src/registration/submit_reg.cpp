/**
  @file    submit_reg.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/registration/submit_reg.cgi
  This receives the registration data from the customer and validates it
  If the registration is valid, it saves it and makes the request to Stripe to start the payment session
  This script is called when the customer presses submit on the registration form
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/Encoder.h"
#include "../core/HttpRequest.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/JsonUtils.h"
#include "../core/KeyValueParser.h"
#include "../core/PostDataParser.h"
#include "../email/Email.h"
#include "../email/EmailTemplates.h"
#include "../prices.h"
#include "DinnerUtils.h"

#include "../ext/nlohmann/json.hpp"

// Page that customers are sent to once the order is completed
#define CONFIRMATION_PAGE_URL "/cgi-bin/registration/confirmation_reg.cgi"

struct dinner_form {
    int dinner_id;
    std::string title;
    time_t order_close_time;
    std::string config;
    bool in_use;
};

// hacky function to check the incoming dinner meal selection JSON
nlohmann::json validateMeal(nlohmann::json json_in) {
    nlohmann::json json_out;
    if (json_in.contains("name") && json_in.at("name").is_string())
        json_out["name"] = json_in.at("name");

    if (json_in.contains("courses") && json_in.at("courses").is_object()) {
        json_out["courses"] = nlohmann::json::object();
        for (nlohmann::json::iterator it = json_in.at("courses").begin(); it != json_in.at("courses").end(); ++it) {
            std::string key = it.key();
            int value = it.value();
            json_out["courses"][key] = value;
        }
    }

    if (json_in.contains("item_options") && json_in.at("item_options").is_object()) {
        json_out["item_options"] = nlohmann::json::object();
        for (nlohmann::json::iterator it = json_in.at("item_options").begin(); it != json_in.at("item_options").end(); ++it) {
            std::string key = it.key();
            if (it.value().is_string()) {
                std::string value = it.value();
                json_out["item_options"][key] = value;
            } else if (it.value().is_array()) {
                std::vector<bool> value = it.value();
                json_out["item_options"][key] = value;
            }
        }
    }

    return json_out;
}

// Makes the post data for a single item to be put in the data sent to Stripe
std::string make_stripe_line_item(int idx, std::string name, std::string description, int price, int quantity, std::string image_url = "") {
    std::string line_item = "";
    line_item += "line_items[" + std::to_string(idx) + "][price_data][product_data][name]=" + name;
    line_item += "&line_items[" + std::to_string(idx) + "][price_data][product_data][description]=" + description;
    if (image_url.size())
        line_item += "&line_items[" + std::to_string(idx) + "][price_data][product_data][images][]=" + image_url;
    line_item += "&line_items[" + std::to_string(idx) + "][price_data][unit_amount]=" + std::to_string(price);
    line_item += "&line_items[" + std::to_string(idx) + "][price_data][currency]=aud";
    line_item += "&line_items[" + std::to_string(idx) + "][quantity]=" + std::to_string(quantity);
    return line_item;
}

int main () {
    try {
        JlweCore jlwe;
        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << JsonUtils::makeJsonError(postData.errorText());
            return 0;
        }

        sql::Statement *stmt;
        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        std::string regType = urlQueries.getValue("type");
        if (regType.size() == 0)
            regType = "event";

        nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

        // generic stuff
        std::string idempotency = jsonDocument.at("idempotency");
        std::string gc_username = jsonDocument.at("gc_username");
        std::string email = jsonDocument.at("email");
        std::string phone = jsonDocument.at("phone");
        std::string mode = jsonDocument.at("mode");
        std::string payment_type = jsonDocument.at("payment_type");

        // This gets used in a lot of places so make sure it only has safe characters in it
        std::string idempotencySafe = Encoder::filterSafeCharsOnly(idempotency);

        // event registration details
        std::string real_names_adults = jsonDocument.value("real_names_adults", "");
        std::string real_names_children = jsonDocument.value("real_names_children", "");
        int number_adults = jsonDocument.value("number_adults", 0);
        int number_children = jsonDocument.value("number_children", 0);
        bool email_gpx = jsonDocument.value("email_gpx", false);
        bool past_jlwe = jsonDocument.value("past_jlwe", false);
        bool have_lanyard = jsonDocument.value("have_lanyard", false);

        bool camping = false;

        std::vector<std::string> dinner_comments;
        std::vector<std::string> dinner_orders;
        std::vector<int> dinner_number_adults;
        std::vector<int> dinner_number_children;

        std::string camping_type = "unpowered";
        std::string camping_comment = "";
        int arrive_date = 0;
        int leave_date = 0;
        int number_people = 0;

        bool isValid = true;
        bool isFullEvent = (regType == "event");
        std::string error_message = "";

        // get list of dinner forms
        std::vector<dinner_form> dinner_forms;
        stmt = jlwe.getMysqlCon()->createStatement();
        res = stmt->executeQuery("SELECT dinner_id,title,unix_timestamp(order_close_time),config FROM dinner_forms WHERE enabled > 0;");
        while (res->next()) {
            dinner_forms.push_back({res->getInt(1), res->getString(2), res->getInt64(3), res->getString(4), false});
        }
        delete res;
        delete stmt;


        // dinner details
        if (jsonDocument.contains("dinner") && jsonDocument.at("dinner").is_object()) {
            for (unsigned int i = 0; i < dinner_forms.size(); i++) {
                if (jsonDocument.at("dinner").contains(std::to_string(dinner_forms.at(i).dinner_id))) {
                    dinner_forms[i].in_use = true;
                    //dinner_forms_in_use++;
                    nlohmann::json dinnerObject = jsonDocument.at("dinner").at(std::to_string(dinner_forms.at(i).dinner_id));

                    nlohmann::json configJson = nlohmann::json::parse(dinner_forms.at(i).config);
                    std::vector<DinnerUtils::dinner_menu_item> menu_items = DinnerUtils::getDinnerMenuItems(jlwe.getMysqlCon(), dinner_forms.at(i).dinner_id);

                    unsigned int adults = 0;
                    unsigned int children = 0;

                    nlohmann::json orderObject = {};
                    if (dinnerObject.contains("categories") && configJson.contains("categories")) {

                        // Check that the meal options are valid and remove any extra objects that might be in the JSON
                        // Essently does this: orderObject["categories"] = dinnerObject.at("categories");
                        // This can probably be done better
                        for (nlohmann::json::iterator course_it = configJson.at("courses").begin(); course_it != configJson.at("courses").end(); ++course_it) {
                            int category_id = course_it.value().at("category_id");
                            if (dinnerObject.at("categories").contains(std::to_string(category_id)) && dinnerObject.at("categories").at(std::to_string(category_id)).is_array()) {

                                nlohmann::json meal_array_out = nlohmann::json::array();
                                for (nlohmann::json::iterator meal_it = dinnerObject.at("categories").at(std::to_string(category_id)).begin(); meal_it != dinnerObject.at("categories").at(std::to_string(category_id)).end(); ++meal_it) {
                                    meal_array_out.push_back(validateMeal(meal_it.value()));
                                }

                                orderObject["categories"][std::to_string(category_id)] = meal_array_out;
                            }
                        }

                        // hack to get total counts for adult and child meals
                        if (dinnerObject.at("categories").contains("10") && dinnerObject.at("categories").at("10").is_array())
                            adults = dinnerObject.at("categories").at("10").size();
                        if (dinnerObject.at("categories").contains("20") && dinnerObject.at("categories").at("20").is_array())
                            children = dinnerObject.at("categories").at("20").size();
                    }

                    if (dinnerObject.contains("meals")) {

                        // Check that the meal options are valid and remove any extra objects that might be in the JSON
                        // Essently does this: orderObject["meals"] = dinnerObject.at("meals");
                        // This can probably be done better
                        for (unsigned int j = 0; j < menu_items.size(); j++) {
                            int meal_id = menu_items.at(j).id;
                            if (dinnerObject.at("meals").contains(std::to_string(meal_id)) && dinnerObject.at("meals").at(std::to_string(meal_id)).is_array()) {

                                nlohmann::json meal_array_out = nlohmann::json::array();
                                for (nlohmann::json::iterator meal_it = dinnerObject.at("meals").at(std::to_string(meal_id)).begin(); meal_it != dinnerObject.at("meals").at(std::to_string(meal_id)).end(); ++meal_it) {
                                    meal_array_out.push_back(validateMeal(meal_it.value()));
                                }

                                orderObject["meals"][std::to_string(meal_id)] = meal_array_out;
                            }
                        }

                        // hack to get total counts for adult and child meals
                        if (dinnerObject.at("meals").contains("1") && dinnerObject.at("meals").at("1").is_array())
                            adults = dinnerObject.at("meals").at("1").size();
                        if (dinnerObject.at("meals").contains("2") && dinnerObject.at("meals").at("2").is_array())
                            children = dinnerObject.at("meals").at("2").size();
                    }

                    dinner_comments.push_back(dinnerObject.value("comment", ""));
                    dinner_orders.push_back(orderObject.dump());
                    dinner_number_adults.push_back(adults);
                    dinner_number_children.push_back(children);
                } else {
                    dinner_comments.push_back("");
                    dinner_orders.push_back("");
                    dinner_number_adults.push_back(0);
                    dinner_number_children.push_back(0);
                }
            }
        }

        // camping details
        if (jsonDocument.contains("camping")) {
            if (jsonDocument.at("camping").is_boolean()) {
                camping = jsonDocument.at("camping");
            } else if (jsonDocument.at("camping").is_object()) {
                camping = true;
                nlohmann::json campingObject = jsonDocument.at("camping");

                camping_type = campingObject.value("camping_type", "unknown");
                arrive_date = campingObject.value("arrive_date", 0);
                leave_date = campingObject.value("leave_date", 0);
                camping_comment = campingObject.value("camping_comment", "");
                number_people = campingObject.value("number_people", 0);

                if (regType == "camping_only") {
                    number_adults = number_people;
                    number_children = 0;
                }
            }
        }
        int camping_nights = leave_date - arrive_date;

        if (regType != "event" && regType != "camping_only" && regType != "dinner_only") {
            isValid = false;
            error_message = "Invalid regType: " + regType;
        }

        // check data submitted is valid

        if (idempotencySafe.size() < 30) {
            isValid = false;
            error_message = "Idempotency key is too short";
        }

        bool server_test_mode = jlwe.config.at("stripe").value("testMode", false);
        if (server_test_mode && mode != "test") {
            isValid = false;
            error_message = "Server mode (test) does not match: " + mode;
        }
        if (!server_test_mode && mode != "live") {
            isValid = false;
            error_message = "Server mode (live) does not match: " + mode;
        }


        if (!Email::isValidEmail(email)) {
            isValid = false;
            error_message = "Invalid email address: " + email;
        }

        bool unique_name = true;
        // enable this bit to force unique gc usernames
        /*if (regType == "event") {
            prep_stmt = con->prepareStatement("SELECT * FROM event_registrations WHERE gc_username = ?;");
            prep_stmt->setString(1, gc_username);
            res = prep_stmt->executeQuery();
            if (res->next()){
                unique_name = false;
            }
            delete res;
            delete prep_stmt;
        }*/
        if (!unique_name || gc_username.size() == 0) {
            isValid = false;
            error_message = "Invalid GC username (or name already registered)";
        }

        if (payment_type != "card" && payment_type != "cash" && payment_type != "bank") {
            isValid = false;
            error_message = "Invalid payment type: " + payment_type;
        }

        // get cutoff dates
        time_t camping_cutoff = 0;
        try {
            camping_cutoff = std::stoll(jlwe.getGlobalVar("camping_cutoff_date"));
        } catch (...) {}

        // check main event details
        if (isFullEvent) {
            if (number_adults < 0 || number_children < 0) {
                isValid = false;
                error_message = "Number of people must not be negative";
            }

            if ((number_adults + number_children) < 1) {
                isValid = false;
                error_message = "You must have at least one person in your team";
            }
        }

        // check dinner
        for (unsigned int i = 0; i < dinner_forms.size(); i++) {
            if (dinner_forms.at(i).in_use) {
                if (dinner_forms.at(i).order_close_time) {
                    if ((time(nullptr) > dinner_forms.at(i).order_close_time)) {
                        isValid = false;
                        error_message = "Orders for the " + dinner_forms.at(i).title + " have now closed. You are too late to order a meal.";
                    }
                }
            }
        }

        // check camping
        int camping_total_available = 0;
        std::string camping_display_name = "Unknown Type";
        std::string camping_price_code = "unknown";
        if (camping) {
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT display_name,price_code,total_available,active FROM camping_options WHERE id_string = ?;");
            prep_stmt->setString(1, camping_type);
            res = prep_stmt->executeQuery();
            if (res->next()) {
                camping_display_name = res->getString(1);
                camping_price_code = res->getString(2);
                if (res->getInt(4)) {
                    // type ok, get total available sites
                    if (res->isNull(3)) {
                        camping_total_available = 1000;
                    } else {
                        camping_total_available = res->getInt(3);
                    }
                } else { // inactive type
                    isValid = false;
                    error_message = "Sorry, " + camping_display_name + " camping is no longer available for purchase";
                }
            } else {
                isValid = false;
                error_message = "Invalid camping type: " + camping_type;
            }
            delete res;
            delete prep_stmt;

            if (camping_nights == 0) {
                isValid = false;
                error_message = "If you arrive and leave on the same day, you won't be camping (invalid camping dates)";
            }

            if (camping_nights < 0) {
                isValid = false;
                error_message = "You can't leave before you arrive (invalid camping dates)";
            }

            if (number_people < 1) {
                isValid = false;
                error_message = "There needs to be at least one person to book a camping site.";
            }

            if (number_people > 5) {
                isValid = false;
                error_message = "There is a maximum of 5 people per camping site.";
            }

            if ((time(nullptr) > camping_cutoff)) {
                isValid = false;
                error_message = "Camping site bookings have now closed. You are too late to reserve a camping site.";
            }
            // uncomment this to block powered sites
            /*if (camping_type == "powered") {
                isValid = false;
                error_message = "Powered camping sites are no longer available.";
            }*/
        }

        if (jlwe.getGlobalVar("registration_enabled") != "1") {
            isValid = false;
            error_message = "Registrations are now closed.";
        }


        if (isValid) {

            bool saveEvent = false;
            if (isFullEvent) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT insertRegistration(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
                prep_stmt->setString(1, idempotencySafe);
                prep_stmt->setString(2, email);
                prep_stmt->setString(3, gc_username);
                prep_stmt->setString(4, phone);
                prep_stmt->setInt(5, (mode == "live"));
                prep_stmt->setString(6, real_names_adults);
                prep_stmt->setString(7, real_names_children);
                prep_stmt->setInt(8, number_adults);
                prep_stmt->setInt(9, number_children);
                prep_stmt->setInt(10, past_jlwe);
                prep_stmt->setInt(11, have_lanyard);
                prep_stmt->setString(12, (camping ? "yes" : "no"));

                int dinnerCount = 0;
                for (unsigned int i = 0; i < dinner_forms.size(); i++)
                    if (dinner_forms.at(i).in_use) dinnerCount++;
                prep_stmt->setString(13, std::to_string(dinnerCount));

                prep_stmt->setString(14, payment_type);
                prep_stmt->setString(15, jlwe.getCurrentUserIP());
                prep_stmt->setString(16, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next() && res->getInt(1))
                    saveEvent = true;
                delete res;
                delete prep_stmt;
            } else {
                saveEvent = true;
            }

            bool saveCamping = false;
            if (camping) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT insertCamping(?,?,?,?,?,?,?,?,?,?,?,?,?);");
                prep_stmt->setString(1, idempotencySafe);
                prep_stmt->setString(2, email);
                prep_stmt->setString(3, gc_username);
                prep_stmt->setString(4, phone);
                prep_stmt->setInt(5, (mode == "live"));
                prep_stmt->setInt(6, number_people);
                prep_stmt->setString(7, camping_type);
                prep_stmt->setInt(8, arrive_date);
                prep_stmt->setInt(9, leave_date);
                prep_stmt->setString(10, camping_comment);
                prep_stmt->setString(11, isFullEvent ? "event" : payment_type);
                prep_stmt->setString(12, jlwe.getCurrentUserIP());
                prep_stmt->setString(13, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next() && res->getInt(1))
                    saveCamping = true;
                delete res;
                delete prep_stmt;
            } else {
                saveCamping = true;
            }

            int saveDinnerCount = 0;
            for (unsigned int i = 0; i < dinner_forms.size(); i++) {
                if (dinner_forms.at(i).in_use) {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT insertDinner(?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
                    prep_stmt->setString(1, idempotencySafe);
                    prep_stmt->setString(2, email);
                    prep_stmt->setString(3, gc_username);
                    prep_stmt->setString(4, phone);
                    prep_stmt->setInt(5, dinner_forms[i].dinner_id);
                    prep_stmt->setInt(6, (mode == "live"));
                    prep_stmt->setInt(7, dinner_number_adults[i]);
                    prep_stmt->setInt(8, dinner_number_children[i]);
                    prep_stmt->setString(9, dinner_comments[i]);
                    prep_stmt->setString(10, isFullEvent ? "event" : payment_type);
                    prep_stmt->setString(11, jlwe.getCurrentUserIP());
                    prep_stmt->setString(12, jlwe.getCurrentUsername());
                    prep_stmt->setString(13, dinner_orders[i]);
                    prep_stmt->setString(14, "");
                    res = prep_stmt->executeQuery();
                    if (res->next() && res->getInt(1))
                        saveDinnerCount++;
                    delete res;
                    delete prep_stmt;
                } else {
                    saveDinnerCount++;
                }
            }


            if (saveEvent && (saveDinnerCount == dinner_forms.size()) && saveCamping) { // if everything saved to MySQL successfully
                if (payment_type == "card") {
                    std::string post_data = "mode=payment&payment_method_types[]=card&customer_email=" + Encoder::urlEncode(email) + "&success_url=" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(CONFIRMATION_PAGE_URL) + "?session_id={CHECKOUT_SESSION_ID}&cancel_url=" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(CONFIRMATION_PAGE_URL) + "?session_id={CHECKOUT_SESSION_ID}%26cancel=true";
                    int total_cost = 0;

                    int line_items_idx = 0;
                    if (isFullEvent) {
                        int payment_event_total = number_adults * PRICE_EVENT_ADULT + number_children * PRICE_EVENT_CHILD;
                        if (payment_event_total > 0) {
                            std::string description = "Event+registration+for+" + Encoder::urlEncode(gc_username) + " (" + std::to_string(number_adults) + "+adult(s)+and+" + std::to_string(number_children) + "+children)";
                            post_data += "&" + make_stripe_line_item(line_items_idx++, "Event+registration", description, payment_event_total, 1, std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/img/jlwe_logo.png");
                            total_cost += payment_event_total;
                        }
                    }

                    if (camping) {
                        int payment_camping_total = getCampingPrice(camping_price_code, number_people, camping_nights);
                        std::string description = "";

                        std::string camping_display_name_encoded = JlweUtils::replaceString(camping_display_name, " ", "+");
                        description = camping_display_name_encoded + ",+" + std::to_string(number_people) + "+" + (number_people > 1 ? "people" : "person") + ",+" + std::to_string(camping_nights) + "+night" + (camping_nights > 1 ? "s" : "");

                        if (payment_camping_total > 0) {
                            post_data += "&" + make_stripe_line_item(line_items_idx++, "Camping", description, payment_camping_total, 1, std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/img/camping_icon.png");
                            total_cost += payment_camping_total;
                        }
                    }

                    for (unsigned int i = 0; i < dinner_forms.size(); i++) {
                        if (dinner_forms.at(i).in_use) {
                            int payment_dinner_total = DinnerUtils::getDinnerCost(jlwe.getMysqlCon(), dinner_forms.at(i).dinner_id, dinner_orders[i]);
                            if (payment_dinner_total > 0) {
                                std::string dinner_event_name_encoded = JlweUtils::replaceString(dinner_forms.at(i).title, " ", "+");
                                std::string description = std::to_string(dinner_number_adults[i]) + "+adult+dinner" + (dinner_number_adults[i] == 1 ? "" : "s") + "+and+" + std::to_string(dinner_number_children[i]) + "+child+dinner" + (dinner_number_children[i] == 1 ? "" : "s");
                                post_data += "&" + make_stripe_line_item(line_items_idx++, dinner_event_name_encoded, description, payment_dinner_total, 1, std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/img/dinner_icon.png");
                                total_cost += payment_dinner_total;
                            }
                        }
                    }

                    int card_surcharge_cost = static_cast<int>(static_cast<double>(total_cost + 30) / (1 - 0.0175)) - total_cost;
                    if (card_surcharge_cost > 0)
                        post_data += "&" + make_stripe_line_item(line_items_idx++, "Card+processing+fee", "Stripe+card+payment+processing+fee", card_surcharge_cost, 1);

                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT insertStripeFee(?,?,?,?);");
                    prep_stmt->setString(1, idempotencySafe);
                    prep_stmt->setInt(2, card_surcharge_cost);
                    prep_stmt->setString(3, jlwe.getCurrentUserIP());
                    prep_stmt->setString(4, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    delete res;
                    delete prep_stmt;

                    std::string auth_data;
                    if (server_test_mode) {
                        auth_data = std::string(jlwe.config.at("stripe").value("secretKeyTest", "")) + ":";
                    } else {
                        auth_data = std::string(jlwe.config.at("stripe").value("secretKey", "")) + ":";
                    }
                    std::string auth_header = "Basic " + Encoder::base64encode(auth_data);
                    HttpRequest request(jlwe.config.at("stripe").at("url"));
                    request.setHeader("Authorization: " + auth_header);
                    request.setHeader("Idempotency-Key: " + idempotencySafe);
                    if (request.post(post_data, "application/x-www-form-urlencoded")) {
                        std::string http_response = request.responseAsString();

                        nlohmann::json stripeJsonDocument = nlohmann::json::parse(http_response);

                        if (stripeJsonDocument.contains("error") && stripeJsonDocument.at("error").is_object()) {
                            std::string stripe_error_message = stripeJsonDocument.at("error").value("message", "");

                            nlohmann::json jsonOut;
                            jsonOut["success"] = false;
                            jsonOut["error"] = stripe_error_message;
                            jsonOut["registrationId"] = idempotencySafe;
                            std::cout << JsonUtils::makeJsonHeader() + jsonOut.dump();
                        } else {
                            std::string stripe_id = stripeJsonDocument.at("id");
                            std::string stripe_url = stripeJsonDocument.value("url", "");
                            //std::string payment_intent = stripeJsonDocument.at("payment_intent");

                            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT addStripeSessionID(?,?);");
                            prep_stmt->setString(1, idempotencySafe);
                            prep_stmt->setString(2, stripe_id);
                            res = prep_stmt->executeQuery();
                            if (res->next() && res->getInt(1)) {

                                if (email_gpx) // add the email to the mailing list if requested
                                    EmailTemplates::addEmailtoMailingList(email, &jlwe);

                                nlohmann::json jsonOut;
                                jsonOut["success"] = true;
                                jsonOut["message"] = "Registration received, redirecting to Stripe for payment";
                                jsonOut["registrationId"] = idempotencySafe;
                                jsonOut["stripeSessionId"] = stripe_id;
                                if (stripe_url.size()) jsonOut["stripeUrl"] = stripe_url; else jsonOut["stripeUrl"] = nullptr;
                                std::cout << JsonUtils::makeJsonHeader() + jsonOut.dump();
                            } else {
                                nlohmann::json jsonOut;
                                jsonOut["success"] = false;
                                jsonOut["error"] = "Error saving Stripe session ID to jlwe database";
                                jsonOut["registrationId"] = idempotencySafe;
                                std::cout << JsonUtils::makeJsonHeader() + jsonOut.dump();
                            }
                            delete res;
                            delete prep_stmt;
                        }
                    } else {
                        std::cout << JsonUtils::makeJsonError("Error connecting to Stripe: " + request.errorMessage());
                    }

                } else { // if bank or cash payment

                    if (email_gpx) // add the email to the mailing list if requested
                        EmailTemplates::addEmailtoMailingList(email, &jlwe);

                    nlohmann::json jsonOut;
                    jsonOut["success"] = true;
                    jsonOut["message"] = "Registration received";
                    jsonOut["redirect"] = std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(CONFIRMATION_PAGE_URL) + "?key=" + idempotencySafe;
                    jsonOut["registrationId"] = idempotencySafe;
                    std::cout << JsonUtils::makeJsonHeader() + jsonOut.dump();
                }
            } else {
                std::cout << JsonUtils::makeJsonError("Error saving registration");
            }
        } else {
            std::cout << JsonUtils::makeJsonError(error_message);
        }

    } catch (sql::SQLException &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")");
    } catch (const std::exception &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()));
    }

    return 0;
}
