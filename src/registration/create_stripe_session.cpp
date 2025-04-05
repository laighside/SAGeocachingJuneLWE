/**
  @file    create_stripe_session.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/registration/create_stripe_session.cgi
  Creates a new Stripe seesion/invoice for a given order
  Used if a customer wants to change payment types after the order is placed
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/Encoder.h"
#include "../core/HttpRequest.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/JsonUtils.h"
#include "../core/PaymentUtils.h"
#include "../core/PostDataParser.h"
#include "../prices.h"
#include "DinnerUtils.h"

#include "../ext/nlohmann/json.hpp"

// Page that customers are sent to once the order is completed
#define CONFIRMATION_PAGE_URL "/cgi-bin/registration/confirmation_reg.cgi"

// Makes the post data for a single item to be put in the data sent to Stripe
// This function is defined twice, it probably shouldn't be!
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

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << JsonUtils::makeJsonError(postData.errorText());
            return 0;
        }

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_registrations")) { //if logged in

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            std::string userKey = jsonDocument.at("key");
            std::string userEmail = PaymentUtils::getUserEmail(jlwe.getMysqlCon(), userKey);
            std::string gc_username = PaymentUtils::getUserName(jlwe.getMysqlCon(), userKey);

            std::string post_data = "mode=payment&payment_method_types[]=card&customer_email=" + Encoder::urlEncode(userEmail) + "&success_url=" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(CONFIRMATION_PAGE_URL) + "?session_id={CHECKOUT_SESSION_ID}&cancel_url=" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(CONFIRMATION_PAGE_URL) + "?session_id={CHECKOUT_SESSION_ID}%26cancel=true";
            int total_cost = 0;

            int line_items_idx = 0;
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT number_adults, number_children FROM event_registrations WHERE idempotency = ?;");
            prep_stmt->setString(1, userKey);
            res = prep_stmt->executeQuery();
            if (res->next()){
                int payment_event_total = res->getInt(1) * PRICE_EVENT_ADULT + res->getInt(2) * PRICE_EVENT_CHILD;
                if (payment_event_total > 0) {
                    std::string description = "Event+registration+for+" + Encoder::urlEncode(gc_username) + " (" + std::to_string(res->getInt(1)) + "+adult(s)+and+" + std::to_string(res->getInt(2)) + "+children)";
                    post_data += "&" + make_stripe_line_item(line_items_idx++, "Event+registration", description, payment_event_total, 1, std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/img/jlwe_logo.png");
                    total_cost += payment_event_total;
                }
            }
            delete res;
            delete prep_stmt;

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT camping.number_people, camping_options.price_code, camping_options.display_name, (camping.leave_date - camping.arrive_date) FROM camping INNER JOIN camping_options ON camping.camping_type=camping_options.id_string WHERE camping.idempotency = ?;");
            prep_stmt->setString(1, userKey);
            res = prep_stmt->executeQuery();
            if (res->next()){
                int payment_camping_total = getCampingPrice(res->getString(2), res->getInt(1), res->getInt(4));
                std::string description = "";

                std::string camping_display_name_encoded = JlweUtils::replaceString(res->getString(3), " ", "+");
                description = camping_display_name_encoded + ",+" + std::to_string(res->getInt(1)) + "+" + (res->getInt(1) > 1 ? "people" : "person") + ",+" + std::to_string(res->getInt(4)) + "+night" + (res->getInt(4) > 1 ? "s" : "");

                if (payment_camping_total > 0) {
                    post_data += "&" + make_stripe_line_item(line_items_idx++, "Camping", description, payment_camping_total, 1, std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/img/camping_icon.png");
                    total_cost += payment_camping_total;
                }
            }
            delete res;
            delete prep_stmt;

            std::vector<DinnerUtils::dinner_form> dinner_forms = DinnerUtils::getDinnerFormList(jlwe.getMysqlCon());
            for (unsigned int i = 0; i < dinner_forms.size(); i++) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT number_adults,number_children,dinner_options_adults FROM sat_dinner WHERE idempotency = ? AND dinner_form_id = ?;");
                prep_stmt->setString(1, userKey);
                prep_stmt->setInt(2, dinner_forms.at(i).dinner_id);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    int payment_dinner_total = DinnerUtils::getDinnerCost(jlwe.getMysqlCon(), dinner_forms.at(i).dinner_id, res->getString(3));
                    if (payment_dinner_total > 0) {
                        std::string dinner_event_name_encoded = JlweUtils::replaceString(dinner_forms.at(i).title, " ", "+");
                        std::string description = std::to_string(res->getInt(1)) + "+adult+dinner" + (res->getInt(1) == 1 ? "" : "s") + "+and+" + std::to_string(res->getInt(2)) + "+child+dinner" + (res->getInt(2) == 1 ? "" : "s");
                        post_data += "&" + make_stripe_line_item(line_items_idx++, dinner_event_name_encoded, description, payment_dinner_total, 1, std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + "/img/dinner_icon.png");
                        total_cost += payment_dinner_total;
                    }
                }
            }

            int card_surcharge_cost = static_cast<int>(static_cast<double>(total_cost + 30) / (1 - 0.0175)) - total_cost;
            if (card_surcharge_cost > 0)
                post_data += "&" + make_stripe_line_item(line_items_idx++, "Card+processing+fee", "Stripe+card+payment+processing+fee", card_surcharge_cost, 1);

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT insertStripeFee(?,?,?,?);");
            prep_stmt->setString(1, userKey);
            prep_stmt->setInt(2, card_surcharge_cost);
            prep_stmt->setString(3, jlwe.getCurrentUserIP());
            prep_stmt->setString(4, jlwe.getCurrentUsername());
            res = prep_stmt->executeQuery();
            delete res;
            delete prep_stmt;

            std::string auth_data;
            bool server_test_mode = jlwe.config.at("stripe").value("testMode", false);
            if (server_test_mode) {
                auth_data = std::string(jlwe.config.at("stripe").value("secretKeyTest", "")) + ":";
            } else {
                auth_data = std::string(jlwe.config.at("stripe").value("secretKey", "")) + ":";
            }
            std::string auth_header = "Basic " + Encoder::base64encode(auth_data);
            HttpRequest request(jlwe.config.at("stripe").at("url"));
            request.setHeader("Authorization: " + auth_header);
            request.setHeader("Idempotency-Key: " + userKey);
            if (request.post(post_data, "application/x-www-form-urlencoded")) {
                std::string http_response = request.responseAsString();

                nlohmann::json stripeJsonDocument = nlohmann::json::parse(http_response);

                if (stripeJsonDocument.contains("error") && stripeJsonDocument.at("error").is_object()) {
                    std::string stripe_error_message = stripeJsonDocument.at("error").value("message", "");

                    nlohmann::json jsonOut;
                    jsonOut["success"] = false;
                    jsonOut["error"] = stripe_error_message;
                    jsonOut["registrationId"] = userKey;
                    std::cout << JsonUtils::makeJsonHeader() + jsonOut.dump();
                } else {
                    std::string stripe_id = stripeJsonDocument.at("id");
                    std::string stripe_url = stripeJsonDocument.value("url", "");
                    //std::string payment_intent = stripeJsonDocument.at("payment_intent");

                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT addStripeSessionID(?,?);");
                    prep_stmt->setString(1, userKey);
                    prep_stmt->setString(2, stripe_id);
                    res = prep_stmt->executeQuery();
                    if (res->next() && res->getInt(1)) {

                        nlohmann::json jsonOut;
                        jsonOut["success"] = true;
                        jsonOut["message"] = "Registration received, redirecting to Stripe for payment";
                        jsonOut["registrationId"] = userKey;
                        jsonOut["stripeSessionId"] = stripe_id;
                        if (stripe_url.size()) jsonOut["stripeUrl"] = stripe_url; else jsonOut["stripeUrl"] = nullptr;
                        std::cout << JsonUtils::makeJsonHeader() + jsonOut.dump();
                    } else {
                        nlohmann::json jsonOut;
                        jsonOut["success"] = false;
                        jsonOut["error"] = "Error saving Stripe session ID to jlwe database";
                        jsonOut["registrationId"] = userKey;
                        std::cout << JsonUtils::makeJsonHeader() + jsonOut.dump();
                    }
                    delete res;
                    delete prep_stmt;
                }
            } else {
                std::cout << JsonUtils::makeJsonError("Error connecting to Stripe: " + request.errorMessage());
            }
        } else {
            std::cout << JsonUtils::makeJsonError("You do not have permission to view this area");
        }
    } catch (sql::SQLException &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")");
    } catch (const std::exception &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()));
    }

    return 0;
}
