/**
  @file    JlweCore.cpp
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
#include "JlweCore.h"

#include <stdexcept>

#include "CgiEnvironment.h"
#include "KeyValueParser.h"
#include "JlweUtils.h"

// This is where the configuration file is stored
#ifndef CONFIG_FILE
#define CONFIG_FILE  "/etc/jlwe/jlwe.json"
#endif

JlweCore::JlweCore() {
    this->mysqlCon = nullptr;
    this->m_isLoggedIn = false;
    this->m_currentUserId = -1;
    this->m_currentUserIP = CgiEnvironment::getRemoteAddr();
    this->m_currentUsername = "";
    this->m_currentUserEmail = "";

    // Load configuration file, ignoring comments in it
    this->config = nlohmann::json::parse(JlweUtils::readFileToString(CONFIG_FILE), nullptr, true, true);

    // Connect to MySQL database
    this->connectToMysql();

    // Check if the user is logged in, and get their details if they are
    this->loadUserDetails();
}

JlweCore::~JlweCore() {
    if (this->mysqlCon)
        delete this->mysqlCon;
}

void JlweCore::connectToMysql() {
    sql::Driver *driver = get_driver_instance();
    this->mysqlCon = driver->connect(std::string(this->config.at("mysql").value("host", "localhost")).c_str(), std::string(this->config.at("mysql").at("username")).c_str(), std::string(this->config.at("mysql").value("password", "")).c_str());
    this->mysqlCon->setSchema(std::string(this->config.at("mysql").at("database")).c_str());
}

sql::Connection * JlweCore::getMysqlCon() const {
    if (this->mysqlCon == nullptr)
        throw std::runtime_error("Call to getMysqlCon() but not connected to MySQL database");
    return this->mysqlCon;
}

void JlweCore::loadUserDetails() {
    KeyValueParser cookies(CgiEnvironment::getCookies());
    std::string accessToken = cookies.getValue("accessToken");

    if (!accessToken.size()) // if there is no cookie then we know the user isn't logged in
        return;

    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;
    std::string username = "";
    prep_stmt = this->mysqlCon->prepareStatement("SELECT username FROM user_tokens WHERE token = ?;"); // AND ip_address = ?
    prep_stmt->setString(1, accessToken);
    //prep_stmt->setString(2, userIP);
    res = prep_stmt->executeQuery();
    if (res->next()){
        username = res->getString(1);
    }
    delete res;
    delete prep_stmt;

    if (!username.size()) // not logged in
        return;

    prep_stmt = this->mysqlCon->prepareStatement("SELECT user_id,username,email FROM users WHERE username = ? AND active = 1;");
    prep_stmt->setString(1, username);
    res = prep_stmt->executeQuery();
    if (res->next()){
        this->m_isLoggedIn = true;
        this->m_currentUserId = res->getInt(1);
        this->m_currentUsername = res->getString(2);
        this->m_currentUserEmail = res->getString(3);
    }
    delete res;
    delete prep_stmt;

    if (this->m_isLoggedIn) {
        prep_stmt = this->mysqlCon->prepareStatement("SELECT permission FROM user_permissions WHERE user = ? AND value = 1;");
        prep_stmt->setString(1, this->m_currentUsername);
        res = prep_stmt->executeQuery();
        while (res->next()){
            this->userPermissions.push_back({res->getString(1), true});
        }
        delete res;
        delete prep_stmt;
    }

}

bool JlweCore::isLoggedIn() const {
    return this->m_isLoggedIn;
}

int JlweCore::getCurrentUserId() const {
    return this->m_currentUserId;
}

std::string JlweCore::getCurrentUserIP() const {
    return this->m_currentUserIP;
}

std::string JlweCore::getCurrentUsername() const {
    return this->m_currentUsername;
}

std::string JlweCore::getCurrentUserEmail() const {
    return this->m_currentUserEmail;
}

bool JlweCore::getPermissionValue(const std::string &permissionName) const {
    for (size_t i = 0; i < this->userPermissions.size(); i++) {
        if (this->userPermissions.at(i).name == permissionName)
            return this->userPermissions.at(i).value;
    }
    return false;
}

std::string JlweCore::getGlobalVar(const std::string& name) const {
    std::string result = "";
    sql::PreparedStatement *prep_stmt = this->mysqlCon->prepareStatement("SELECT value FROM vars WHERE name = ?;");
    prep_stmt->setString(1, name);
    sql::ResultSet *res = prep_stmt->executeQuery();
    if (res->next()){
        result = res->getString(1);
    }
    delete res;
    delete prep_stmt;
    return result;
}
