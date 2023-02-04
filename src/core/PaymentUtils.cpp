/**
  @file    PaymentUtils.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions commonly used when displaying payment info
  All functions are static so there is no need to create instances of the PaymentUtils object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "PaymentUtils.h"

#include "../prices.h"

#include <mysql_driver.h>
#include <mysql_connection.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>


std::vector<PaymentUtils::paymentEntry> PaymentUtils::getUserPayments(sql::Connection *con, const std::string &userKey) {
    std::vector<paymentEntry> result;
    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;

    //get bank/cash payments
    prep_stmt = con->prepareStatement("SELECT id, timestamp, amount_received, payment_type FROM payment_log WHERE user_key = ?;");
    prep_stmt->setString(1, userKey);
    res = prep_stmt->executeQuery();
    while (res->next()){
        paymentEntry entry;
        entry.id = res->getString(1);
        entry.timestamp = res->getInt64(2);
        entry.payment_amount = res->getInt(3);
        entry.payment_type = res->getString(4);
        result.push_back(entry);
    }
    delete res;
    delete prep_stmt;

    //get card payments
    std::string cs_id = "";
    prep_stmt = con->prepareStatement("SELECT stripe_session_id FROM event_registrations WHERE idempotency = ? UNION SELECT stripe_session_id FROM camping WHERE idempotency = ? UNION SELECT stripe_session_id FROM sat_dinner WHERE idempotency = ? UNION SELECT stripe_session_id FROM merch_orders WHERE idempotency = ?;");
    prep_stmt->setString(1, userKey);
    prep_stmt->setString(2, userKey);
    prep_stmt->setString(3, userKey);
    prep_stmt->setString(4, userKey);
    res = prep_stmt->executeQuery();
    if (res->next()){
        cs_id = res->getString(1);
    }
    delete res;
    delete prep_stmt;

    if (cs_id.size()) {
        std::string payment_intent = "";
        prep_stmt = con->prepareStatement("SELECT payment_intent FROM stripe_event_log WHERE cs_id = ?;");
        prep_stmt->setString(1, cs_id);
        res = prep_stmt->executeQuery();
        if (res->next()){
            payment_intent = res->getString(1);
        }
        delete res;
        delete prep_stmt;

        if (payment_intent.size()) {

            prep_stmt = con->prepareStatement("SELECT id, timestamp, amount_received FROM stripe_event_log WHERE payment_intent = ? AND (type = 'payment_intent.succeeded' OR type = 'charge.refunded');");
            prep_stmt->setString(1, payment_intent);
            res = prep_stmt->executeQuery();
            while (res->next()){
                paymentEntry entry;
                entry.id = res->getString(1);
                entry.timestamp = res->getInt64(2);
                entry.payment_amount = res->getInt(3);
                entry.payment_type = "card";
                result.push_back(entry);
            }
            delete res;
            delete prep_stmt;
        }
    }

    return result;
}


int PaymentUtils::getTotalPaymentReceived(sql::Connection *con, const std::string &userKey) {
    std::vector<paymentEntry> table = getUserPayments(con, userKey);
    return getTotalPaymentReceived(&table);
}

int PaymentUtils::getTotalPaymentReceived(std::vector<PaymentUtils::paymentEntry> *table) {
    int total = 0;
    for (unsigned int i = 0; i < table->size(); i++) {
        total += table->at(i).payment_amount;
    }
    return total;
}

int PaymentUtils::getUserCost(sql::Connection *con, const std::string &userKey) {
    int result = 0;
    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;
    prep_stmt = con->prepareStatement("SELECT number_adults, number_children FROM event_registrations WHERE idempotency = ?;");
    prep_stmt->setString(1, userKey);
    res = prep_stmt->executeQuery();
    if (res->next()){
        result = res->getInt(1) * PRICE_EVENT_ADULT + res->getInt(2) * PRICE_EVENT_CHILD;
    }
    delete res;
    delete prep_stmt;

    prep_stmt = con->prepareStatement("SELECT number_people, camping_type, (leave_date - arrive_date) FROM camping WHERE idempotency = ?;");
    prep_stmt->setString(1, userKey);
    res = prep_stmt->executeQuery();
    if (res->next()){
        result += getCampingPrice(res->getString(2), res->getInt(1), res->getInt(3));
    }
    delete res;
    delete prep_stmt;

    // dinner costs
    prep_stmt = con->prepareStatement("SELECT number_adults, number_children FROM sat_dinner WHERE idempotency = ?;");
    prep_stmt->setString(1, userKey);
    res = prep_stmt->executeQuery();
    if (res->next()){
        result += res->getInt(1) * PRICE_DINNER_ADULT + res->getInt(2) * PRICE_DINNER_CHILD;
    }
    delete res;
    delete prep_stmt;

    // merch costs
    prep_stmt = con->prepareStatement("SELECT SUM(merch_items.cost) FROM merch_items INNER JOIN merch_order_items ON merch_items.id=merch_order_items.item_id INNER JOIN merch_orders ON merch_orders.order_id=merch_order_items.order_id WHERE merch_orders.idempotency = ?;");
    prep_stmt->setString(1, userKey);
    res = prep_stmt->executeQuery();
    if (res->next()){
        result += res->getInt(1);
    }
    delete res;
    delete prep_stmt;

    // card payment fees
    prep_stmt = con->prepareStatement("SELECT fee FROM stripe_card_fees WHERE idempotency = ?;");
    prep_stmt->setString(1, userKey);
    res = prep_stmt->executeQuery();
    if (res->next()){
        result += res->getInt(1);
    }
    delete res;
    delete prep_stmt;

    return result;
}

std::string PaymentUtils::getRegistrationType(sql::Connection *con, const std::string &userKey) {
    std::string result = "";
    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;

    prep_stmt = con->prepareStatement("SELECT gc_username FROM event_registrations WHERE idempotency = ?;");
    prep_stmt->setString(1, userKey);
    res = prep_stmt->executeQuery();
    if (res->next()){
        result = "event";
    }
    delete res;
    delete prep_stmt;

    if (!result.size()) {
        prep_stmt = con->prepareStatement("SELECT gc_username FROM camping WHERE idempotency = ?;");
        prep_stmt->setString(1, userKey);
        res = prep_stmt->executeQuery();
        if (res->next()){
            result = "camping_only";
        }
        delete res;
        delete prep_stmt;
    }

    if (!result.size()) {
        prep_stmt = con->prepareStatement("SELECT gc_username FROM sat_dinner WHERE idempotency = ?;");
        prep_stmt->setString(1, userKey);
        res = prep_stmt->executeQuery();
        if (res->next()){
            result = "dinner_only";
        }
        delete res;
        delete prep_stmt;
    }

    if (!result.size()) {
        prep_stmt = con->prepareStatement("SELECT gc_username FROM merch_orders WHERE idempotency = ?;");
        prep_stmt->setString(1, userKey);
        res = prep_stmt->executeQuery();
        if (res->next()){
            result = "merch";
        }
        delete res;
        delete prep_stmt;
    }

    return result;
}

std::string PaymentUtils::getUserEmail(sql::Connection *con, const std::string &userKey) {
    std::string result = "";
    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;

    prep_stmt = con->prepareStatement("SELECT email_address FROM event_registrations WHERE idempotency = ? UNION SELECT email_address FROM camping WHERE idempotency = ? UNION SELECT email_address FROM sat_dinner WHERE idempotency = ? UNION SELECT email_address FROM merch_orders WHERE idempotency = ?;");
    prep_stmt->setString(1, userKey);
    prep_stmt->setString(2, userKey);
    prep_stmt->setString(3, userKey);
    prep_stmt->setString(4, userKey);
    res = prep_stmt->executeQuery();
    if (res->next()){
        result = res->getString(1);
    }
    delete res;
    delete prep_stmt;

    return result;
}


std::string PaymentUtils::getUserName(sql::Connection *con, const std::string &userKey) {
    std::string result = "";
    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;

    prep_stmt = con->prepareStatement("SELECT gc_username FROM event_registrations WHERE idempotency = ? UNION SELECT gc_username FROM camping WHERE idempotency = ? UNION SELECT gc_username FROM sat_dinner WHERE idempotency = ? UNION SELECT gc_username FROM merch_orders WHERE idempotency = ?;");
    prep_stmt->setString(1, userKey);
    prep_stmt->setString(2, userKey);
    prep_stmt->setString(3, userKey);
    prep_stmt->setString(4, userKey);
    res = prep_stmt->executeQuery();
    if (res->next()){
        result = res->getString(1);
    }
    delete res;
    delete prep_stmt;

    return result;
}
