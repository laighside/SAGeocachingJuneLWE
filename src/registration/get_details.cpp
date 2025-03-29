/**
  @file    get_details.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/registration/get_details.cgi?key=...
  Returns the details of a given registration
  GET requests, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"
#include "../core/KeyValueParser.h"
#include "../core/PaymentUtils.h"
#include "DinnerUtils.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;
        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_registrations")) { //if logged in

            std::string userKey = urlQueries.getValue("key");

            if (userKey.size()) {
                bool regFound = false;
                nlohmann::json jsonDocument;

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address,gc_username,phone_number,livemode,number_adults,number_children,past_jlwe FROM event_registrations WHERE idempotency = ?;");
                prep_stmt->setString(1, userKey);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    jsonDocument["email_address"] = res->getString(1);
                    jsonDocument["gc_username"] = res->getString(2);
                    jsonDocument["phone_number"] = res->getString(3);
                    jsonDocument["livemode"] = res->getInt(4);
                    jsonDocument["payment_total"] = PaymentUtils::getUserCost(jlwe.getMysqlCon(), userKey);
                    jsonDocument["payment_received"] = PaymentUtils::getTotalPaymentReceived(jlwe.getMysqlCon(), userKey);

                    jsonDocument["event"]["number_adult"] = res->getInt(5);
                    jsonDocument["event"]["number_child"] = res->getInt(6);
                    jsonDocument["event"]["past_jlwe"] = res->getInt(7);

                    regFound = true;
                }
                delete res;
                delete prep_stmt;

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address,gc_username,phone_number,livemode,camping_type,arrive_date,leave_date,number_people FROM camping WHERE idempotency = ?;");
                prep_stmt->setString(1, userKey);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    if (!regFound) {
                        jsonDocument["email_address"] = res->getString(1);
                        jsonDocument["gc_username"] = res->getString(2);
                        jsonDocument["phone_number"] = res->getString(3);
                        jsonDocument["livemode"] = res->getInt(4);
                        jsonDocument["payment_total"] = PaymentUtils::getUserCost(jlwe.getMysqlCon(), userKey);
                        jsonDocument["payment_received"] = PaymentUtils::getTotalPaymentReceived(jlwe.getMysqlCon(), userKey);
                    }

                    jsonDocument["camping"]["camping_type"] = res->getString(5);
                    jsonDocument["camping"]["arrive_date"] = res->getInt(6);
                    jsonDocument["camping"]["leave_date"] = res->getInt(7);
                    jsonDocument["camping"]["number_people"] = res->getInt(8);

                    regFound = true;
                }
                delete res;
                delete prep_stmt;

                jsonDocument["dinner"] = nlohmann::json::array();
                std::vector<DinnerUtils::dinner_form> dinner_forms = DinnerUtils::getDinnerFormList(jlwe.getMysqlCon());
                for (unsigned int i = 0; i < dinner_forms.size(); i++) {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address,gc_username,phone_number,livemode,number_adults,number_children FROM sat_dinner WHERE idempotency = ? AND dinner_form_id = ?;");
                    prep_stmt->setString(1, userKey);
                    prep_stmt->setInt(2, dinner_forms.at(i).dinner_id);
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        if (!regFound) {
                            jsonDocument["email_address"] = res->getString(1);
                            jsonDocument["gc_username"] = res->getString(2);
                            jsonDocument["phone_number"] = res->getString(3);
                            jsonDocument["livemode"] = res->getInt(4);
                            jsonDocument["payment_total"] = PaymentUtils::getUserCost(jlwe.getMysqlCon(), userKey);
                            jsonDocument["payment_received"] = PaymentUtils::getTotalPaymentReceived(jlwe.getMysqlCon(), userKey);
                        }

                        nlohmann::json dinnerObject;
                        dinnerObject["dinner_id"] = dinner_forms.at(i).dinner_id;
                        dinnerObject["title"] = dinner_forms.at(i).title;
                        dinnerObject["meals"] = res->getInt(5) + res->getInt(6);
                        jsonDocument["dinner"].push_back(dinnerObject);

                        regFound = true;
                    }
                    delete res;
                    delete prep_stmt;
                }

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_address,gc_username,phone_number,livemode,(SELECT COUNT(*) FROM merch_order_items WHERE merch_order_items.order_id=merch_orders.order_id) FROM merch_orders WHERE idempotency = ?;");
                prep_stmt->setString(1, userKey);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    if (!regFound) {
                        jsonDocument["email_address"] = res->getString(1);
                        jsonDocument["gc_username"] = res->getString(2);
                        jsonDocument["phone_number"] = res->getString(3);
                        jsonDocument["livemode"] = res->getInt(4);
                        jsonDocument["payment_total"] = PaymentUtils::getUserCost(jlwe.getMysqlCon(), userKey);
                        jsonDocument["payment_received"] = PaymentUtils::getTotalPaymentReceived(jlwe.getMysqlCon(), userKey);
                    }

                    jsonDocument["merch"]["item_count"] = res->getInt(5);

                    regFound = true;
                }
                delete res;
                delete prep_stmt;

                if (regFound) {
                    std::cout << JsonUtils::makeJsonHeader() + jsonDocument.dump();
                } else {
                    std::cout << JsonUtils::makeJsonError("Invalid Key");
                }
            } else {
                std::cout << JsonUtils::makeJsonError("Invalid Key");
            }
        } else {
            std::cout << JsonUtils::makeJsonError("You do not have permission to view this area");
        }
    } catch (sql::SQLException &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")");
    } catch( const std::exception &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()));
    }

    return 0;
}
