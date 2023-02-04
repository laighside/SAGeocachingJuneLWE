/**
  @file    settings.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/settings/settings.cgi
  This page allows admins to view and edit various website settings.

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
        if (!html.outputHeader(&jlwe, "JLWE Admin area", false))
            return 0;

        if (jlwe.isLoggedIn()) { //if logged in
            std::cout << "<script type=\"text/javascript\" src=\"/js/utils.js\"></script>\n";
            std::cout << "<script type=\"text/javascript\">\n";

            std::cout << "function setTextVariable(var_name){\n";
            std::cout << "    setVariable(var_name, document.getElementById(var_name + \"_textbox\").value);\n";
            std::cout << "}\n\n";

            std::cout << "function setDateTimeVariable(var_name){\n";
            std::cout << "    setVariable(var_name, (new Date(document.getElementById(var_name + \"_datebox\").value + \"T\" + document.getElementById(var_name + \"_timebox\").value).getTime() / 1000).toFixed(0));\n";
            std::cout << "}\n\n";

            std::cout << "function setSelectVariable(var_name){\n";
            std::cout << "    setVariable(var_name, document.getElementById(var_name + \"_selectbox\").value);\n";
            std::cout << "}\n\n";

            std::cout << "function setEnabledVariable(var_name){\n";
            std::cout << "    var value = '';\n";
            std::cout << "    if (document.getElementById(var_name + \"_yes\").checked) value = '1';\n";
            std::cout << "    if (document.getElementById(var_name + \"_no\").checked) value = '0';\n";
            std::cout << "    setVariable(var_name, value);\n";
            std::cout << "}\n\n";

            std::cout << "function setVariable(var_name, var_value){\n";
            std::cout << "    var jsonObj = {\n";
            std::cout << "        \"var_name\":var_name,\n";
            std::cout << "        \"var_value\":var_value\n";
            std::cout << "    };\n";

            std::cout << "    postUrl('set_variable.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "        function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, null, null);\n";
            std::cout << "         }, httpErrorResponseHandler);\n";
            std::cout << "}\n\n";

            std::cout << "function doSetZonePost(jsonObj){\n";
            std::cout << "    postUrl('set_zone_points.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "        function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, null, null);\n";
            std::cout << "         }, httpErrorResponseHandler);\n";
            std::cout << "}\n\n";

            std::cout << "function setZonePoints(kml_name){\n";
            std::cout << "    var jsonObj = {\n";
            std::cout << "        \"kml_name\":kml_name,\n";
            std::cout << "        \"zone_name\":document.getElementById(kml_name + \"_name_textbox\").value,\n";
            std::cout << "        \"points\":parseInt(document.getElementById(kml_name + \"_points\").value)\n";
            std::cout << "    };\n";
            std::cout << "    doSetZonePost(jsonObj);\n";
            std::cout << "}\n\n";

            std::cout << "function deleteZone(kml_name){\n";
            std::cout << "    if (confirm(\"Are you sure you wish to delete this zone?\") == true) {\n";
            std::cout << "        var jsonObj = {\n";
            std::cout << "            \"kml_name\":kml_name,\n";
            std::cout << "            \"delete\":true\n";
            std::cout << "        };\n";
            std::cout << "        doSetZonePost(jsonObj);\n";
            std::cout << "    }\n";
            std::cout << "}\n\n";

            std::cout << "function addZone(){\n";
            std::cout << "    var dd = document.getElementById(\"add_zone_kmls\");\n";
            std::cout << "    var kml_name = dd.options[dd.selectedIndex].value;\n";
            std::cout << "    var jsonObj = {\n";
            std::cout << "        \"kml_name\":kml_name,\n";
            std::cout << "        \"zone_name\":kml_name,\n";
            std::cout << "        \"points\":0\n";
            std::cout << "    };\n";
            std::cout << "    doSetZonePost(jsonObj);\n";
            std::cout << "}\n\n";

            std::cout << "function loadDateTime(var_name, unix_time){\n";
            std::cout << "    var dateObj = new Date(parseInt(unix_time) * 1000);\n";
            std::cout << "    document.getElementById(var_name + '_datebox').value = dateObj.getFullYear().toFixed().padStart(4, '0') + \"-\" + (dateObj.getMonth() + 1).toFixed().padStart(2, '0') + \"-\" + dateObj.getDate().toFixed().padStart(2, '0');\n";
            std::cout << "    document.getElementById(var_name + '_timebox').value = dateObj.getHours().toFixed().padStart(2, '0') + \":\" + dateObj.getMinutes().toFixed().padStart(2, '0');\n";
            std::cout << "}\n\n";

            std::cout << "function showPopup(element_name){\n";
            std::cout << "    document.getElementById(element_name).classList.toggle(\"show\");\n";
            std::cout << "}\n\n";

            std::cout << "</script>\n";

            html.outputAdminMenu();

            std::string kml_file_list = "<option value=\"\"></option>";
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT CONCAT(directory,filename) FROM files WHERE RIGHT(filename, 4) = '.kml';");
            while (res->next()){
                std::string kml_file = res->getString(1);
                kml_file_list += "<option value=\"" + Encoder::htmlAttributeEncode(kml_file) + "\">" + Encoder::htmlEntityEncode(kml_file) + "</option>";
            }
            delete res;
            delete stmt;

            std::string gpx_file_list = "<option value=\"\"></option>";
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT CONCAT(directory,filename) FROM files WHERE RIGHT(filename, 4) = '.gpx';");
            while (res->next()) {
                std::string gpx_file = res->getString(1);
                gpx_file_list += "<option value=\"" + Encoder::htmlAttributeEncode(gpx_file) + "\">" + Encoder::htmlEntityEncode(gpx_file) + "</option>";
            }
            delete res;
            delete stmt;


            std::cout << "<h2 style=\"text-align:center\">JLWE website settings</h2>\n";
            std::cout << "<p id=\"page_note\" style=\"color:red;text-align:center;\"></p>";

            std::cout << "<h2 style=\"text-align:center\">Variables</h2>\n";
            std::cout << "<p><table align=\"center\"><tr>\n";
            std::cout << "<th>Name</th><th>Value</th><th></th>\n";
            std::cout << "</tr>\n";


            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT name,value,editable,comment FROM vars;");
            while (res->next()) {
                std::string var_name = res->getString(1);
                std::string comment = res->getString(4);
                std::string setFunctionName = "setTextVariable";
                bool editable = res->getInt(3); //&& jlwe.getPermissionValue("perm_admin");
                std::cout << "<tr>\n";
                std::cout << "<td>" << var_name << (comment.size() ? " <div class=\"popup\" onclick=\"showPopup('" + var_name + "_popup')\"><svg width=\"25\" height=\"25\"><image xlink:href=\"/img/info.svg\" width=\"25\" height=\"25\"/></svg><span class=\"popuptext\" id=\"" + var_name + "_popup\">" + Encoder::htmlEntityEncode(comment) + "</span></div>" : "" ) << "</td>\n";
                if (var_name.substr(var_name.length() - 5) == "_date") {
                    std::cout << "<td><input type=\"date\" id=\"" << var_name << "_datebox\" " << (editable ? "" : "disabled") << "> <input type=\"time\" id=\"" << var_name << "_timebox\" " << (editable ? "" : "disabled") << "><script>loadDateTime('" << var_name << "', '" << Encoder::javascriptAttributeEncode(res->getString(2)) << "');</script></td>\n";
                    setFunctionName = "setDateTimeVariable";
                } else if (var_name.substr(var_name.length() - 4) == "_kml") {
                    std::cout << "<td><select id=\"" << var_name << "_selectbox\" " << (editable ? "" : "disabled") << ">" << kml_file_list << "</select><script>document.getElementById('" << var_name << "_selectbox').value = '" << Encoder::javascriptAttributeEncode(res->getString(2)) << "';</script></td>\n";
                    setFunctionName = "setSelectVariable";
                } else if (var_name.substr(var_name.length() - 4) == "_gpx") {
                    std::cout << "<td><select id=\"" << var_name << "_selectbox\" " << (editable ? "" : "disabled") << ">" << gpx_file_list << "</select><script>document.getElementById('" << var_name << "_selectbox').value = '" << Encoder::javascriptAttributeEncode(res->getString(2)) << "';</script></td>\n";
                    setFunctionName = "setSelectVariable";
                } else if (var_name.substr(var_name.length() - 8) == "_enabled") {
                    std::cout << "<td>" << FormElements::radioButton(var_name + "_yes", var_name, "Yes", "yes", "", res->getString(2) == "1", !editable) << "&nbsp;&nbsp;&nbsp;" << FormElements::radioButton(var_name + "_no", var_name, "No", "no", "", res->getString(2) == "0", !editable) << "</td>\n";
                    setFunctionName = "setEnabledVariable";
                } else {
                    std::cout << "<td><input type=\"text\" id=\"" << var_name << "_textbox\" size=\"40\" value=\"" << Encoder::htmlAttributeEncode(res->getString(2)) << "\" " << (editable ? "" : "disabled") << "></td>\n";
                    setFunctionName = "setTextVariable";
                }
                if (editable) {
                    std::cout << "<td><input onclick=\"" << setFunctionName << "('" << var_name << "');\" type=\"button\" value=\"Save\" /></td>\n";
                } else {
                    std::cout << "<td></td>\n";
                }
                std::cout << "</tr>\n";
            }
            delete res;
            delete stmt;
            std::cout << "</table></p>\n";

            std::cout << "<h2 style=\"text-align:center\">Bonus zones</h2>\n";
            std::cout << "<p style=\"text-align:center\">Note that zone bonus points need to be set before any caches are entered.</p>\n";
            std::cout << "<p><table align=\"center\"><tr>\n";
            std::cout << "<th>KML file</th><th>Name</th><th>Points</th><th></th><th></th>\n";
            std::cout << "</tr>\n";

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT kml_file,name,points FROM zones;");
            while (res->next()){
                std::string kml_file = res->getString(1);
                bool editable = jlwe.getPermissionValue("perm_admin");
                std::cout << "<tr>\n";
                std::cout << "<td>" << kml_file << "</td>\n";
                std::cout << "<td><input type=\"text\" id=\"" << Encoder::htmlAttributeEncode(kml_file) << "_name_textbox\" size=\"30\" value=\"" << Encoder::htmlAttributeEncode(res->getString(2)) << "\" " << (editable ? "" : "disabled") << "></td>\n";
                std::cout << "<td><input type=\"number\" id=\"" << Encoder::htmlAttributeEncode(kml_file) << "_points\" size=\"10\" min=\"0\" max=\"100\" value=\"" << res->getInt(3) << "\" " << (editable ? "" : "disabled") << "></td>\n";
                if (editable) {
                    std::cout << "<td><input onclick=\"setZonePoints('" << Encoder::javascriptAttributeEncode(kml_file) << "');\" type=\"button\" value=\"Save\" /></td>\n";
                    std::cout << "<td><input onclick=\"deleteZone('" << Encoder::javascriptAttributeEncode(kml_file) << "');\" type=\"button\" value=\"Delete\" /></td>\n";
                } else {
                    std::cout << "<td></td><td></td>\n";
                }
                std::cout << "</tr>\n";
            }
            delete res;
            delete stmt;
            std::cout << "</table></p>\n";

            if (jlwe.getPermissionValue("perm_admin")) {
                std::cout << "<p style=\"text-align:center;\">Add a new zone: \n";
                std::cout << "<select id=\"add_zone_kmls\">" << kml_file_list << "\n";
                std::cout << "</select> <button onclick=\"addZone();\" type=\"button\">Add</button>\n";
                std::cout << "</p>\n";
            }

        }else{
            std::cout << "<p>You need to be logged in to view this area.</p>";
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
