/**
  @file    users.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/users/users.cgi
  This page allows admins to view the list of users and change user preferences and permissions.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/Encoder.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"

static inline std::string boolToChecked(bool checked) {
    if (checked) {
        return "checked=\"true\"";
    } else{
        return "";
    }
}

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "Users", false))
            return 0;

        if (jlwe.isLoggedIn()) { //if logged in
            std::cout << "<script type=\"text/javascript\" src=\"/js/utils.js\"></script>\n";
            std::cout << "<script type=\"text/javascript\">\n";

            std::cout << "function setUserPerm(user, perm_name){\n";
            std::cout << "    var cb = document.getElementById(\"checkbox_\" + user + \"_\" + perm_name);\n";
            std::cout << "    var jsonObj = {\n";
            std::cout << "        \"user\":user,\n";
            std::cout << "        \"perm_name\":perm_name,\n";
            std::cout << "        \"setting\": cb.checked\n";
            std::cout << "    };\n";

            std::cout << "    postUrl('set_user_perm.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "        function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, true, null, null);\n";
            std::cout << "         }, httpErrorResponseHandler);\n";
            std::cout << "}\n\n";

            std::cout << "function sendPasswordReset(email){\n";
            std::cout << "    var jsonObj = {\n";
            std::cout << "        \"email\":email\n";
            std::cout << "    };\n";

            std::cout << "    postUrl('/cgi-bin/password/send_reset.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "        function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, null, null);\n";
            std::cout << "         }, httpErrorResponseHandler);\n";
            std::cout << "}\n\n";

            std::cout << "function updatePreferences(){\n";
            std::cout << "    document.getElementById(\"reg_type_dsp\").style.display = (document.getElementById(\"daily_reg\").checked == true) ? \"inline\" : \"none\";\n";
            std::cout << "    document.getElementById(\"merch_type_dsp\").style.display = (document.getElementById(\"daily_merch\").checked == true) ? \"inline\" : \"none\";\n";
            std::cout << "    var jsonObj = {\n";
            std::cout << "        \"daily_reg\":(document.getElementById(\"daily_reg\").checked == true),\n";
            std::cout << "        \"reg_type\":(document.getElementById(\"reg_type_simple\").checked == true) ? 'S' : 'F',\n";
            std::cout << "        \"every_reg\":(document.getElementById(\"every_reg\").checked == true),\n";
            std::cout << "        \"daily_merch\":(document.getElementById(\"daily_merch\").checked == true),\n";
            std::cout << "        \"merch_type\":(document.getElementById(\"merch_type_simple\").checked == true) ? 'S' : 'F',\n";
            std::cout << "        \"every_merch\":(document.getElementById(\"every_merch\").checked == true)\n";
            std::cout << "    };\n";

            std::cout << "    postUrl('set_user_preferences.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "        function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, true, null, null);\n";
            std::cout << "         }, httpErrorResponseHandler);\n";

            std::cout << "}\n\n";

            std::cout << "</script>\n";

            html.outputAdminMenu();

            std::cout << "<p id=\"page_note\" style=\"color:red;text-align:center;\"></p>";

            std::cout << "<h2 style=\"text-align:center\">Preferences</h2>\n";

            std::string reg_daily = "N";
            bool reg_every = false;
            std::string merch_daily = "N";
            bool merch_every = false;

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT email_reg_daily, email_reg_every, email_merch_daily, email_merch_every FROM user_preferences WHERE user_id = ?;");
            prep_stmt->setInt(1, jlwe.getCurrentUserId());
            res = prep_stmt->executeQuery();
            while (res->next()) {
                reg_daily = res->getString(1);
                reg_every = res->getInt(2);
                merch_daily = res->getString(3);
                merch_every = res->getInt(4);
            }
            delete res;
            delete prep_stmt;

            std::cout << "<p style=\"text-align:center\">Select which notification emails you wish to receive:</p>\n";

            std::cout << "<p style=\"text-align:center\">\n";
            std::cout << "<span class=\"checkbox_container\"><label>Daily Registrations Update\n";
            std::cout << "  <input type=\"checkbox\" id=\"daily_reg\" name=\"daily_reg\" value=\"true\" oninput=\"updatePreferences();\" " << ((reg_daily == "N") ? "" : "checked") << "/>\n";
            std::cout << "  <span class=\"checkmark\"></span>\n";
            std::cout << "</label></span><br />\n";
            std::cout << "<span id=\"reg_type_dsp\">\n";
            std::cout << "Type: \n";
            std::cout << "  <span class=\"checkbox_container\"><label>Full\n";
            std::cout << "    <input type=\"radio\" name=\"reg_type\" id=\"reg_type_full\" value=\"full\" oninput=\"updatePreferences();\" " << ((reg_daily == "F") ? "checked" : "") << "/>\n";
            std::cout << "    <span class=\"radiobox\"></span>\n";
            std::cout << "  </label></span>&nbsp;&nbsp;&nbsp;\n";
            std::cout << "  <span class=\"checkbox_container\"><label>Simple\n";
            std::cout << "    <input type=\"radio\" name=\"reg_type\" id=\"reg_type_simple\" value=\"simple\" oninput=\"updatePreferences();\" " << ((reg_daily == "S") ? "checked" : "") << "/>\n";
            std::cout << "    <span class=\"radiobox\"></span>\n";
            std::cout << "  </label></span>\n";
            std::cout << "</span>\n";
            std::cout << "</p>\n";

            std::cout << "<p style=\"text-align:center\">\n";
            std::cout << "<span class=\"checkbox_container\"><label>Email for every new registration\n";
            std::cout << "  <input type=\"checkbox\" id=\"every_reg\" name=\"every_reg\" value=\"true\" oninput=\"updatePreferences();\" " << (reg_every ? "checked" : "") << "/>\n";
            std::cout << "  <span class=\"checkmark\"></span>\n";
            std::cout << "</label></span>\n";
            std::cout << "</p>\n";

            std::cout << "<p style=\"text-align:center\">\n";
            std::cout << "<span class=\"checkbox_container\"><label>Daily Merchandise Orders Update\n";
            std::cout << "  <input type=\"checkbox\" id=\"daily_merch\" name=\"daily_merch\" value=\"true\" oninput=\"updatePreferences();\" " << ((merch_daily == "N") ? "" : "checked") << "/>\n";
            std::cout << "  <span class=\"checkmark\"></span>\n";
            std::cout << "</label></span><br />\n";
            std::cout << "<span id=\"merch_type_dsp\">\n";
            std::cout << "Type: \n";
            std::cout << "  <span class=\"checkbox_container\"><label>Full\n";
            std::cout << "    <input type=\"radio\" name=\"merch_type\" id=\"merch_type_full\" value=\"full\" oninput=\"updatePreferences();\" " << ((merch_daily == "F") ? "checked" : "") << "/>\n";
            std::cout << "    <span class=\"radiobox\"></span>\n";
            std::cout << "  </label></span>&nbsp;&nbsp;&nbsp;\n";
            std::cout << "  <span class=\"checkbox_container\"><label>Simple\n";
            std::cout << "    <input type=\"radio\" name=\"merch_type\" id=\"merch_type_simple\" value=\"simple\" oninput=\"updatePreferences();\" " << ((merch_daily == "S") ? "checked" : "") << "/>\n";
            std::cout << "    <span class=\"radiobox\"></span>\n";
            std::cout << "  </label></span>\n";
            std::cout << "</span>\n";
            std::cout << "</p>\n";

            std::cout << "<p style=\"text-align:center\">\n";
            std::cout << "<span class=\"checkbox_container\"><label>Email for every new merchandise order\n";
            std::cout << "  <input type=\"checkbox\" id=\"every_merch\" name=\"every_merch\" value=\"true\" oninput=\"updatePreferences();\" " << (merch_every ? "checked" : "") << "/>\n";
            std::cout << "  <span class=\"checkmark\"></span>\n";
            std::cout << "</label></span>\n";
            std::cout << "</p>\n";

            std::cout << "<h2 style=\"text-align:center\">Users</h2>\n";
            std::cout << "<p><table align=\"center\"><thead>\n";
            std::cout << "<tr><th>Name</th><th>Email</th><th></th><th>Active</th><th>Last IP</th></tr>\n";
            std::cout << "</thead>\n";

            std::cout << "<script type=\"text/javascript\">\n";
            std::cout << "    document.getElementById(\"reg_type_dsp\").style.display = (document.getElementById(\"daily_reg\").checked == true) ? \"inline\" : \"none\";\n";
            std::cout << "    document.getElementById(\"merch_type_dsp\").style.display = (document.getElementById(\"daily_merch\").checked == true) ? \"inline\" : \"none\";\n";
            std::cout << "</script>\n";

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT user_id,username,email,active,(SELECT ip_address FROM user_tokens WHERE users.username=user_tokens.username ORDER BY user_tokens.expire_time DESC LIMIT 1) FROM users;");
            std::vector<std::string> usernames;
            while (res->next()) {
                int user_id = res->getInt(1);
                std::string table_username = res->getString(2);
                usernames.push_back(table_username);
                std::string table_email = res->getString(3);
                std::cout << "<tr>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(table_username) << "</td>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(table_email) << "</td>\n";
                if (table_email.size() && (jlwe.getPermissionValue("perm_admin") || jlwe.getCurrentUserId() == user_id)) {
                    std::cout << "<td><button onclick=\"sendPasswordReset('" << Encoder::javascriptAttributeEncode(res->getString(3)) << "')\" type=\"button\">Reset Password</button></td>\n";
                } else {
                    std::cout << "<td></td>\n";
                }
                if (res->getInt(4)) {
                    std::cout << "<td>Yes</td>\n";
                } else {
                    std::cout << "<td>No</td>\n";
                }
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(5)) << "</td>\n";
                std::cout << "</tr>\n";
            }
            delete res;
            delete stmt;
            std::cout << "</table></p>\n";

            if (jlwe.getPermissionValue("perm_admin")) {
                stmt = jlwe.getMysqlCon()->createStatement();
                res = stmt->executeQuery("SELECT permission_id, permission_name FROM permission_list;");
                std::vector<std::string> permission_ids;
                std::string permission_header = "<tr><th>Username</th>";
                std::string permission_id_list = "";
                while (res->next()) {
                    if (permission_ids.size() > 0)
                        permission_id_list += ", ";
                    permission_ids.push_back(res->getString(1));
                    permission_id_list += "'" + res->getString(1) + "'";
                    permission_header += "<th>" + Encoder::htmlEntityEncode(res->getString(2)) +"</th>";
                }
                permission_header += "</tr>\n";
                delete res;
                delete stmt;


                std::cout << "<h2 style=\"text-align:center\">User Permissions</h2>\n";
                std::cout << "<p><table align=\"center\"><thead>\n";
                std::cout << permission_header;
                std::cout << "</thead>\n";

                prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT permission FROM user_permissions WHERE user = ? AND value = 1 AND permission IN (" + permission_id_list + ") ORDER BY FIELD(permission, " + permission_id_list + ");");

                for (unsigned int i = 0; i < usernames.size(); i++) {
                    std::cout << "<tr>\n";
                    std::string table_username = usernames.at(i);
                    std::cout << "<td>" << table_username << "</td>\n";


                    prep_stmt->setString(1, table_username);
                    res = prep_stmt->executeQuery();
                    bool sql_valid = res->next();

                    std::string checkboxEnabled = "";
                    if (table_username == "admin" || table_username == jlwe.getCurrentUsername()) // can't edit admin or your own permissions
                        checkboxEnabled = " disabled=\"true\"";
                    for (unsigned int j = 0; j < permission_ids.size(); j++) {

                        bool checked = false;
                        if (sql_valid && res->getString(1) == permission_ids.at(j)) {
                            sql_valid = res->next();
                            checked = true;
                        }

                        std::cout << "<td style=\"text-align:center\"><span class=\"checkbox_container\"><label>\n";
                        std::cout << "<input type=\"checkbox\" id=\"checkbox_" << table_username << "_" << permission_ids.at(j) << "\" " << boolToChecked(checked) << checkboxEnabled << " onclick='setUserPerm(\"" << table_username << "\", \"" << permission_ids.at(j) << "\");' />\n";
                        std::cout << "<span class=\"checkmark\"></span></label></span></td>\n";
                    }

                    delete res;

                    std::cout << "</tr>\n";
                }
                delete prep_stmt;

                std::cout << "</table></p>\n";
            }

            std::cout << "<p style=\"text-align:center;\"><a href=\"/cgi-bin/log/user.cgi\">View user access log</a></p>";
        } else {
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
