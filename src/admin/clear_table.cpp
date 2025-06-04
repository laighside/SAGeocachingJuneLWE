/**
  @file    clear_table.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/admin/clear_table.cgi
  Clears a given table in the MySQL database. Used for resetting things for the next year's event.
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <filesystem>

#include "../core/CgiEnvironment.h"
#include "../core/KeyValueParser.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/JsonUtils.h"
#include "../core/PostDataParser.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << JsonUtils::makeJsonError(postData.errorText());
            return 0;
        }

        sql::Statement *stmt;
        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_admin")) { //if logged in

            nlohmann::json jsonDocument = nlohmann::json::parse(postData.dataAsString());

            bool confirm = jsonDocument.value("confirm", false);
            std::string table_name = jsonDocument.value("table", "");

            if (confirm == true) {

                if (table_name == "cache_handout") {

                    int cache_count = jsonDocument.value("cache_count", 0);
                    if (cache_count < 1 || cache_count > 10000)
                        throw std::invalid_argument("Invalid cache_count: " + std::to_string(cache_count));

                    bool error = false;
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT setVariable(?,?,?,?,?);");
                    prep_stmt->setString(1, "number_game_caches");
                    prep_stmt->setString(2, std::to_string(cache_count));
                    prep_stmt->setString(3, jlwe.getCurrentUserIP());
                    prep_stmt->setString(4, jlwe.getCurrentUsername());
                    prep_stmt->setInt(5, 1); // ignore editable flag
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        if (res->getInt(1) != 0)
                            error = true;
                    } else {
                        error = true;
                    }
                    delete res;
                    delete prep_stmt;

                    if (error == false) {
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT clearCacheHandoutList(?,?,?);");
                        prep_stmt->setString(1, jlwe.getCurrentUserIP());
                        prep_stmt->setString(2, jlwe.getCurrentUsername());
                        prep_stmt->setInt(3, cache_count);
                        res = prep_stmt->executeQuery();
                        delete res;
                        delete prep_stmt;

                        std::cout << JsonUtils::makeJsonSuccess("Cache handout list successfully cleared");
                    } else {
                        std::cout << JsonUtils::makeJsonError("Error updating number_game_caches setting");
                    }

                } else if (table_name == "public_file_upload") {

                    // delete the files first
                    std::string public_upload_dir = jlwe.config.at("publicFileUpload").at("directory");
                    stmt = jlwe.getMysqlCon()->createStatement();
                    res = stmt->executeQuery("SELECT server_filename FROM public_file_upload;");
                    while (res->next()) {
                        std::string filename = res->getString(1);
                        std::string full_filename = public_upload_dir + "/" + filename;
                        if (std::filesystem::is_regular_file(full_filename)) {
                            if (!std::filesystem::remove(full_filename))
                                throw std::runtime_error("Unable to delete file: " + full_filename);
                        }
                        full_filename = public_upload_dir + "/.resize/" + filename;
                        if (std::filesystem::is_regular_file(full_filename)) {
                            if (!std::filesystem::remove(full_filename))
                                throw std::runtime_error("Unable to delete file: " + full_filename);
                        }
                    }
                    delete res;
                    delete stmt;

                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT clearPublicFileUpload(?,?);");
                    prep_stmt->setString(1, jlwe.getCurrentUserIP());
                    prep_stmt->setString(2, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    delete res;
                    delete prep_stmt;

                    std::cout << JsonUtils::makeJsonSuccess("Public file list successfully cleared");

                } else if (table_name == "caches") {

                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT clearGpxBuilder(?,?);");
                    prep_stmt->setString(1, jlwe.getCurrentUserIP());
                    prep_stmt->setString(2, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    delete res;
                    delete prep_stmt;

                    std::cout << JsonUtils::makeJsonSuccess("Caches list successfully cleared");

                } else if (table_name == "game_teams") {

                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT clearTeamList(?,?);");
                    prep_stmt->setString(1, jlwe.getCurrentUserIP());
                    prep_stmt->setString(2, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    delete res;
                    delete prep_stmt;

                    std::cout << JsonUtils::makeJsonSuccess("Team list successfully cleared");

                } else if (table_name == "email_list") {

                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT clearEmailList(?,?);");
                    prep_stmt->setString(1, jlwe.getCurrentUserIP());
                    prep_stmt->setString(2, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    delete res;
                    delete prep_stmt;

                    std::cout << JsonUtils::makeJsonSuccess("Mailing list successfully cleared");

                } else if (table_name == "registrations") {

                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT clearRegistrations(?,?);");
                    prep_stmt->setString(1, jlwe.getCurrentUserIP());
                    prep_stmt->setString(2, jlwe.getCurrentUsername());
                    res = prep_stmt->executeQuery();
                    delete res;
                    delete prep_stmt;

                    std::cout << JsonUtils::makeJsonSuccess("Registration lists successfully cleared");

                } else {
                    std::cout << JsonUtils::makeJsonError("Invalid table name");
                }
            } else {
                std::cout << JsonUtils::makeJsonError("Please confirm that you wish to overwrite data");
            }
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
