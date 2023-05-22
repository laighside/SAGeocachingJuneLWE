/**
  @file    set_slide.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/scoring/set_slide.cgi
  Sets the title, content, enabled and order of the powerpoint slides.
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

            if (jsonDocument.contains("order") && jsonDocument["order"].is_array()) {
                int i = 1;
                int errorCount = 0;
                for (nlohmann::json::iterator it = jsonDocument["order"].begin(); it != jsonDocument["order"].end(); ++it) {
                    int slide_id = *it;
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setSlideOrder(?,?,?,?);");
                    prep_stmt->setInt(1, slide_id);
                    prep_stmt->setInt(2, i);
                    prep_stmt->setString(3, jlwe.getCurrentUserIP());
                    prep_stmt->setString(4, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        errorCount += res->getInt(1);
                    } else {
                        errorCount++;
                    }
                    delete res;
                    delete prep_stmt;
                    i++;
                }
                if (errorCount) {
                    result = JsonUtils::makeJsonError("Error executing an order update query");
                } else {
                    result = JsonUtils::makeJsonSuccess("Slide order updated");
                }
            } else {

                int slide_id = jsonDocument.at("slide_id");
                std::string slide_type = "";
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT type FROM powerpoint_slides WHERE id = ?;");
                prep_stmt->setInt(1, slide_id);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    slide_type = res->getString(1);
                } else {
                    throw std::invalid_argument("Invalid slide ID: " + std::to_string(slide_id));
                }
                delete res;
                delete prep_stmt;

                if (jsonDocument.contains("enabled")) {
                    if (slide_type != "winner" && slide_type != "runnerup" && slide_type != "naga" && slide_type != "scores") {
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setSlideEnabled(?,?,?,?);");
                        prep_stmt->setInt(1, slide_id);
                        prep_stmt->setInt(2, jsonDocument.at("enabled") ? 1 : 0);
                        prep_stmt->setString(3, jlwe.getCurrentUserIP());
                        prep_stmt->setString(4, jlwe.getCurrentUsername());
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                            if (res->getInt(1) == 0) {
                                result = JsonUtils::makeJsonSuccess("Slide enabled updated");
                            } else {
                                result = JsonUtils::makeJsonError("Slide ID not found");
                            }
                        } else {
                            result = JsonUtils::makeJsonError("Unable to execute query");
                        }
                        delete res;
                        delete prep_stmt;
                    } else {
                        result = JsonUtils::makeJsonError("Slide type \"" + slide_type + "\" doesn't allow enabled setting");
                    }
                }

                if (jsonDocument.contains("title") && jsonDocument.contains("content")) {
                    if (slide_type == "generic") {
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setSlideContent(?,?,?,?,?);");
                        prep_stmt->setInt(1, slide_id);
                        prep_stmt->setString(2, std::string(jsonDocument.at("title")));
                        prep_stmt->setString(3, std::string(jsonDocument.at("content")));
                        prep_stmt->setString(4, jlwe.getCurrentUserIP());
                        prep_stmt->setString(5, jlwe.getCurrentUsername());
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                            if (res->getInt(1) == 0) {
                                result = JsonUtils::makeJsonSuccess("Slide content updated");
                            } else {
                                result = JsonUtils::makeJsonError("Slide ID not found");
                            }
                        } else {
                            result = JsonUtils::makeJsonError("Unable to execute query");
                        }
                        delete res;
                        delete prep_stmt;
                    } else {
                        result = JsonUtils::makeJsonError("Slide type \"" + slide_type + "\" doesn't allow editing content");
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
