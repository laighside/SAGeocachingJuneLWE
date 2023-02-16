/**
  @file    JlweCore.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A class that performs the core functions of every HTTP request
   - Loads the jlwe.json config file
   - Connect to the MySQL database
   - Reads the cookies and finds if the user is logged in
   - Gets the permissions for the current user

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef JLWECORE_H
#define JLWECORE_H

#include <string>
#include <vector>

#include <mysql_driver.h>
#include <mysql_connection.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "../ext/nlohmann/json.hpp"

class JlweCore {

public:
    /*!
     * \brief JLWE Constructor.
     *
     * This loads the config file, connects to MySQL and loads the user details
     */
    JlweCore();
    /*!
     * \brief JLWE Destructor.
     */
    ~JlweCore();

    // This object should never be copied
    // Use pointers if it needs to be passed to functions
    JlweCore( const JlweCore& ) = delete; // non construction-copyable
    JlweCore& operator=( const JlweCore& ) = delete; // non copyable

    /*!
     * \brief JSON config object.
     *
     * This holds the website configuration in JSON format
     * The path to the configuration file is defined in JlweCore.cpp
     */
    nlohmann::json config;

    /*!
     * \brief Get the MySQL connection object.
     *
     * \return The MySQL connection object
     */
    sql::Connection * getMysqlCon() const;

    /*!
     * \brief Gets a the value of a variable from the vars table in the JLWE database
     *
     * \param name The name of the variable
     * \return The value of the variable as a string
     */
    std::string getGlobalVar(const std::string &name) const;

    /*!
     * \brief Returns true if the user is logged in, false otherwise
     *
     * \return True if the user is logged in, false otherwise
     */
    bool isLoggedIn() const;

    /*!
     * \brief Gets the ID number of the user making the HTTP request
     *
     * \return The ID number of user, or -1 if they are not logged in
     */
    int getCurrentUserId() const;

    /*!
     * \brief Gets the IP address of the user making the HTTP request
     *
     * \return The IP address
     */
    std::string getCurrentUserIP() const;

    /*!
     * \brief Gets the username of the user making the HTTP request
     *
     * \return The username of user, or an empty string if they are not logged in
     */
    std::string getCurrentUsername() const;

    /*!
     * \brief Gets the email address of the user making the HTTP request
     *
     * \return The email address of user, or an empty string if they are not logged in
     */
    std::string getCurrentUserEmail() const;

    /*!
     * \brief Check if a user has a certain permission or not
     *
     * \param name The name of the permission
     * \return True if the user has the permission, false otherwise
     */
    bool getPermissionValue(const std::string &permissionName) const;

    /*!
     * \brief Returns the filename of the config file
     *
     * \return The filename
     */
    std::string getConfigFilename() const;

private:
    struct permission {
        std::string name;
        bool value;
    };

    sql::Connection *mysqlCon;
    bool m_isLoggedIn;
    int m_currentUserId;
    std::string m_currentUserIP;
    std::string m_currentUsername;
    std::string m_currentUserEmail;
    std::vector<permission> userPermissions;

    void connectToMysql();
    void loadUserDetails();

};

#endif // JLWECORE_H
