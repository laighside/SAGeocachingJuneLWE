/**
  @file    public_upload.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/public_upload/list_public_upload.cgi
  This page lists all the files from the public upload, and allows them to be edited/deleted and uploaded to Google Drive

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/Encoder.h"
#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"

std::string fileSizeToString(int filesize) {
    if (filesize <= 1024)
        return std::to_string(filesize) + "B";
    if (filesize <= 1024 * 1024)
        return std::to_string(filesize / 1024) + "kB";
    if (filesize <= 1024 * 1024 * 1024)
        return std::to_string(filesize / (1024 * 1024)) + "MB";
    return std::to_string(filesize / (1024 * 1024 * 1024)) + "GB";
}

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - Public file uploads", false))
            return 0;

        if (jlwe.getPermissionValue("perm_file")) { //if logged in
	    std::string message = "";

            std::cout << FormElements::includeJavascript("/js/utils.js");
            std::cout << FormElements::includeJavascript("/js/menu.js");
            std::cout << FormElements::includeJavascript("/js/public_upload_file_list.js");

            html.outputAdminMenu();

            std::cout << "<h2 style=\"text-align:center\">Public file upload</h2>\n";
            std::cout << "<p id=\"page_note\" style=\"color:red;text-align:center;\"></p>";
            std::cout << "<p style=\"color:red;text-align:center;\"><noscript>You need javascript enabled to use the admin tools on this site.</noscript></p>\n";

            bool hasGoogleDrive = false;
            if (jlwe.config.contains("google_drive_json_auth_file") && std::string(jlwe.config.at("google_drive_json_auth_file")).size() > 0) {
                hasGoogleDrive = true;

                std::string service_account_email = nlohmann::json::parse(JlweUtils::readFileToString(std::string(jlwe.config.at("google_drive_json_auth_file")).c_str())).value("client_email", "");
                std::cout << "<h3 style=\"text-align:center\">Google Drive</h3>\n";
                std::cout << "<p style=\"text-align:center;\">Select a Google Drive folder to upload files to:<br/><select id=\"gd_folder_select\" style=\"min-width:200px;\"></select></p>";
                std::cout << "<p style=\"text-align:center;\">To add a folder to this list, share a Google Drive folder with this email address:</p>";
                std::cout << "<p style=\"text-align:center;\"><code style=\"font-size:14px;\">" + Encoder::htmlEntityEncode(service_account_email) + "</code></p>";
            }

            std::cout << "<h3 style=\"text-align:center\">Files</h3>\n";

            std::string public_upload_url = jlwe.config.at("files").at("urlPrefix");
            public_upload_url += "/public_upload/";

            std::cout << "<table id=\"file_table\" align=\"center\">\n";
            std::cout << "<tr><th>" << FormElements::checkbox("checkbox_all", "", false) << "</th><th>Cache</th><th>Filename</th><th>Size</th><th>On GD?</th><th></th></tr>\n";

            int id_count = 100;
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT cache_number,server_filename,file_size FROM public_file_upload WHERE status = 'S' ORDER BY cache_number;");
            while (res->next()) {
                std::string filename = res->getString(2);
                std::string url = public_upload_url + filename;
                std::cout << "<tr id=\"table_row_" << Encoder::htmlAttributeEncode(filename) << "\" class=\"table_file_row\" data-filename=\"" << Encoder::htmlAttributeEncode(filename) << "\">\n";
                std::cout << "<td>" << FormElements::checkbox("checkbox_" + filename, "", false) << "</td>\n";
                std::cout << "<td style=\"text-align:center;\">" << (res->getInt(1) > 0 ? std::to_string(res->getInt(1)) : "-") << "</td>\n";
                std::cout << "<td><a href=\"" << Encoder::htmlAttributeEncode(public_upload_url + ".resize/" + filename) << "\" data-lightbox=\"image-list\" data-title=\"" << Encoder::htmlAttributeEncode(filename) << "\">" << Encoder::htmlEntityEncode(filename) << "</a> ";
                std::cout << "(<a href=\"" << Encoder::htmlAttributeEncode(url) << "\">full size</a>)</td>\n";
                std::cout << "<td>" << fileSizeToString(res->getInt(3)) << "</td>\n";
                std::cout << "<td id=\"on_gd_cell_" + Encoder::htmlAttributeEncode(filename) + "\" style=\"text-align:center;\">" + (hasGoogleDrive ? "" : "N/A") + "</td>\n";

                std::vector<FormElements::dropDownMenuItem> menu_items = {{"sendFileToGD('" + Encoder::javascriptAttributeEncode(filename) + "')", "Send to Google Drive", hasGoogleDrive},
                                                                          {"rotateAntiClock('" + Encoder::javascriptAttributeEncode(filename) + "')", "Rotate Anti-clockwise", true},
                                                                          {"rotateClockwise('" + Encoder::javascriptAttributeEncode(filename) + "')", "Rotate Clockwise", true},
                                                                          {"deleteImage('" + Encoder::javascriptAttributeEncode(filename) + "')", "Delete Image", true}};
                std::cout << "<td>" << FormElements::dropDownMenu(id_count++, menu_items) << "</td>\n";
                std::cout << "</tr>\n";
            }
            delete res;
            delete stmt;
            std::cout << "</table>\n";

            if (hasGoogleDrive)
                std::cout << "<p style=\"text-align:center;\"><input type=\"button\" id=\"sendSelectedButton\" value=\"Send selected to Google Drive\" style=\"width:400px\" onclick=\"sendSelectedToGD()\" /></p>";

            std::cout << "<script type=\"text/javascript\">\n";
            std::cout << "document.getElementById(\"checkbox_all\").addEventListener(\"change\", checkboxAllChanged, false);\n";
            if (hasGoogleDrive) {
                std::cout << "document.getElementById(\"gd_folder_select\").addEventListener(\"change\", onFolderSelectChange, false);\n";
                std::cout << "window.onload = getGDFolderList();\n";
            }
            std::cout << "</script>\n";

        } else {
            if (jlwe.isLoggedIn()) {
                std::cout << "<p>You don't have permission to view this area.</p>";
            } else {
                std::cout << "<p>You need to be logged in to view this area.</p>";
            }
        }

        html.outputFooter();

    } catch (const sql::SQLException &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << e.what() << " (MySQL error code: " << std::to_string(e.getErrorCode()) << ")\n";
    } catch (const std::exception &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << e.what();
    }

    return 0;
}
