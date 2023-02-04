/**
  @file    delete_cache.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/gpx_builder/delete_cache.cgi
  Deletes a given cache
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/PostDataParser.h"
#include "../core/JsonUtils.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << JsonUtils::makeJsonError(postData.errorText());
            return 0;
        }

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_gpxbuilder")) { //if logged in

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

                int id_number = jsonDocument.value("id_number", 0);
                int reviewCacheId = jsonDocument.value("reviewCacheId", 0);

                if (id_number) {
                    bool del = false;
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT * FROM caches WHERE cache_number = ?;");
                    prep_stmt->setInt(1, id_number);
                    res = prep_stmt->executeQuery();
                    if (res->next())
                        del = true;
                    delete res;
                    delete prep_stmt;
                    if (del) {
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT deleteCache(?,?,?);");
                        prep_stmt->setInt(1, id_number);
                        prep_stmt->setString(2, jlwe.getCurrentUserIP());
                        prep_stmt->setString(3, jlwe.getCurrentUsername());
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                            std::cout << JsonUtils::makeJsonSuccess("Cache " + std::to_string(id_number) + " deleted");
                        } else {
                            std::cout << JsonUtils::makeJsonError("Error deleting cache " + std::to_string(id_number));
                        }
                        delete res;
                        delete prep_stmt;
                    } else {
                        std::cout << JsonUtils::makeJsonError("Cache " + std::to_string(id_number) + " not found");
                    }
                } else if (reviewCacheId) {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setCacheStatus(?,?,?,?);");
                    prep_stmt->setInt(1, reviewCacheId);
                    prep_stmt->setString(2, "D");
                    prep_stmt->setString(3, jlwe.getCurrentUserIP());
                    prep_stmt->setString(4, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        if (res->getInt(1)) {
                            std::cout << JsonUtils::makeJsonError("Cache " + std::to_string(reviewCacheId) + " (review ID) not found");
                        } else {
                            std::cout << JsonUtils::makeJsonSuccess("Cache deleted");
                        }
                    } else {
                        std::cout << JsonUtils::makeJsonError("Error deleting cache " + std::to_string(reviewCacheId) + " (review ID)");
                    }
                    delete res;
                    delete prep_stmt;
                } else {
                    std::cout << JsonUtils::makeJsonError("Invalid cache number");
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
