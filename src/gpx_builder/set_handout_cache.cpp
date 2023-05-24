/**
  @file    set_handout_cache.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/gpx_builder/set_handout_cache.cgi
  Sets the owner for a cache, or sets a cache as returned
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/JsonUtils.h"
#include "../core/KeyValueParser.h"
#include "../core/PostDataParser.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        if (jlwe.getPermissionValue("perm_gpxbuilder")) { //if logged in

            sql::PreparedStatement *prep_stmt;
            sql::ResultSet *res;

            PostDataParser postData(jlwe.config.at("maxPostSize"));
            if (postData.hasError()) {
                std::cout << JsonUtils::makeJsonError(postData.errorText());
                return 0;
            }

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            int cache_number = jsonDocument.at("cache_number");

            // link cache to owner (aka. set owner)
            if (jsonDocument.contains("linkCache") && jsonDocument.at("linkCache").is_object()) {
                std::string owner = jsonDocument.at("linkCache").value("owner", "");
                JlweUtils::trimString(owner);

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setHandoutCacheOwner(?,?,?,?);");
                prep_stmt->setInt(1, cache_number);
                prep_stmt->setString(2, owner);
                prep_stmt->setString(3, jlwe.getCurrentUserIP());
                prep_stmt->setString(4, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    nlohmann::json jsonDocumentOut;

                    jsonDocumentOut["success"] = true;
                    if (owner.size()) {
                        jsonDocumentOut["message"] = "Cache " + std::to_string(cache_number) + " owner set to " + owner;
                    } else {
                        jsonDocumentOut["message"] = "Cache " + std::to_string(cache_number) + " owner cleared";
                    }
                    jsonDocumentOut["cache_number"] = cache_number;
                    jsonDocumentOut["owner"] = owner;

                    std::cout << JsonUtils::makeJsonHeader() + jsonDocumentOut.dump();
                } else {
                    std::cout << JsonUtils::makeJsonError("Error writing to database");
                }
                delete res;
                delete prep_stmt;

            } else if (jsonDocument.contains("returnCache") && jsonDocument.at("returnCache").is_object()) {
                bool returned = jsonDocument.at("returnCache").value("returned", false);

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setHandoutCacheReturned(?,?,?,?);");
                prep_stmt->setInt(1, cache_number);
                prep_stmt->setInt(2, returned);
                prep_stmt->setString(3, jlwe.getCurrentUserIP());
                prep_stmt->setString(4, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    nlohmann::json jsonDocumentOut;

                    jsonDocumentOut["success"] = true;
                    jsonDocumentOut["message"] = "Cache " + std::to_string(cache_number) + " set to " + (returned ? "returned" : "not returned");
                    jsonDocumentOut["cache_number"] = cache_number;
                    jsonDocumentOut["returned"] = returned;

                    std::cout << JsonUtils::makeJsonHeader() + jsonDocumentOut.dump();
                } else {
                    std::cout << JsonUtils::makeJsonError("Error writing to database");
                }
                delete res;
                delete prep_stmt;

            } else {
                std::cout << JsonUtils::makeJsonError("Invalid request");
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
