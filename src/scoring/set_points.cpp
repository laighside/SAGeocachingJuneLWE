/**
  @file    set_points.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/scoring/set_points.cgi
  Sets the point values for caches.
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"
#include "../core/PostDataParser.h"

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

        if (jlwe.getPermissionValue("perm_pptbuilder")) { //if logged in

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            std::string result = JsonUtils::makeJsonError("Something has gone wrong");

            std::string points_type = jsonDocument.at("type");

            if (points_type == "find_trads") {

                int item_id = jsonDocument.at("id");

                if (jsonDocument.contains("enabled")) {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setFindPointsTradsEnabled(?,?,?,?);");
                    prep_stmt->setInt(1, item_id);
                    prep_stmt->setInt(2, jsonDocument.at("enabled") ? 1 : 0);
                    prep_stmt->setString(3, jlwe.getCurrentUserIP());
                    prep_stmt->setString(4, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        if (res->getInt(1) == 0) {
                            result = JsonUtils::makeJsonSuccess("Enabled updated");
                        } else {
                            result = JsonUtils::makeJsonError("ID not found");
                        }
                    } else {
                        result = JsonUtils::makeJsonError("Unable to execute query");
                    }
                    delete res;
                    delete prep_stmt;
                }

                if (jsonDocument.contains("hide_or_find")) {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setFindPointsTradsType(?,?,?,?);");
                    prep_stmt->setInt(1, item_id);
                    prep_stmt->setString(2, std::string(jsonDocument.at("hide_or_find")).substr(0,1));
                    prep_stmt->setString(3, jlwe.getCurrentUserIP());
                    prep_stmt->setString(4, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        if (res->getInt(1) == 0) {
                            result = JsonUtils::makeJsonSuccess("Type updated");
                        } else {
                            result = JsonUtils::makeJsonError("ID not found");
                        }
                    } else {
                        result = JsonUtils::makeJsonError("Unable to execute query");
                    }
                    delete res;
                    delete prep_stmt;
                }

                if (jsonDocument.contains("config")) {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setFindPointsTradsConfig(?,?,?,?);");
                    prep_stmt->setInt(1, item_id);
                    prep_stmt->setString(2, std::string(jsonDocument.at("config").dump()));
                    prep_stmt->setString(3, jlwe.getCurrentUserIP());
                    prep_stmt->setString(4, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        if (res->getInt(1) == 0) {
                            result = JsonUtils::makeJsonSuccess("Config updated");
                        } else {
                            result = JsonUtils::makeJsonError("ID not found");
                        }
                    } else {
                        result = JsonUtils::makeJsonError("Unable to execute query");
                    }
                    delete res;
                    delete prep_stmt;
                }

            }

            if (points_type == "find_extras") {

                if (jsonDocument.contains("new") && jsonDocument.at("new") == true) {
                    // Create new item
                    int new_id = 0;
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT addFindPointsExtrasItem(?,?);");
                    prep_stmt->setString(1, jlwe.getCurrentUserIP());
                    prep_stmt->setString(2, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        new_id = res->getInt(1);
                    } else {
                        result = JsonUtils::makeJsonError("Unable to execute query");
                    }
                    delete res;
                    delete prep_stmt;

                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT id, name, point_value, enabled FROM game_find_points_extras WHERE id = ?;");
                    prep_stmt->setInt(1, new_id);
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        nlohmann::json jsonObject;
                        jsonObject["id"] = res->getInt(1);
                        jsonObject["name"] = res->getString(2);
                        jsonObject["point_value"] = res->getInt(3);
                        jsonObject["enabled"] = (res->getInt(4) != 0);
                        jsonObject["success"] = true;
                        result = JsonUtils::makeJsonHeader() + jsonObject.dump();
                    } else {
                        result = JsonUtils::makeJsonError("Unable to create new item");
                    }
                    delete res;
                    delete prep_stmt;

                } else {
                    // Edit an existing item
                    int item_id = jsonDocument.at("id");

                    if (jsonDocument.contains("enabled")) {
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setFindPointsExtrasEnabled(?,?,?,?);");
                        prep_stmt->setInt(1, item_id);
                        prep_stmt->setInt(2, jsonDocument.at("enabled") ? 1 : 0);
                        prep_stmt->setString(3, jlwe.getCurrentUserIP());
                        prep_stmt->setString(4, jlwe.getCurrentUsername());
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                            if (res->getInt(1) == 0) {
                                result = JsonUtils::makeJsonSuccess("Enabled updated");
                            } else {
                                result = JsonUtils::makeJsonError("ID not found");
                            }
                        } else {
                            result = JsonUtils::makeJsonError("Unable to execute query");
                        }
                        delete res;
                        delete prep_stmt;
                    }

                    if (jsonDocument.contains("name")) {
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setFindPointsExtrasName(?,?,?,?);");
                        prep_stmt->setInt(1, item_id);
                        prep_stmt->setString(2, std::string(jsonDocument.at("name")));
                        prep_stmt->setString(3, jlwe.getCurrentUserIP());
                        prep_stmt->setString(4, jlwe.getCurrentUsername());
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                            if (res->getInt(1) == 0) {
                                result = JsonUtils::makeJsonSuccess("Name updated");
                            } else {
                                result = JsonUtils::makeJsonError("ID not found");
                            }
                        } else {
                            result = JsonUtils::makeJsonError("Unable to execute query");
                        }
                        delete res;
                        delete prep_stmt;
                    }

                    if (jsonDocument.contains("point_value")) {
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setFindPointsExtrasPoints(?,?,?,?);");
                        prep_stmt->setInt(1, item_id);
                        prep_stmt->setInt(2, jsonDocument.at("point_value"));
                        prep_stmt->setString(3, jlwe.getCurrentUserIP());
                        prep_stmt->setString(4, jlwe.getCurrentUsername());
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                            if (res->getInt(1) == 0) {
                                result = JsonUtils::makeJsonSuccess("Point value updated");
                            } else {
                                result = JsonUtils::makeJsonError("ID not found");
                            }
                        } else {
                            result = JsonUtils::makeJsonError("Unable to execute query");
                        }
                        delete res;
                        delete prep_stmt;
                    }

                    if (jsonDocument.contains("delete") && jsonDocument.at("delete") == true) {
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT deleteFindPointsExtrasItem(?,?,?);");
                        prep_stmt->setInt(1, item_id);
                        prep_stmt->setString(2, jlwe.getCurrentUserIP());
                        prep_stmt->setString(3, jlwe.getCurrentUsername());
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                            if (res->getInt(1) == 0) {
                                result = JsonUtils::makeJsonSuccess("Item deleted");
                            } else {
                                result = JsonUtils::makeJsonError("ID not found");
                            }
                        } else {
                            result = JsonUtils::makeJsonError("Unable to execute query");
                        }
                        delete res;
                        delete prep_stmt;
                    }
                }
            }


            if (points_type == "zone") {

                if (jsonDocument.contains("new") && jsonDocument.at("new") == true) {
                    // Create new item
                    int new_id = 0;
                    std::string kml_file = jsonDocument.at("kml_file");
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT addZone(?,?,?);");
                    prep_stmt->setString(1, kml_file);
                    prep_stmt->setString(2, jlwe.getCurrentUserIP());
                    prep_stmt->setString(3, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        new_id = res->getInt(1);
                    } else {
                        result = JsonUtils::makeJsonError("Unable to execute query");
                    }
                    delete res;
                    delete prep_stmt;

                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT id, kml_file, name, points, enabled FROM zones WHERE id = ?;");
                    prep_stmt->setInt(1, new_id);
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        nlohmann::json jsonObject;
                        jsonObject["id"] = res->getInt(1);
                        jsonObject["kml_file"] = res->getString(2);
                        jsonObject["name"] = res->getString(3);
                        jsonObject["points"] = res->getInt(4);
                        jsonObject["enabled"] = (res->getInt(5) != 0);
                        jsonObject["success"] = true;
                        result = JsonUtils::makeJsonHeader() + jsonObject.dump();
                    } else {
                        result = JsonUtils::makeJsonError("Unable to create new item");
                    }
                    delete res;
                    delete prep_stmt;

                } else {
                    // Edit an existing item
                    int item_id = jsonDocument.at("id");

                    if (jsonDocument.contains("name")) {
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setZoneName(?,?,?,?);");
                        prep_stmt->setInt(1, item_id);
                        prep_stmt->setString(2, std::string(jsonDocument.at("name")));
                        prep_stmt->setString(3, jlwe.getCurrentUserIP());
                        prep_stmt->setString(4, jlwe.getCurrentUsername());
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                            if (res->getInt(1) == 0) {
                                result = JsonUtils::makeJsonSuccess("Name updated");
                            } else {
                                result = JsonUtils::makeJsonError("ID not found");
                            }
                        } else {
                            result = JsonUtils::makeJsonError("Unable to execute query");
                        }
                        delete res;
                        delete prep_stmt;
                    }

                    if (jsonDocument.contains("point_value")) {
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setZonePoints(?,?,?,?);");
                        prep_stmt->setInt(1, item_id);
                        prep_stmt->setInt(2, jsonDocument.at("point_value"));
                        prep_stmt->setString(3, jlwe.getCurrentUserIP());
                        prep_stmt->setString(4, jlwe.getCurrentUsername());
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                            if (res->getInt(1) == 0) {
                                result = JsonUtils::makeJsonSuccess("Point value updated");
                            } else {
                                result = JsonUtils::makeJsonError("ID not found");
                            }
                        } else {
                            result = JsonUtils::makeJsonError("Unable to execute query");
                        }
                        delete res;
                        delete prep_stmt;
                    }

                    if (jsonDocument.contains("delete") && jsonDocument.at("delete") == true) {
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT deleteZone(?,?,?);");
                        prep_stmt->setInt(1, item_id);
                        prep_stmt->setString(2, jlwe.getCurrentUserIP());
                        prep_stmt->setString(3, jlwe.getCurrentUsername());
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                            if (res->getInt(1) == 0) {
                                result = JsonUtils::makeJsonSuccess("Zone deleted");
                            } else {
                                result = JsonUtils::makeJsonError("ID not found");
                            }
                        } else {
                            result = JsonUtils::makeJsonError("Unable to execute query");
                        }
                        delete res;
                        delete prep_stmt;
                    }
                }
            }

            std::cout << result;
        } else {
            std::cout << JsonUtils::makeJsonError("You need to be logged in to view this area");
        }
    } catch (sql::SQLException &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()) + " (MySQL error code: " + std::to_string(e.getErrorCode()) + ")");
    } catch (const std::exception &e) {
        std::cout << JsonUtils::makeJsonError(std::string(e.what()));
    }

    return 0;
}
