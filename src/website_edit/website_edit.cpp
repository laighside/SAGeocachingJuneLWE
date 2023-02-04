/**
  @file    website_edit.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/website_edit/website_edit.cgi
  This page allows admins to edit the public side of the website.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>

#include "../core/Encoder.h"
#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - Edit Website", false))
            return 0;

        if (jlwe.getPermissionValue("perm_website_edit")) { //if logged in
	    std::string message = "";

            std::cout << FormElements::includeJavascript("/js/utils.js");
            std::cout << FormElements::includeJavascript("/js/page_tab_tools.js");

            std::cout << "<script type=\"text/javascript\">\n";
            std::cout << "function savePageHTML(){\n";
            std::cout << "    var jsonObj = {\n";
            std::cout << "        \"path\":document.getElementById(\"page_select\").value,\n";
            std::cout << "        \"title\":document.getElementById(\"title_edit\").value,\n";
            std::cout << "        \"html\":document.getElementById(\"html_edit\").value\n";
            std::cout << "    };\n";

            std::cout << "    postUrl('/cgi-bin/website_edit/set_webpage_json.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "            function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, \n";
            std::cout << "                    function(jsonData) {\n";
            std::cout << "                        document.getElementById(\"save_note\").style.display = 'none';\n";
            std::cout << "                    }\n";
            std::cout << "                , null);\n";
            std::cout << "         }, httpErrorResponseHandler);\n";
            std::cout << "}\n\n";


            std::cout << "function previewHTML(){\n";
            std::cout << "    document.getElementById(\"page_tab_button_preview_tab\").click();\n";
            std::cout << "    document.getElementById(\"save_note\").style.display = 'inline';\n";
            std::cout << "};\n";


            std::cout << "function openPreviewTab(){\n";
            std::cout << "    document.getElementById(\"preview_area\").innerHTML = document.getElementById(\"html_edit\").value;\n";
            std::cout << "};\n";


            std::cout << "function getPageHTML(){\n";

            std::cout << "    var page_path = document.getElementById(\"page_select\").value;\n";
            std::cout << "    downloadUrl('/cgi-bin/website_edit/get_webpage_json.cgi?page=' + page_path, null,\n";
            std::cout << "        function(data, responseCode) {\n";

            std::cout << "            if (responseCode === 200){\n";
            std::cout << "                var jsonObj = JSON.parse(data);\n";

            std::cout << "                if (jsonObj.error == null){\n";
            std::cout << "                    document.getElementById(\"title_edit\").value = jsonObj.title;\n";
            std::cout << "                    document.getElementById(\"html_edit\").value = jsonObj.html;\n";
            std::cout << "                    document.getElementById(\"preview_area\").innerHTML = jsonObj.html;\n";
            std::cout << "                    document.getElementById(\"page_note\").innerHTML = '';\n";
            std::cout << "                }else{\n";
            std::cout << "                    document.getElementById(\"title_edit\").value = '';\n";
            std::cout << "                    document.getElementById(\"html_edit\").value = '';\n";
            std::cout << "                    document.getElementById(\"page_note\").innerHTML = 'Error: ' + jsonObj.error;\n";
            std::cout << "                }\n";
            std::cout << "            }\n";
            std::cout << "     });\n";
            std::cout << "};\n";

            std::cout << "var file_changed_name = '';\n\n";
            std::cout << "function deleteImage(file_name){\n";
            std::cout << "    if (confirm(\"Are you sure you wish to delete image: \" + file_name) == true) {\n";

            std::cout << "        var jsonObj = {\n";
            std::cout << "            \"delete_image\":file_name\n";
            std::cout << "        };\n";
            std::cout << "        file_changed_name = file_name;\n";

            std::cout << "        postUrl('delete_image.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "        function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, function() {\n";
            std::cout << "                        var row = document.getElementById(\"table_row_\" + file_changed_name);\n";
            std::cout << "                        row.parentNode.removeChild(row);\n";
            std::cout << "                    }, null);\n";
            std::cout << "         }, httpErrorResponseHandler);\n";
            std::cout << "    }\n";
            std::cout << "}\n\n";


            std::cout << "var image_url_base = \"" + std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(jlwe.config.at("files").at("imagePrefix")) + "/\";\n";
            std::cout << "function uploadImage(){\n";
            std::cout << "    var fileInput = document.getElementById('fileInput');\n";
            std::cout << "    if (fileInput.files.length > 0) {\n";

            std::cout << "        var fd = new FormData();\n";
            std::cout << "        fd.append('file', fileInput.files[0]);\n";

            std::cout << "        postUrl('upload_image.cgi', fd, null,\n";
            std::cout << "        function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, function(jsonResponse) {\n";
            std::cout << "                        if (jsonResponse.filename) {\n";
            std::cout << "                            var table = document.getElementById(\"image_table\");\n";
            std::cout << "                            var newRow = table.insertRow(1);\n";
            std::cout << "                            newRow.setAttribute(\"id\", \"table_row_\" + jsonResponse.filename);\n";
            std::cout << "                            var cellName = newRow.insertCell(0);\n";
            std::cout << "                            var cellUrl = newRow.insertCell(1);\n";
            std::cout << "                            var cellDelete = newRow.insertCell(2);\n";
            std::cout << "                            cellName.innerHTML = '<a href=\"' + image_url_base + jsonResponse.filename + '\" data-lightbox=\"image-list\" data-title=\"' + jsonResponse.filename + '\">' + jsonResponse.filename + '</a>';\n";
            std::cout << "                            cellUrl.innerHTML = '<input type=\"text\" value=\"' + image_url_base + jsonResponse.filename + '\" style=\"width:400px\" disabled />';\n";
            std::cout << "                            cellDelete.innerHTML = '<button onclick=\"deleteImage(\\\'' + jsonResponse.filename + '\\\')\" type=\"button\">Delete</button></td>';\n";

            std::cout << "                        }\n";
            std::cout << "                    }, null);\n";
            std::cout << "         }, httpErrorResponseHandler);\n";

            std::cout << "        fileInput.value = \"\";\n";
            std::cout << "    }\n";
            std::cout << "}\n\n";

            std::cout << "</script>\n";

            html.outputAdminMenu();

            std::cout << "<h2 style=\"text-align:center\">Edit Website</h2>\n";
            std::cout << "<p id=\"page_note\" style=\"color:red;text-align:center;\"></p>";
            std::cout << "<p style=\"color:red;text-align:center;\"><noscript>You need javascript enabled to use the admin tools on this site.</noscript></p>\n";

            std::cout << "<p style=\"text-align:center;\">Select page to edit: \n";
            std::cout << "<select id=\"page_select\" onchange=\"getPageHTML()\">\n";
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT path FROM webpages WHERE editable = 1;");
            while (res->next()) {
                std::string page_path = res->getString(1);
                if (page_path == "/index.html") {
                    std::cout << "<option value=\"" << Encoder::htmlAttributeEncode(page_path) << "\" selected=\"true\">" << Encoder::htmlEntityEncode(page_path) << "</option>";
                } else {
                    std::cout << "<option value=\"" << Encoder::htmlAttributeEncode(page_path) << "\">" << Encoder::htmlEntityEncode(page_path) << "</option>";
                }
            }
            delete res;
            delete stmt;
            std::cout << "</select>\n";
            std::cout << "</p>\n";

            std::cout << FormElements::pageTabs({{"edit_tab", "Edit"}, {"preview_tab", "Preview"}, {"images_tab", "Images"}, {"help_tab", "Help"}});

            std::cout << "<div id=\"edit_tab\" class=\"pageTabContent\">\n";

            std::cout << "<p style=\"text-align:center;\">Page Title: <input type=\"text\" id=\"title_edit\" /></p>\n";
            std::cout << "<p style=\"text-align:center;\"><textarea id=\"html_edit\" rows=\"30\" style=\"width:100%;\"></textarea></p>\n";
            std::cout << "<p style=\"text-align:center;\"><input type=\"button\" onclick=\"previewHTML()\" value=\"Show Preview\" /> <input type=\"button\" onclick=\"savePageHTML()\" value=\"Save changes\" /></p>\n";

            std::cout << "</div>\n";
            std::cout << "<div id=\"preview_tab\" class=\"pageTabContent\">\n";

            std::cout << "<div id=\"preview_block\">\n";
            std::cout << "<p style=\"text-align:center;background-color: #4FAE27;margin: 10px 0 10px 0;padding: 5px;font-weight: bold;color: white;\">Preview of Webpage is shown below<span id=\"save_note\" style=\"color: red;display:none;\"><br/>Note: changes have not been saved yet</span></p>\n";
            std::cout << "<p id=\"preview_area\"></p>\n";
            std::cout << "<p style=\"text-align:center;background-color: #4FAE27;margin: 10px 0 10px 0;padding: 5px;font-weight: bold;color: white;font-style: italic;\">End of Preview</p>\n";
            std::cout << "</div>\n";

            std::cout << "</div>\n";
            std::cout << "<div id=\"images_tab\" class=\"pageTabContent\">\n";

            std::cout << "<h2 style=\"text-align:center\">Images</h2>\n";
            std::cout << "<p>Below is a list of images on the server that may be used on webpages. You can also upload new images.</p>";

            std::cout << "<p style=\"text-align:center;\"><input type=\"button\" onclick=\"document.getElementById('fileInput').click();\" value=\"Upload New Image\"></p>\n";
            std::cout << "<input type=\"file\" id=\"fileInput\" style=\"display:none\" onchange=\"uploadImage();\">\n";

            std::cout << "<p><table id=\"image_table\" align=\"center\"><tr>\n";
            std::cout << "<th>Filename</th><th>URL</th><th></th>\n";
            std::cout << "</tr>\n";

            std::string image_url_base = std::string(jlwe.config.at("http")) + std::string(jlwe.config.at("websiteDomain")) + std::string(jlwe.config.at("files").at("imagePrefix")) + "/";
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT filename FROM webpage_images;");
            while (res->next()) {
                std::string filename = res->getString(1);
                std::string image_url = image_url_base + filename;
                std::cout << "<tr id=\"table_row_" << Encoder::htmlAttributeEncode(filename) << "\">\n";
                std::cout << "<td><a href=\"" << Encoder::htmlAttributeEncode(image_url) << "\" data-lightbox=\"image-list\" data-title=\"" << Encoder::htmlAttributeEncode(filename) << "\">" << Encoder::htmlEntityEncode(filename) << "</a></td>\n";
                std::cout << "<td><input type=\"text\" value=\"" << Encoder::htmlAttributeEncode(image_url) << "\" style=\"width:400px\" disabled /></td>\n";
                std::cout << "<td><button onclick=\"deleteImage('" << Encoder::javascriptAttributeEncode(filename) << "')\" type=\"button\">Delete</button></td>\n";
                std::cout << "</tr>\n";
            }
            delete res;
            delete stmt;
            std::cout << "</table></p>\n";

            std::cout << "</div>\n";

            // HTML help tab
            std::cout << "<div id=\"help_tab\" class=\"pageTabContent\">\n";

            std::cout << "<p>To change the cutoff dates for camping/dinner/merchandise orders, please go to <a href=\"/cgi-bin/settings/settings.cgi\">Website Settings</a>.</p>";

            std::cout << "<p>If you want to temporarily remove some content from a webpage (eg. the link to the dinner event during a year without a dinner event), please just comment it out so the code can easily be put back in the following year.</p>\n";

            std::cout << "<h2 style=\"text-align:center\">HTML hints</h2>\n";
            std::cout << "<p style=\"text-align:center\">Below is a list of common HTML tags for formatting text.</p>";

            std::cout << "<p></p><table align=\"center\" class=\"no_border\">\n";

            std::cout << "<tr><td style=\"text-align: right;\">HTML Comments:</td><td><code>" << Encoder::htmlEntityEncode("<!-- This will not appear on the page -->") << "</code></td>\n";
            std::cout << "<tr><td style=\"text-align: right;\">New line:</td><td><code>" << Encoder::htmlEntityEncode("<br />") << "</code></td>\n";
            std::cout << "<tr><td style=\"text-align: right;\">Make a paragraph:</td><td><code>" << Encoder::htmlEntityEncode("<p>This is a paragraph</p>") << "</code></td>\n";
            std::cout << "<tr><td style=\"text-align: right;\">Bold text:</td><td><code>" << Encoder::htmlEntityEncode("<span style=\"font-weight:bold;\">This text will be bold</span>") << "</code></td>\n";
            std::cout << "<tr><td style=\"text-align: right;\">Italic text:</td><td><code>" << Encoder::htmlEntityEncode("<span style=\"font-style:italic;\">This text will be italic</span>") << "</code></td>\n";
            std::cout << "<tr><td style=\"text-align: right;\">Change the font size:</td><td><code>" << Encoder::htmlEntityEncode("<span style=\"font-size:20px;\">This text will font size 20</span>") << "</code></td>\n";
            std::cout << "<tr><td style=\"text-align: right;\">Center a paragraph:</td><td><code>" << Encoder::htmlEntityEncode("<p style=\"text-align:center;\">This paragraph will be centered</p>") << "</code></td>\n";
            std::cout << "<tr><td style=\"text-align: right;\">Link to a URL:</td><td><code>" << Encoder::htmlEntityEncode("<a href=\"https://example.com\">Link text</a>") << "</code></td>\n";
            std::cout << "<tr><td style=\"text-align: right;\">Insert a image:</td><td><code>" << Encoder::htmlEntityEncode("<img src=\"https://example.com/image.png\" alt=\"Alt text\" />") << "</code></td>\n";
            std::cout << "<tr><td style=\"text-align: right;\">Image with lightbox:</td><td><code>" << Encoder::htmlEntityEncode("<a href=\"https://example.com/image.png\" data-lightbox=\"gallery\" data-title=\"\"><img src=\"https://example.com/image.png\" /></a>") << "</code></td>\n";

            std::cout << "</table>\n";

            std::cout << "</div>\n";


            std::cout << "<script type=\"text/javascript\">\n";
            std::cout << "window.onload = getPageHTML();\n";
            std::cout << "document.getElementsByClassName(\"defaultPageTab\")[0].click();\n";

            std::cout << "document.getElementById(\"page_tab_button_preview_tab\").addEventListener(\"click\", openPreviewTab, false);\n";

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
