/**
  @file    file_api.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/files/file_api.cgi
  This does a number of things related to the file manager.
  Gets a list of files/directories, creates directories, renames files/directories, deletes files/directories, change public/private status of files
  Requests to this API come from h5ai
  POST requests only, with JSON data, return type is always JSON.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/Encoder.h"
#include "../core/JlweUtils.h"
#include "../core/JlweCore.h"
#include "../core/JsonUtils.h"
#include "../core/PostDataParser.h"

#include "../ext/nlohmann/json.hpp"

std::string removeFileUrlPrefix(std::string href, std::string prefix) {
    size_t prefix_length = prefix.size();
    if (href.substr(0, prefix_length) == prefix) {
        return href.substr(prefix_length);
    } else {
        throw std::invalid_argument("Invalid href file path");
    }
}

int main () {
    try {
        JlweCore jlwe;

        // Allow CORS if file manager is running on a different domain
        /*
        std::cout << "Access-Control-Allow-Origin: *\r\n";
        std::cout << "Access-Control-Allow-Credentials: true\r\n";
        std::cout << "Access-Control-Allow-Methods: GET,HEAD,OPTIONS,POST,PUT\r\n";
        std::cout << "Access-Control-Allow-Headers: Access-Control-Allow-Headers, Origin,Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers\r\n";
        */

        PostDataParser postData(jlwe.config.at("maxPostSize"));
        if (postData.hasError()) {
            std::cout << JsonUtils::makeJsonError(postData.errorText());
            return 0;
        }

        sql::Statement *stmt;
        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        if (jlwe.getPermissionValue("perm_file")) { //if logged in

            nlohmann::json jsonDocumentIn = nlohmann::json::parse(postData.dataAsString());

            nlohmann::json jsonDocumentOut;

            std::string action = jsonDocumentIn.at("action");

            if (JlweUtils::compareStringsNoCase(action, "get")) {

                if (jsonDocumentIn.value("options", false)) {
                    jsonDocumentOut["options"] = nlohmann::json::parse(JlweUtils::readFileToString("options.json"), nullptr, true, true);
                }

                if (jsonDocumentIn.value("types", false)) {
                    jsonDocumentOut["types"] = nlohmann::json::parse(JlweUtils::readFileToString("types.json"), nullptr, true, true);
                }

                if (jsonDocumentIn.value("setup", false)) {
                    nlohmann::json jsonObject;
                    jsonObject["publicHref"] = "/h5ai/";
                    jsonObject["rootHref"] = "/files/";
                    jsonObject["h5aiHref"] = "/h5ai/";
                    jsonDocumentOut["setup"] = jsonObject;
                }

                if (jsonDocumentIn.value("langs", false)) {
                    nlohmann::json jsonObject;
                    jsonObject["en"] = "english";
                    jsonDocumentOut["langs"] = jsonObject;
                }

                if (jsonDocumentIn.contains("items")) {

                    std::string public_upload_dir = jlwe.config.at("publicFileUpload").at("directory");
                    std::string base_file_dir = jlwe.config.at("files").at("directory");
                    bool includePublicUploads = (base_file_dir.size() < public_upload_dir.size()) && (public_upload_dir.substr(0, base_file_dir.size()) == base_file_dir);

                    // public uploads
                    if (includePublicUploads) {
                        std::string upload_folder_url = std::string(jlwe.config.at("files").at("urlPrefix")) + public_upload_dir.substr(base_file_dir.size()) + "/";

                        nlohmann::json jsonObject;
                        jsonObject["href"] = upload_folder_url;
                        jsonObject["size"] = 0;
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT SUM(file_size) FROM public_file_upload;");
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                            jsonObject["size"] = res->getInt(1);
                        }
                        delete res;
                        delete prep_stmt;
                        jsonObject["time"] = 0;
                        jsonObject["owner"] = "";
                        jsonObject["status"] = "";
                        jsonObject["fetched"] = false;
                        jsonObject["managed"] = true;
                        jsonObject["readOnly"] = true;
                        jsonDocumentOut["items"].push_back(jsonObject);

                        stmt = jlwe.getMysqlCon()->createStatement();
                        res = stmt->executeQuery("SELECT server_filename,unix_timestamp(timestamp),file_size,user_ip FROM public_file_upload ORDER BY timestamp DESC;");
                        while (res->next()) {
                            nlohmann::json jsonObject;
                            jsonObject["href"] = upload_folder_url + res->getString(1);
                            jsonObject["size"] = res->getInt(3);
                            jsonObject["time"] = res->getInt64(2) * 1000;
                            jsonObject["owner"] = res->getString(4);
                            jsonObject["status"] = "";
                            jsonObject["readOnly"] = true;

                            jsonDocumentOut["items"].push_back(jsonObject);
                        }
                        delete res;
                        delete stmt;
                    }


                    // Normal files
                    stmt = jlwe.getMysqlCon()->createStatement();
                    res = stmt->executeQuery("SELECT filename,directory,size,year,owner,public,unix_timestamp(date_uploaded) FROM files ORDER BY year DESC;");
                    while (res->next()) {

                        std::string filename = res->getString(1);
                        bool isFolder = (filename.substr(filename.size() - 1) == "/");
                        nlohmann::json jsonObject;
                        jsonObject["href"] = std::string(jlwe.config.at("files").at("urlPrefix")) + res->getString(2) + filename;
                        jsonObject["size"] = res->getInt(3);
                        jsonObject["time"] = res->getInt64(7) * 1000;
                        jsonObject["owner"] = res->getString(5);
                        std::string changeStatusButton = "<input type=\"image\" src=\"/h5ai/images/ui/change.svg\" alt=\"change status\" title=\"Change Status\" onclick=\"changeStatus('" + Encoder::javascriptAttributeEncode(res->getString(2) + filename) + "', " + (res->getInt(6) ? "0" : "1") + ")\">";
                        jsonObject["status"] = isFolder ? std::string("") : ((res->getInt(6) ? "Public" : "Private") + changeStatusButton);

                        // if it is a directory
                        if (isFolder) {
                            jsonObject["fetched"] = false;
                            jsonObject["managed"] = true;

                            std::string folderName = res->getString(2) + filename;
                            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT SUM(size) FROM files WHERE LEFT(directory, ?) = ?;");
                            prep_stmt->setUInt(1, folderName.size());
                            prep_stmt->setString(2, folderName);
                            sql::ResultSet *res2 = prep_stmt->executeQuery();
                            if (res2->next()) {
                                jsonObject["size"] = res2->getInt(1);
                            }
                            delete res2;
                            delete prep_stmt;

                        }

                        jsonDocumentOut["items"].push_back(jsonObject);

                    }
                    delete res;
                    delete stmt;

                }

                if (jsonDocumentIn.contains("custom")) {
                    // no custom headers and footers
                    jsonDocumentOut["custom"]["header"] =  {{"content", nullptr}, {"type", nullptr}};
                    jsonDocumentOut["custom"]["footer"] =  {{"content", nullptr}, {"type", nullptr}};
                }

                if (jsonDocumentIn.contains("theme")) {
                    jsonDocumentOut["theme"] = {
                            {"ar-apk", "comity/ar-apk.svg"},
                            {"ar-deb", "comity/ar-deb.svg"},
                            {"ar-rpm", "comity/ar-rpm.svg"},
                            {"txt-css", "comity/txt-css.svg"},
                            {"txt-go", "comity/txt-go.svg"},
                            {"txt-html", "comity/txt-html.svg"},
                            {"txt-js", "comity/txt-js.svg"},
                            {"txt-less", "comity/txt-less.svg"},
                            {"txt-md", "comity/txt-md.svg"},
                            {"txt-php", "comity/txt-php.svg"},
                            {"txt-py", "comity/txt-py.svg"},
                            {"txt-rb", "comity/txt-rb.svg"},
                            {"txt-rust", "comity/txt-rust.svg"},
                            {"txt-script", "comity/txt-script.svg"},
                            {"txt-kml", "comity/txt-kml.svg"},
                            {"txt-gpx", "comity/txt-gpx.svg"},
                            {"x-pdf", "comity/x-pdf.svg"}};
                }

                if (jsonDocumentIn.contains("thumbs") && jsonDocumentIn.at("thumbs").is_array()) {
                    nlohmann::json jsonArray = jsonDocumentIn.at("thumbs");
                    for (size_t i = 0; i < jsonArray.size(); i++) {
                        std::string thumbUrl = "/cgi-bin/files/thumbnail.cgi?";
                        thumbUrl += "&type=" + Encoder::urlEncode(jsonArray.at(i).value("type", ""));
                        thumbUrl += "&file=" + Encoder::urlEncode(jsonArray.at(i).value("href", ""));
                        thumbUrl += "&w=" + std::to_string(jsonArray.at(i).value("width", 100));
                        thumbUrl += "&h=" + std::to_string(jsonArray.at(i).value("height", 100));
                        if (jsonArray.at(i).value("type", "") == "img" || jsonArray.at(i).value("type", "") == "doc") {
                            jsonDocumentOut["thumbs"].push_back(thumbUrl);
                        } else {
                            jsonDocumentOut["thumbs"].push_back(nullptr);
                        }
                    }

                }

                std::cout << JsonUtils::makeJsonHeader() << jsonDocumentOut.dump();

            } else if (JlweUtils::compareStringsNoCase(action, "mkdir")) {
                std::string baseHref = removeFileUrlPrefix(jsonDocumentIn.at("baseHref"), jlwe.config.at("files").at("urlPrefix"));

                bool baseDirExists = false;
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT filename FROM files WHERE CONCAT(directory,filename) = ?;");
                prep_stmt->setString(1, baseHref);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    baseDirExists = true;
                }
                delete res;
                delete prep_stmt;

                if (baseDirExists || baseHref == "/") {

                    std::string file_dir = jlwe.config.at("files").at("directory");

                    int successCount = 0;
                    nlohmann::json jsonArray = jsonDocumentIn.at("folders");
                    for (size_t i = 0; i < jsonArray.size(); i++) {
                        std::string newFolder = Encoder::filterSafeCharsOnly(jsonArray.at(i)) + "/";

                        bool dirExists = false;
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT filename FROM files WHERE directory = ? AND filename = ?;");
                        prep_stmt->setString(1, baseHref);
                        prep_stmt->setString(2, newFolder);
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                            dirExists = true;
                        }
                        delete res;
                        delete prep_stmt;

                        if (!dirExists && newFolder.size() > 1) {

                            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT createFile(?,?,?,?,?,?,?);");
                            prep_stmt->setString(1, newFolder);
                            prep_stmt->setString(2, baseHref);
                            prep_stmt->setInt(3, 0);
                            prep_stmt->setInt(4, 0);
                            prep_stmt->setString(5, jlwe.getCurrentUsername());
                            prep_stmt->setInt(6, 0);
                            prep_stmt->setString(7, jlwe.getCurrentUserIP());
                            res = prep_stmt->executeQuery();
                            if (res->next() && res->getInt(1) == 0) {
                                std::string command = "mkdir " + file_dir + baseHref + newFolder;
                                system(command.c_str());
                                command = "mkdir " + file_dir + "/.thumb" + baseHref + newFolder;
                                system(command.c_str());
                                successCount++;
                            }else{
                            }
                        }
                    }

                    std::cout << JsonUtils::makeJsonSuccess(std::to_string(successCount) + " Folder(s) created");
                } else {
                    std::cout << JsonUtils::makeJsonError("Base directory does not exist");
                }




            } else if (JlweUtils::compareStringsNoCase(action, "rm")) { // =remove (delete file)
                std::string href = removeFileUrlPrefix(jsonDocumentIn.at("href"), jlwe.config.at("files").at("urlPrefix"));

                bool fileExists = false;
                bool folderExists = false;
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT filename FROM files WHERE CONCAT(directory,filename) = ?;");
                prep_stmt->setString(1, href);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    std::string filename = res->getString(1);
                    if (filename == jsonDocumentIn.at("filename")) {
                        if (filename.substr(filename.size() - 2) == "/") {
                            folderExists = true;
                        } else {
                            fileExists = true;
                        }
                    }
                }
                delete res;
                delete prep_stmt;

                if (fileExists) {

                    std::string file_dir = jlwe.config.at("files").at("directory");
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT deleteFile(?, ?, ?);");
                    prep_stmt->setString(1, href);
                    prep_stmt->setString(2, jlwe.getCurrentUsername());
                    prep_stmt->setString(3, jlwe.getCurrentUserIP());
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        remove(std::string(file_dir + href).c_str());
                        std::cout << JsonUtils::makeJsonSuccess("File successfully deleted");
                    } else {
                        std::cout << JsonUtils::makeJsonError("Error deleting file");
                    }
                    delete res;
                    delete prep_stmt;

                } else {
                    std::cout << JsonUtils::makeJsonError("File does not exist");
                }


            } else if (JlweUtils::compareStringsNoCase(action, "rename")) {
                std::string href = removeFileUrlPrefix(jsonDocumentIn.at("href"), jlwe.config.at("files").at("urlPrefix"));
                std::string newHref = jsonDocumentIn.value("newHref", "");
                std::string oldName = jsonDocumentIn.at("oldName");
                std::string newName = Encoder::filterSafeCharsOnly(jsonDocumentIn.value("newName", oldName));


                size_t filename_length = oldName.size();
                if (href.substr(href.length() - filename_length) == oldName) {
                    href = href.substr(0, href.length() - filename_length);

                    if (newHref.size() == 0) {
                        newHref = href;
                    } else {
                        newHref = removeFileUrlPrefix(newHref, jlwe.config.at("files").at("urlPrefix"));
                    }

                    bool newDirExists = (newHref == "/");
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT filename FROM files WHERE CONCAT(directory,filename) = ?;");
                    prep_stmt->setString(1, newHref);
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        newDirExists = true;
                    }
                    delete res;
                    delete prep_stmt;

                    if (newName.size() && newDirExists) {

                        bool fileExists = false;
                        bool folderExists = false;
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT filename FROM files WHERE directory = ? AND filename = ?;");
                        prep_stmt->setString(1, href);
                        prep_stmt->setString(2, oldName);
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                            std::string filename = res->getString(1);
                            if (filename == oldName) {
                                if (filename.substr(filename.size() - 2) == "/") {
                                    folderExists = true;
                                } else {
                                    fileExists = true;
                                }
                            }
                        }
                        delete res;
                        delete prep_stmt;

                        bool newFileExists = false;
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT filename FROM files WHERE directory = ? AND filename = ?;");
                        prep_stmt->setString(1, newHref);
                        prep_stmt->setString(2, newName);
                        res = prep_stmt->executeQuery();
                        if (res->next()) {
                            newFileExists = true;
                        }
                        delete res;
                        delete prep_stmt;

                        if (fileExists && !newFileExists) {

                            std::string file_dir = jlwe.config.at("files").at("directory");
                            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT renameFile(?, ?, ?, ?, ?, ?);");
                            prep_stmt->setString(1, href);
                            prep_stmt->setString(2, oldName);
                            prep_stmt->setString(3, newHref);
                            prep_stmt->setString(4, newName);
                            prep_stmt->setString(5, jlwe.getCurrentUsername());
                            prep_stmt->setString(6, jlwe.getCurrentUserIP());
                            res = prep_stmt->executeQuery();
                            if (res->next()) {
                                int renameResult = rename(std::string(file_dir + href + oldName).c_str(), std::string(file_dir + newHref + newName).c_str());
                                if (renameResult == 0) {
                                    std::cout << JsonUtils::makeJsonSuccess("File name changed");
                                } else {
                                    std::cout << JsonUtils::makeJsonError("Error renaming file");
                                }
                            } else {
                                std::cout << JsonUtils::makeJsonError("Error renaming file");
                            }
                            delete res;
                            delete prep_stmt;

                        } else {
                            std::cout << JsonUtils::makeJsonError("File does not exist or new file already exists");
                        }

                    } else {
                        std::cout << JsonUtils::makeJsonError("Invalid file/folder name");
                    }
                } else {
                    std::cout << JsonUtils::makeJsonError("Invalid base directory");
                }


            } else if (JlweUtils::compareStringsNoCase(action, "changeStatus")) { // = change public/private
                std::string sqlHref = jsonDocumentIn.at("sqlHref");
                int newStatus = jsonDocumentIn.at("newStatus");

                bool fileExists = false;
                bool folderExists = false;
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT filename FROM files WHERE CONCAT(directory,filename) = ?;");
                prep_stmt->setString(1, sqlHref);
                res = prep_stmt->executeQuery();
                if (res->next()) {
                    std::string filename = res->getString(1);
                    if (filename.substr(filename.size() - 2) == "/") {
                        folderExists = true;
                    } else {
                        fileExists = true;
                    }
                }
                delete res;
                delete prep_stmt;

                if (fileExists) {

                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT changeStatus(?, ?, ?, ?);");
                    prep_stmt->setString(1, sqlHref);
                    prep_stmt->setInt(2, newStatus);
                    prep_stmt->setString(3, jlwe.getCurrentUsername());
                    prep_stmt->setString(4, jlwe.getCurrentUserIP());
                    res = prep_stmt->executeQuery();
                    if (res->next()) {
                        std::cout << JsonUtils::makeJsonSuccess("File \"" + sqlHref + "\" set to " + (newStatus ? "public" : "private"));
                    } else {
                        std::cout << JsonUtils::makeJsonError("Error changing file status");
                    }
                    delete res;
                    delete prep_stmt;

                } else if (folderExists) {
                    std::cout << JsonUtils::makeJsonError("Can not change the status of folders");
                } else {
                    std::cout << JsonUtils::makeJsonError("File does not exist");
                }

            } else {
                std::cout << JsonUtils::makeJsonError("Invalid action");
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
