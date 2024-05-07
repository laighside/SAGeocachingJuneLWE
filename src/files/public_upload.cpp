/**
  @file    public_upload.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the API endpoint at /cgi-bin/files/public_upload.cgi
  Uploads a file from the public file upload page.
  POST requests only, with multipart/form-data data, return type is HTML.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <cstdio>  // for popen

#include "../core/CgiEnvironment.h"
#include "../core/Encoder.h"
#include "../core/HtmlTemplate.h"
#include "../core/HttpRequest.h"
#include "../core/JlweCore.h"
#include "../core/PostDataParser.h"

#include "../ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        if (jlwe.config.at("publicFileUpload").value("enabled", false) == false) {
            HtmlTemplate::outputPageWithMessage(&jlwe, "File upload is disabled", "JLWE - Upload photos");
            return 0;
        }

        // Parse POST data
        PostDataParser postData(jlwe.config.at("publicFileUpload").at("maxUploadSize"));
        if (postData.hasError()) {
            HtmlTemplate::outputPageWithMessage(&jlwe, postData.errorText(), "JLWE - Upload photos");
            return 0;
        }

        // Check CAPTCHA
        std::string g_recaptcha_response = postData.getValue("g-recaptcha-response");
        std::string captchaPost = "secret=" + std::string(jlwe.config.at("recaptcha").at("key")) + "&response=" + g_recaptcha_response + "&remoteip=" + jlwe.getCurrentUserIP();
        std::string captchaResponse = "{}";
        HttpRequest request(jlwe.config.at("recaptcha").at("url"));
        if (request.post(captchaPost, "application/x-www-form-urlencoded")) {
            captchaResponse = request.responseAsString();
        } else {
            HtmlTemplate::outputPageWithMessage(&jlwe, "Unable to verify CAPTCHA, please try again later or contact " + std::string(jlwe.config.at("adminEmail")), "JLWE - Upload photos");
            return 0;
        }
        nlohmann::json captchaResponseJson = nlohmann::json::parse(captchaResponse);
        if (captchaResponseJson.value("success", false) == false) {
            HtmlTemplate::outputPageWithMessage(&jlwe, "CAPTCHA fail, please try again or contact " + std::string(jlwe.config.at("adminEmail")), "JLWE - Upload photos");
            return 0;
        }

        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        int cache_number = 0;
        try {
            cache_number = std::stoi(postData.getValue("cache_number"));
        } catch (...) {}

        std::string public_upload_dir = jlwe.config.at("publicFileUpload").at("directory");

        nlohmann::json resultJson = nlohmann::json::array();

        // make temp folder and filenames
        char dir_template[] = "/tmp/tmpdir.XXXXXX";
        char *dir_name = mkdtemp(dir_template);
        if (dir_name == nullptr)
            throw std::runtime_error("Unable to create temporary directory");

        std::string tmp_filename = std::string(dir_name) + "/image";

        for (unsigned int i = 0; i < postData.getFiles()->size(); i++) {
            std::string result_str = "Error: unknown error";

            if (postData.getFiles()->at(i).dataType.substr(0, 6) == "image/") {

                int file_idx = 0;
                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT insertPublicFileUpload(?,?,?,?);");
                prep_stmt->setString(1, postData.getFiles()->at(i).filename);
                prep_stmt->setInt(2, cache_number);
                prep_stmt->setString(3, jlwe.getCurrentUserIP());
                prep_stmt->setString(4, jlwe.getCurrentUsername());
                res = prep_stmt->executeQuery();
                if (res->next()){
                    file_idx = res->getInt(1);
                }
                delete res;
                delete prep_stmt;

                std::string server_filename = "";
                if (file_idx > 0) {
                    prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT server_filename FROM public_file_upload WHERE id = ?;");
                    prep_stmt->setInt(1, file_idx);
                    res = prep_stmt->executeQuery();
                    if (res->next()){
                        server_filename = res->getString(1);
                    }
                    delete res;
                    delete prep_stmt;
                }

                if (server_filename.size()) {

                    // make temp file
                    FILE* temp_file = fopen(tmp_filename.c_str(), "wb");
                    if (!temp_file)
                        throw std::runtime_error("Unable to create temporary file");

                    fwrite(postData.getFiles()->at(i).data.c_str(), 1, postData.getFiles()->at(i).data.size(), temp_file);
                    fclose(temp_file);

                    // convert or re-encode the file as a JPEG
                    std::string command = "convert " + tmp_filename + " " + public_upload_dir + "/" + server_filename + " 2>&1";

                    bool convert_successful = false;

                    // run the command and get any errors printed
                    char buffer[1024];
                    std::string convert_result = "";
                    FILE *pipe = popen(command.c_str(), "r");
                    if (!pipe) throw std::runtime_error("popen() failed");
                    while (fgets(buffer, 1024, pipe) != nullptr) {
                        convert_result.append(buffer);
                    }
                    int exit_code = pclose(pipe);

                    // print only the first line of the error
                    size_t idx = convert_result.find_first_of('\n');
                    if (idx != std::string::npos)
                        convert_result = convert_result.substr(0, idx);

                    if (exit_code == 0 && convert_result.size() == 0) {
                        convert_successful = true;
                    } else {
                        result_str = "Image convert error: (exit code " + std::to_string(exit_code) + "), " + convert_result;
                    }

                    remove(tmp_filename.c_str());

                    if (convert_successful) {
                        // Get and save file size in database
                        std::filesystem::path p(public_upload_dir + "/" + server_filename);
                        uint64_t file_size = std::filesystem::file_size(p);
                        prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT updatePublicFileUploadSize(?,?,?,?);");
                        prep_stmt->setInt(1, file_idx);
                        prep_stmt->setUInt64(2, file_size);
                        prep_stmt->setString(3, jlwe.getCurrentUserIP());
                        prep_stmt->setString(4, jlwe.getCurrentUsername());
                        res = prep_stmt->executeQuery();
                        delete res;
                        delete prep_stmt;

                        result_str = "Uploaded successfully";
                    }
                } else {
                    result_str = "Error: Unable to get filename from database";
                }
            } else {
                result_str = "Error: not an image file";
            }

            nlohmann::json jsonObject;
            jsonObject["filename"] = postData.getFiles()->at(i).filename;
            jsonObject["result"] = result_str;
            resultJson.push_back(jsonObject);

        }

        rmdir(dir_name);

        // Make HTML output
        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE - Upload photos", false))
            return 0;

        std::cout << "<p>Uploading photos...</p>";
        for (nlohmann::json::iterator it = resultJson.begin(); it != resultJson.end(); ++it) {
            std::cout << "<p><span style=\"font-weight:bold;\">" << Encoder::htmlEntityEncode(it.value()["filename"]) << ":</span> " << Encoder::htmlEntityEncode(it.value()["result"]) << "</p>\n";
        }
        std::cout << "<p><a href=\"/upload\">Click here to upload more photos</a></p>";

        html.outputFooter();

    } catch (sql::SQLException &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << std::string(e.what());
    } catch (const std::exception &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << "<p>" << e.what() << "</p>";
    }

    return 0;
}
