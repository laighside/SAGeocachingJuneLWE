/**
  @file    get_cache.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/gpx_builder/get_cache.cgi
  This gets the details for a given cache
  GET requests only, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/CgiEnvironment.h"
#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"
#include "../core/KeyValueParser.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_gpxbuilder")) { //if logged in

            int cache_number = 0;
            try {
                cache_number = std::stoi(urlQueries.getValue("cache_number"));
            } catch (...) {}
            int id_number = 0;
            try {
                id_number = std::stoi(urlQueries.getValue("user_cache_id_number"));
            } catch (...) {}

            nlohmann::json jsonDocument;

            int cache_number_for_photos = 0;

            if (cache_number) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT cache_name,team_name,latitude,longitude,public_hint,detailed_hint,camo,permanent,private_property,actual_distance FROM caches WHERE cache_number = ?;");
                prep_stmt->setInt(1, cache_number);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    jsonDocument["success"] = true;
                    jsonDocument["cache_name"] = res->getString(1);
                    jsonDocument["team_name"] = res->getString(2);
                    jsonDocument["latitude"] = res->getDouble(3);
                    jsonDocument["longitude"] = res->getDouble(4);
                    jsonDocument["public_hint"] = res->getString(5);
                    jsonDocument["detailed_hint"] = res->getString(6);
                    jsonDocument["camo"] = res->getInt(7);
                    jsonDocument["permanent"] = res->getInt(8);
                    jsonDocument["private_property"] = res->getInt(9);
                    jsonDocument["actual_distance"] = res->getInt(10);
                    jsonDocument["cache_exists"] = true;
                } else {
                    jsonDocument["success"] = false;
                    jsonDocument["error"] = "Cache not found";
                    jsonDocument["cache_exists"] = false;
                }
                delete res;
                delete prep_stmt;

                cache_number_for_photos = cache_number;

            } else if (id_number) {
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT cache_number,cache_name,team_name,phone_number,latitude,longitude,public_hint,detailed_hint,camo,permanent,private_property,actual_distance,IP_address FROM user_hidden_caches WHERE id_number = ?;");
                prep_stmt->setInt(1, id_number);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    jsonDocument["success"] = true;
                    jsonDocument["cache_number"] = res->getInt(1);
                    jsonDocument["cache_name"] = res->getString(2);
                    jsonDocument["team_name"] = res->getString(3);
                    jsonDocument["phone_number"] = res->getString(4);
                    jsonDocument["latitude"] = res->getDouble(5);
                    jsonDocument["longitude"] = res->getDouble(6);
                    jsonDocument["public_hint"] = res->getString(7);
                    jsonDocument["detailed_hint"] = res->getString(8);
                    jsonDocument["camo"] = res->getInt(9);
                    jsonDocument["permanent"] = res->getInt(10);
                    jsonDocument["private_property"] = res->getInt(11);
                    jsonDocument["actual_distance"] = res->getInt(12);
                    jsonDocument["ip_address"] = res->getString(13);

                    cache_number_for_photos = res->getInt(1);
                } else {
                    jsonDocument["success"] = false;
                    jsonDocument["error"] = "Cache not found";
                }
                delete res;
                delete prep_stmt;
            } else {
                jsonDocument["success"] = false;
                jsonDocument["error"] = "Invalid cache number";
            }

            jsonDocument["photos"] = nlohmann::json::array();

            std::string public_upload_dir = jlwe.config.at("publicFileUpload").at("directory");
            std::string base_file_dir = jlwe.config.at("files").at("directory");
            bool includePublicUploads = (base_file_dir.size() < public_upload_dir.size()) && (public_upload_dir.substr(0, base_file_dir.size()) == base_file_dir);

            if (includePublicUploads && cache_number_for_photos > 0) {
                std::string upload_folder_url = std::string(jlwe.config.at("files").at("urlPrefix")) + public_upload_dir.substr(base_file_dir.size()) + "/";

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT server_filename,file_size FROM public_file_upload WHERE cache_number = ? AND file_size > 0;");
                prep_stmt->setInt(1, cache_number_for_photos);
                res = prep_stmt->executeQuery();
                while (res->next()) {
                    nlohmann::json jsonObject;
                    jsonObject["filename"] = res->getString(1);
                    jsonObject["file_size"] = res->getInt(2);
                    jsonObject["href"] = upload_folder_url + res->getString(1);
                    jsonDocument["photos"].push_back(jsonObject);
                }
                delete res;
                delete prep_stmt;
            }

            std::cout << JsonUtils::makeJsonHeader() << jsonDocument.dump();

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
