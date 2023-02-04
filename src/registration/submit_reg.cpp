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
#include "../core/JsonUtils.h"
#include "../core/KeyValueParser.h"
#include "../core/PostDataParser.h"
#include "../email/Email.h"
#include "../email/EmailTemplates.h"
#include "../prices.h"

#include "../ext/nlohmann/json.hpp"

// Page that customers are sent to once the order is completed
#define CONFIRMATION_PAGE_URL "/cgi-bin/registration/confirmation_reg.cgi"

int main () {
    try {
        JlweCore jlwe;
        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << JsonUtils::makeJsonError(postData.errorText());
            return 0;
        }

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
        bool dinner = false;

        int dinner_number_adults = 0;
        int dinner_number_children = 0;
        std::string dinner_comment = "";
        int dinner_number_adults_op1 = 0;
        int dinner_number_adults_op2 = 0;
        int dinner_number_adults_op3 = 0;
        int dinner_number_children_op1 = 0;
        int dinner_number_children_op2 = 0;
        int dinner_number_children_op3 = 0;
        int dinner_number_dessert_op1 = 0;
        int dinner_number_dessert_op2 = 0;
        int dinner_number_dessert_op3 = 0;

        std::string camping_type = "unpowered";
        std::string camping_comment = "";
        int arrive_date = 0;
        int leave_date = 0;
        int number_people = 0;

        int id_number = 0;

        bool isValid = true;
        bool isFullEvent = (regType == "event");
        std::string error_message = "";

        // dinner details
        if (jsonDocument.contains("dinner")) {
            if (jsonDocument.at("dinner").is_boolean()) {
                dinner = jsonDocument.at("dinner");
            } else if (jsonDocument.at("dinner").is_object()) {
                dinner = true;
                nlohmann::json dinnerObject = jsonDocument.at("dinner");

                dinner_number_adults_op1 = dinnerObject.value("dinner_number_adults_op1", 0);
                dinner_number_adults_op2 = dinnerObject.value("dinner_number_adults_op2", 0);
                dinner_number_adults_op3 = dinnerObject.value("dinner_number_adults_op3", 0);
                dinner_number_children_op1 = dinnerObject.value("dinner_number_children_op1", 0);
                dinner_number_children_op2 = dinnerObject.value("dinner_number_children_op2", 0);
                dinner_number_children_op3 = dinnerObject.value("dinner_number_children_op3", 0);
                dinner_number_dessert_op1 = dinnerObject.value("dinner_number_dessert_op1", 0);
                dinner_number_dessert_op2 = dinnerObject.value("dinner_number_dessert_op2", 0);
                dinner_number_dessert_op3 = dinnerObject.value("dinner_number_dessert_op3", 0);

                /*if (dinnerObject.HasMember("dinner_number_adults") && dinnerObject["dinner_number_adults"].IsInt())
                    dinner_number_adults = dinnerObject["dinner_number_adults"].GetInt();
                if (dinnerObject.HasMember("dinner_number_children") && dinnerObject["dinner_number_children"].IsInt())
                    dinner_number_children = dinnerObject["dinner_number_children"].GetInt();*/
                dinner_number_adults = dinner_number_adults_op1 + dinner_number_adults_op2 + dinner_number_adults_op3;
                dinner_number_children = dinner_number_children_op1 + dinner_number_children_op2 + dinner_number_children_op3;

                dinner_comment = dinnerObject.value("dinner_comment", "");

                if (regType == "dinner_only") {
                    number_adults = dinner_number_adults;
                    number_children = dinner_number_children;
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

                camping_type = campingObject.value("camping_type", "unpowered");
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
        time_t dinner_cutoff = 0;
        try {
            dinner_cutoff = std::stoll(jlwe.getGlobalVar("dinner_cutoff_date"));
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
        if (dinner) {
            if (dinner_number_adults < 0 || dinner_number_children < 0) {
                isValid = false;
                error_message = "Number of dinners ordered must not be negative";
            }

            if (dinner_number_adults + dinner_number_children < dinner_number_dessert_op1 + dinner_number_dessert_op2 + dinner_number_dessert_op3) {
                isValid = false;
                error_message = "You can't order more desserts than main meals";
            }

            if ((time(nullptr) > dinner_cutoff)) {
                isValid = false;
                error_message = "Dinner bookings have now closed. You are too late to order a meal.";
            }
        }

        // check camping
        if (camping) {
            if (camping_type != "unpowered" && camping_type != "powered") {
                isValid = false;
                error_message = "Invalid camping type: " + camping_type;
            }

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
                prep_stmt->setString(13, (dinner ? "yes" : "no"));
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

            bool saveDinner = false;
            if (dinner) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT insertDinner(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
                prep_stmt->setString(1, idempotencySafe);
                prep_stmt->setString(2, email);
                prep_stmt->setString(3, gc_username);
                prep_stmt->setString(4, phone);
                prep_stmt->setInt(5, (mode == "live"));
                prep_stmt->setInt(6, dinner_number_adults);
                prep_stmt->setInt(7, dinner_number_children);
                prep_stmt->setString(8, dinner_comment);
                prep_stmt->setString(9, isFullEvent ? "event" : payment_type);
                prep_stmt->setString(10, jlwe.getCurrentUserIP());
                prep_stmt->setString(11, jlwe.getCurrentUsername());
                prep_stmt->setInt(12, dinner_number_adults_op1);
                prep_stmt->setInt(13, dinner_number_adults_op2);
                prep_stmt->setInt(14, dinner_number_adults_op3);
                prep_stmt->setInt(15, dinner_number_children_op1);
                prep_stmt->setInt(16, dinner_number_children_op2);
                prep_stmt->setInt(17, dinner_number_children_op3);
                prep_stmt->setInt(18, dinner_number_dessert_op1);
                prep_stmt->setInt(19, dinner_number_dessert_op2);
                prep_stmt->setInt(20, dinner_number_dessert_op3);
                res = prep_stmt->executeQuery();
                if (res->next() && res->getInt(1))
                    saveDinner = true;
                delete res;
                delete prep_stmt;
            } else {
                saveDinner = true;
            }


            if (saveEvent && saveDinner && saveCamping) { // if everything saved to MySQL successfully
                if (payment_type == "card") {
                    std::string post_data = "payment_method_types[]=card&customer_email=" + Encoder::urlEncode(email) + "&success_url=" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(CONFIRMATION_PAGE_URL) + "?session_id={CHECKOUT_SESSION_ID}&cancel_url=" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(CONFIRMATION_PAGE_URL) + "?session_id={CHECKOUT_SESSION_ID}%26cancel=true";
                    int total_cost = 0;

                    if (isFullEvent) {
                        int payment_event_total = number_adults * PRICE_EVENT_ADULT + number_children * PRICE_EVENT_CHILD;
                        if (payment_event_total > 0) {
                            std::string description = "Event+registration+for+" + Encoder::urlEncode(gc_username) + " (" + std::to_string(number_adults) + "+adult(s)+and+" + std::to_string(number_children) + "+children)";
                            post_data += "&line_items[][name]=Event+registration&line_items[][description]=" + description + "&line_items[][images][]=https://jlwe.org/img/jlwe_logo.png&line_items[][amount]=" + std::to_string(payment_event_total) + "&line_items[][currency]=aud&line_items[][quantity]=1";
                            total_cost += payment_event_total;
                        }
                    }

                    if (camping) {
                        int payment_camping_total = getCampingPrice(camping_type, number_people, camping_nights);
                        std::string description = "";
                        if (camping_type == "unpowered") {
                            description = "Unpowered+site,+" + std::to_string(number_people) + "+people,+" + std::to_string(camping_nights) + "+nights";
                        }
                        if (camping_type == "powered") {
                            description = "Powered+site,+" + std::to_string(number_people) + "+people,+" + std::to_string(camping_nights) + "+nights";
                        }
                        if (payment_camping_total > 0) {
                            post_data += "&line_items[][name]=Camping&line_items[][description]=" + description + "&line_items[][images][]=https://jlwe.org/img/camping_icon.png&line_items[][amount]=" + std::to_string(payment_camping_total) + "&line_items[][currency]=aud&line_items[][quantity]=1";
                            total_cost += payment_camping_total;
                        }
                    }

                    if (dinner) {
                        int payment_dinner_total = dinner_number_adults * PRICE_DINNER_ADULT + dinner_number_children * PRICE_DINNER_CHILD;
                        if (payment_dinner_total > 0) {
                            std::string description = std::to_string(dinner_number_adults) + "+adult+dinners+and+" + std::to_string(dinner_number_children) + "+child+dinners";
                            post_data += "&line_items[][name]=Saturday+dinner&line_items[][description]=" + description + "&line_items[][images][]=https://jlwe.org/img/dinner_icon.png&line_items[][amount]=" + std::to_string(payment_dinner_total) + "&line_items[][currency]=aud&line_items[][quantity]=1";
                            total_cost += payment_dinner_total;
                        }
                    }

                    int card_surcharge_cost = static_cast<int>(static_cast<double>(total_cost + 30) / (1 - 0.0175)) - total_cost;
                    if (card_surcharge_cost > 0)
                        post_data += "&line_items[][name]=Card+processing+fee&line_items[][description]=Stripe+card+payment+processing+fee&line_items[][amount]=" + std::to_string(card_surcharge_cost) + "&line_items[][currency]=aud&line_items[][quantity]=1";


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
                            std::string payment_intent = stripeJsonDocument.at("payment_intent");

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
