/**
  @file    PaymentUtils.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions commonly used when displaying payment info
  All functions are static so there is no need to create instances of the PaymentUtils object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef PAYMENTUTILS_H
#define PAYMENTUTILS_H

#include <string>
#include <vector>

#include <cppconn/connection.h>

class PaymentUtils {
public:

    /*!
     * \brief Object to represent a single payment
     */
    struct paymentEntry{
        // unique id of the payment
        std::string id;

        // time the payment was received
        time_t timestamp;

        // value of the payment (in cents)
        int payment_amount;

        // type of payment (cash, card or bank)
        std::string payment_type;
    };

    /*!
     * \brief Converts a currency value to human readable form
     *
     * eg. 245 gets converted to $2.45
     *
     * \param amount The value in cents
     * \return The string to display to the user
     */
    static inline std::string currencyToString(int amount) {
        char buffer [50];
        sprintf (buffer, "%1.2f", (static_cast<double>(amount) / 100));
        return "$" + std::string(buffer);
    }

    /*!
     * \brief Gets a list of all payments received from a given user
     *
     * \param con MySQL connection object
     * \param userKey The key of the user to get payment info for
     * \return The list of payments
     */
    static std::vector<paymentEntry> getUserPayments(sql::Connection *con, const std::string &userKey);

    /*!
     * \brief Gets the total value of all payments from a table of payments
     *
     * \param table A vector that contains a list of payments to add up
     * \return The total value (in cents)
     */
    static int getTotalPaymentReceived(std::vector<paymentEntry> *table);

    /*!
     * \brief Gets the total value of all payments received from a given user
     *
     * \param con MySQL connection object
     * \param userKey The key of the user to get payment info for
     * \return The total value (in cents)
     */
    static int getTotalPaymentReceived(sql::Connection *con, const std::string &userKey);

    /*!
     * \brief Gets the total cost of all items a given user has ordered
     *
     * \param con MySQL connection object
     * \param userKey The key of the user to get payment info for
     * \return The total cost (in cents)
     */
    static int getUserCost(sql::Connection *con, const std::string &userKey);

    /*!
     * \brief Gets the type of registration for a user (event, camping_only, dinner_only or merch)
     *
     * \param con MySQL connection object
     * \param userKey The key of the user to get info for
     * \return The type of registration (event, camping_only, dinner_only or merch)
     */
    static std::string getRegistrationType(sql::Connection *con, const std::string &userKey);

    /*!
     * \brief Gets the email a user supplied for their order
     *
     * \param con MySQL connection object
     * \param userKey The key of the user to get info for
     * \return The user's email
     */
    static std::string getUserEmail(sql::Connection *con, const std::string &userKey);

    /*!
     * \brief Gets the Geocaching username a user supplied for their order
     *
     * \param con MySQL connection object
     * \param userKey The key of the user to get info for
     * \return The user's Geocaching username
     */
    static std::string getUserName(sql::Connection *con, const std::string &userKey);

    /*!
     * \brief Gets the Stripe card fee for a given order/user
     *
     * \param con MySQL connection object
     * \param userKey The key of the user to get info for
     * \return The total card fees for the order
     */
    static int getCardPaymentFees(sql::Connection *con, const std::string &userKey);

};

#endif // PAYMENTFUNCTIONS_H
