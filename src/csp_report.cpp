/**
  @file    csp_report.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  /cgi-bin/csp_report.cgi
  Handles any CSP reports that are sent to the server

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "core/CgiEnvironment.h"
#include "core/PostDataParser.h"
#include "core/JlweCore.h"
#include "core/JsonUtils.h"

#include "ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (!postData.hasError()) {

            sql::PreparedStatement *prep_stmt;
            sql::ResultSet *res;

            // parse the JSON to make sure it is valid JSON
            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            std::string user_agent = CgiEnvironment::getUserAgent();

            // save the CSP report in the database
            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT addCSPReport(?,?,?);");
            prep_stmt->setString(1, user_agent);
            prep_stmt->setString(2, jsonDocument.dump());
            prep_stmt->setString(3, jlwe.getCurrentUserIP());
            res = prep_stmt->executeQuery();
            if (res->next()){
            }
            delete res;
            delete prep_stmt;
        }

    // ignore any errors
    } catch (sql::SQLException &e) {
    } catch (const std::exception &e) {
    }

    std::cout << JsonUtils::makeJsonSuccess("Report received");
    return 0;
}
