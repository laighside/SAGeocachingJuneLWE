/**
  @file    stripe_webhook.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This handles webhook requests from Stripe
  All user requests to this page should be blocked
  Only valid requests can return 200 OK

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "core/CgiEnvironment.h"
#include "core/JlweCore.h"
#include "core/JlweUtils.h"
#include "core/JsonUtils.h"
#include "core/PostDataParser.h"

#include "ext/nlohmann/json.hpp"
#include "ext/hash_library/hmac.h"
#include "ext/hash_library/sha256.h"

int main () {
    try {
        JlweCore jlwe;

        // Block any IP address that isn't on the approved list
        nlohmann::json ipList = jlwe.config.at("stripe").at("webhookIPAddressList");
        bool ipValid = false;
        if (ipList.is_array()) {
            for (nlohmann::json::iterator it = ipList.begin(); it != ipList.end(); ++it) {
                if (jlwe.getCurrentUserIP() == *it)
                    ipValid = true;
            }
        }
        if (!ipValid) {
            nlohmann::json errorJson;
            errorJson["error"] = "Invalid request IP: " + jlwe.getCurrentUserIP();
            std::cout << "Status:403 Forbidden\r\n";
            std::cout << JsonUtils::makeJsonHeader() + errorJson.dump();
            return 0;
        }

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << "Status:400 Bad Request\r\n";
            std::cout << JsonUtils::makeJsonError(postData.errorText());
            return 0;
        }

        std::vector<std::string> stripe_signature_header = JlweUtils::splitString(JlweUtils::replaceString(CgiEnvironment::getenvAsString("HTTP_STRIPE_SIGNATURE"), " ", ""), ',');

        std::string stripe_time_str = "";
        std::vector<std::string> signatures;
        for (unsigned int i = 0; i < stripe_signature_header.size(); i++) {
            std::string section = stripe_signature_header.at(i);
            if (section.substr(0, 2) == "t=") {
                stripe_time_str = section.substr(2);
            }
            if (section.substr(0, 3) == "v1=") {
                signatures.push_back(section.substr(3));
            }
        }

        // Block any requests with an invalid timestamp
        time_t stripe_time = 0;
        try {
            stripe_time = stol(stripe_time_str);
        } catch (...) {}
        if (labs(time(nullptr) - stripe_time) > 300) {
            std::cout << "Status:403 Forbidden\r\n";
            std::cout << JsonUtils::makeJsonError("Invalid timestamp");
            return 0;
        }

        std::string signed_payload = stripe_time_str + "." + postData.dataAsString();

        // Check signature, block if it's invalid
        nlohmann::json signingSecretList = jlwe.config.at("stripe").at("webhookSigningSecrets");
        bool signatureValid = false;
        if (signingSecretList.is_array()) {
            for (nlohmann::json::iterator it = signingSecretList.begin(); it != signingSecretList.end(); ++it) {
                std::string sha2hmac = hmac<SHA256>(signed_payload, *it);
                for (unsigned int i = 0; i < signatures.size(); i++) {
                    if (signatures.at(i).size() > 0 && signatures.at(i) == sha2hmac)
                        signatureValid = true;
                }
            }
        }
        if (!signatureValid) {
            std::cout << "Status:403 Forbidden\r\n";
            std::cout << JsonUtils::makeJsonError("Invalid Stripe signature header");
            return 0;
        }

        // If we get to here, the request is valid so start decoding it
        nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

        time_t created = jsonDocument.value("created", 0);
        bool livemode = jsonDocument.value("livemode", false);
        std::string id = jsonDocument.at("id");
        std::string type = jsonDocument.at("type");
        std::string api_version = jsonDocument.value("api_version", "");

        std::string cs_id = "";
        std::string payment_intent = "";
        int amount = 0;
        int amount_received = 0;
        std::string message = "";

        if (type == "checkout.session.completed") {
            if (jsonDocument.contains("data") && jsonDocument.at("data").is_object() && jsonDocument.at("data").contains("object") && jsonDocument.at("data").at("object").is_object()) {
                nlohmann::json dataObject = jsonDocument.at("data").at("object");

                if (dataObject.contains("id") && dataObject.at("id").is_string())
                    cs_id = dataObject.at("id");
                if (dataObject.contains("payment_intent") && dataObject.at("payment_intent").is_string())
                    payment_intent = dataObject.at("payment_intent");

            }
        }

        if (type.substr(0, 15) == "payment_intent.") {
            if (jsonDocument.contains("data") && jsonDocument.at("data").is_object() && jsonDocument.at("data").contains("object") && jsonDocument.at("data").at("object").is_object()) {
                nlohmann::json dataObject = jsonDocument.at("data").at("object");

                if (dataObject.contains("id") && dataObject.at("id").is_string())
                    payment_intent = dataObject.at("id");
                if (dataObject.contains("amount") && dataObject.at("amount").is_number())
                    amount = dataObject.at("amount");
                if (dataObject.contains("amount_received") && dataObject.at("amount_received").is_number())
                    amount_received = dataObject.at("amount_received");

                if (type == "payment_intent.canceled") {
                    if (dataObject.contains("cancellation_reason") && dataObject.at("cancellation_reason").is_string())
                        message = "cancellation_reason=" + std::string(dataObject.at("cancellation_reason"));
                }

                if (type == "payment_intent.payment_failed") {
                    if (dataObject.contains("last_payment_error") && dataObject.at("last_payment_error").is_object())
                        if (dataObject.at("last_payment_error").contains("message") && dataObject.at("last_payment_error").at("message").is_string())
                            message = "payment_error=" + std::string(dataObject.at("last_payment_error").at("message"));
                }
            }
        }

        if (type == "charge.refunded") {
            if (jsonDocument.contains("data") && jsonDocument.at("data").is_object() && jsonDocument.at("data").contains("object") && jsonDocument.at("data").at("object").is_object()) {
                nlohmann::json dataObject = jsonDocument.at("data").at("object");

                if (dataObject.contains("amount") && dataObject.at("amount").is_number())
                    amount = dataObject.at("amount");
                if (dataObject.contains("amount_refunded") && dataObject.at("amount_refunded").is_number())
                    amount_received = -1 * static_cast<int>(dataObject.at("amount_refunded"));
                if (dataObject.contains("payment_intent") && dataObject.at("payment_intent").is_string())
                    payment_intent = dataObject.at("payment_intent");
            }
        }

        // Store the data received in the database
        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT insertStripeEvent(?,?,?,?,?,?,?,?,?,?);");
        prep_stmt->setString(1, id);
        prep_stmt->setInt64(2, created);
        prep_stmt->setInt(3, livemode);
        prep_stmt->setString(4, type);
        prep_stmt->setString(5, api_version);
        prep_stmt->setString(6, payment_intent);
        prep_stmt->setString(7, cs_id);
        prep_stmt->setInt(8, amount);
        prep_stmt->setInt(9, amount_received);
        prep_stmt->setString(10, message);
        res = prep_stmt->executeQuery();
        if (res->next()){
            if (res->getInt(1) == 0){
                std::cout << JsonUtils::makeJsonSuccess("Webhook event already received");
            } else {
                std::cout << JsonUtils::makeJsonSuccess("Webhook data recorded");
            }
        } else {
            std::cout << "Status:500 Internal Server Error\r\n";
            std::cout << JsonUtils::makeJsonError("Error executing MySQL query");
        }
        delete res;
        delete prep_stmt;

    } catch (sql::SQLException &e) {
        std::cout << "Status:500 Internal Server Error\r\n";
        std::cout << JsonUtils::makeJsonError(std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")");
    } catch (const std::exception &e) {
        std::cout << "Status:500 Internal Server Error\r\n";
        std::cout << JsonUtils::makeJsonError(std::string(e.what()));
    }

    return 0;
}
