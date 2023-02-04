/**
  @file    EmailTemplates.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions that are used for email related things
  All functions are static so there is no need to create instances of the EmailTemplates object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef EMAILTEMPLATES_H
#define EMAILTEMPLATES_H

#include <string>

#include "../core/JlweCore.h"

// URL for requests to verify addresses and unsubscribe from the mailing list
#define MANAGE_SUBSCRIPTION_URL "/cgi-bin/mailing_list/manage_subscription.cgi"

class EmailTemplates {

public:

    /*!
     * \brief Sends the email users receive when they signup to the mailing list
     *
     * This email asks them to click on a link to verify their address
     * The option to unsubscribe is also provided
     *
     * \param email_address The email address to send the email to
     * \param token_verify The verification token for the user
     * \param token_unsub The unsubscribe token for the user
     * \param jlwe Pointer to JlweCore object (for access to website config file)
     * \return Returns 0 if the email was sent successfully(*)
     */
    static int sendMailingListSignupEmail(const std::string &email_address, const std::string &token_verify, const std::string &token_unsub, JlweCore *jlwe);

    /*!
     * \brief Adds an email address to the mailing list and sends a signup verification email
     *
     * \param email The email address to add to the mailing list
     * \param jlwe Pointer to JlweCore object
     * \param error Pointer to string that, if an error occurs, will be filled with description of the error
     * \return Returns 0 if the email was sent successfully(*)
     */
    static int addEmailtoMailingList(const std::string &email, JlweCore *jlwe, std::string *error = nullptr);

};

#endif // EMAILTEMPLATES_H
