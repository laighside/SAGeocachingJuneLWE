/**
  @file    JlweHtmlEmail.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A subclass of Email.h, used for building and sending emails with "JLWE theme" formatting
  These emails are HTML with the JLWE logo, green border and content style that matches the website

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef JLWEHTMLEMAIL_H
#define JLWEHTMLEMAIL_H

#include <string>

#include <cppconn/connection.h>

#include "../core/JlweCore.h"
#include "Email.h"

class JlweHtmlEmail : public Email {

public:
    /*!
     * \brief JlweHtmlEmail Constructor.
     */
    JlweHtmlEmail();

    /*!
     * \brief JlweHtmlEmail Destructor.
     */
    ~JlweHtmlEmail();

    /*!
     * \brief Appends some arbitrary HTML to the content of the email
     *
     * \param html The HTML to append to the email content
     */
    void addHtml(const std::string &html);

    /*!
     * \brief Adds a title to the email in the form "Merchandise order for <username>"
     *
     * \param username The username to put in the title
     */
    void addMerchTitle(const std::string &username);

    /*!
     * \brief Adds a title to the email in the form "Invoice for <username>"
     *
     * \param username The username to put in the title
     */
    void addInvoiceTitle(const std::string &username);

    /*!
     * \brief Adds the username/email/phone details to the email
     *
     * \param username The username to display
     * \param email The email address to display
     * \param phone The phone number to display
     */
    void addRegistrationDetails(const std::string &username, const std::string &email, const std::string &phone);

    /*!
     * \brief Adds a table to the email with order details and cost for event registration, camping and dinner
     *
     * \param userKey The key of the order to display details for
     * \param con Pointer to MySQL connection object
     */
    void addCostTable(std::string userKey, sql::Connection *con);

    /*!
     * \brief Adds a table to the email with order details and item costs for a merch order
     *
     * \param userKey The key of the order to display details for
     * \param con Pointer to MySQL connection object
     */
    void addMerchOrderTable(std::string userKey, sql::Connection *con);

    /*!
     * \brief Adds a note to the email asking the recipient to pay by bank transfer
     *
     * \param payment_total The amount the email recipient needs to pay
     * \param jlwe Pointer to JlweCore object
     * \param merch_order Set to true if this is this a merch order email
     */
    void addBankDetails(std::string payment_total, JlweCore *jlwe, bool merch_order);

    /*!
     * \brief Adds a note to the email asking the recipient to pay by cash at the event
     *
     * \param payment_total The amount the email recipient needs to pay
     */
    void addCashPayment(std::string payment_total);

    /*!
     * \brief Adds a note to the email thanking the recipient for paying by card
     *
     * \param payment_total The amount the recipient paid
     * \param timestamp The time the payment was received
     */
    void addCardPayment(std::string payment_received, time_t timestamp);

    /*!
     * \brief Sends the email (with "JLWE theme" formatting)
     *
     * \param to_address The email address to send the email to
     * \param reply_address The email address to put in the Reply-To header
     * \param subject The subject line of the email (for the Subject header)
     * \param mailer_address The email address to put in the From header
     * \param from_name The name of the email sender
     * \return Returns 0 if the email was sent successfully(*)
     */
    int sendJlweEmail(std::string to_address, std::string reply_address, std::string subject, std::string mailer_address, std::string from_name = "");


private:

    /*!
     * \brief Adds the template header and footer to the HTML
     *
     * This is called by sendJlweEmail() before the email is sent
     *
     * \param logo_url The src value for the logo img element at the top of the email
     */
    void setHtmlInTemplate(std::string logo_url);

    std::string inner_html;

};

#endif // JLWEHTMLEMAIL_H
