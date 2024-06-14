/**
  @file    gpx_builder.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/gpx_builder/gpx_builder.cgi
  This main page for the GPX Builder. Shows the list of caches, user submitted caches and cache handout list.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <vector>

#include "../core/Encoder.h"
#include "../core/FormElements.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
        sql::ResultSet *res;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area - GPX Builder", false))
            return 0;

        if (jlwe.getPermissionValue("perm_gpxbuilder")) { // if logged in

            // Check that number_game_caches is set to a valid value
            int number_game_caches = 0;
            try {
                number_game_caches = std::stoi(jlwe.getGlobalVar("number_game_caches"));
            } catch (...) {}
            if (number_game_caches < 1)
                throw std::invalid_argument("Invalid setting for number_game_caches = " + std::to_string(number_game_caches));

            html.outputAdminMenu();

            std::cout << FormElements::includeJavascript("/cgi-bin/js_files.cgi");
            std::cout << FormElements::includeJavascript("/js/utils.js");
            std::cout << FormElements::includeJavascript("/js/geo_maths.js");
            std::cout << FormElements::includeJavascript("/js/gpx_map.js");
            std::cout << FormElements::includeJavascript("/js/page_tab_tools.js");

            std::cout << "<h2 style=\"text-align:center\">JLWE Cache list and GPX builder</h2>\n";
            std::cout << "<p id=\"page_note\" style=\"color:red;text-align:center;\"></p>";

            std::cout << "<p style=\"text-align:center;\">\n";
            std::cout << "<a class=\"admin_button\" href=\"edit_caches.cgi\"><span>Edit Caches</span></a>\n";
            std::cout << "<a class=\"admin_button\" href=\"download_gpx.cgi\"><span>Download GPX</span></a>\n";
            std::cout << "<a class=\"admin_button\" href=\"/cache_list.html\"><span style=\"font-size:22px;\">Download Cache List<br/><span style=\"font-size:16px;\">(printable)</span></span></a>\n";
            std::cout << "</p>\n";

            std::cout << "<p>Before downloading the GPX file, ensure gpx_code_prefix is set correctly in <a href=\"/cgi-bin/settings/settings.cgi\">Website settings</a>. The code prefix needs to be unique - if not, some GPS units will fail to read the GPX file. So don't reuse codes from past years and don't use common ones like GC, GA or TP.</p>\n";

            std::cout << "<script>\n";
            std::cout << "var deleted_cache_number = 0;\n";
            std::cout << "function deleteCache(cache_number) {\n";
            std::cout << "    if (confirm(\"Are you sure you wish to delete this cache?\") == true) {\n";
            std::cout << "        deleted_cache_number = cache_number;\n";
            std::cout << "        var jsonObj = {\n";
            std::cout << "            \"id_number\":cache_number\n";
            std::cout << "        };\n";
            std::cout << "        postUrl('delete_cache.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "            function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, function() {deleteCacheRow(deleted_cache_number);}, null);\n";
            std::cout << "         }, httpErrorResponseHandler);\n";
            std::cout << "    }\n";
            std::cout << "}\n";

            std::cout << "function deleteCacheRow(cache_number) {\n";
            std::cout << "    var index = document.getElementById('row_' + cache_number.toString()).rowIndex;\n";
            std::cout << "    document.getElementById('cacheTable').deleteRow(index);\n";
            std::cout << "}\n";
            std::cout << "</script>\n";

            std::cout << FormElements::pageTabs({{"cache_list", "GPX Cache List"}, /*{"cache_map", "Map"},*/ {"user_caches", "User submitted caches"}, {"cache_handout", "Cache Handout/Return"}});

            std::cout << "<div id=\"cache_list\" class=\"pageTabContent\">\n";

            std::cout << "<h2 style=\"text-align:center\">Caches</h2>\n";
            std::cout << "<p style=\"text-align:center\">(<a href=\"builder_map.cgi\">view on map</a>)</p>\n";
            std::cout << "<table id=\"cacheTable\" class=\"reg_table\" align=\"center\"><tr>\n";
            std::cout << "<th>Cache</th><th>Team</th><th>Coordinates</th><th>Public hint</th><th>Walk</th><th></th>\n"; //<th>Creative?</th><th>Permanent?</th>
            std::cout << "</tr>\n";

            std::vector<bool> caches_ready;
            caches_ready.reserve(number_game_caches);
            for (int i = 0; i < number_game_caches; i++)
                caches_ready.push_back(false);

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT cache_number,cache_name,team_name,latitude,longitude,public_hint,detailed_hint,camo,permanent,osm_distance,actual_distance FROM caches ORDER BY cache_number;");
            while (res->next()) {
                int cache_number = res->getInt(1);
                std::cout << "<tr id=\"row_" << cache_number << "\">\n";

                if (cache_number > 0 && cache_number <= caches_ready.size())
                    caches_ready.at(static_cast<size_t>(cache_number - 1)) = true;

                std::cout << "<td>" << Encoder::htmlEntityEncode(JlweUtils::makeFullCacheName(cache_number, res->getString(2).substr(0, 30), res->getInt(8), res->getInt(9))) << "</td>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(3).substr(0, 30)) << "</td>\n";
                // Don't Entity Encode the Coordinate String, it has &nbsp; in it
                std::cout << "<td>" << JlweUtils::makeCoordString(res->getDouble(4), res->getDouble(5), true) << "</td>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(6).substr(0, 30)) << "</td>\n";

                int walking = res->getInt(11);
                if (walking < 0)
                    walking = res->getInt(10);
                std::cout << "<td>" << walking << "m</td>\n";

                /*if (res->getInt(8)){
                    cout << "<td>Yes</td>\n";
                }else{
                    cout << "<td>No</td>\n";
                }
                if (res->getInt(9)){
                    cout << "<td>Yes</td>\n";
                }else{
                    cout << "<td>No</td>\n";
                }*/

                std::vector<FormElements::dropDownMenuItem> menuItems;
                menuItems.push_back({"location.href='edit_caches.cgi?cache=" + std::to_string(cache_number) + "'", "Edit", true});
                menuItems.push_back({"deleteCache(" + std::to_string(res->getInt(1)) + ")", "Delete", true});
                std::cout << "<td>" << FormElements::dropDownMenu(res->getInt(1), menuItems) << "</td>\n";
                std::cout << "</tr>\n";
            }
            delete res;
            delete stmt;
            std::cout << "</table>\n";

            std::string missing_caches_html = "<p>Caches yet to be entered into this list: ";
            bool any_missing = false;
            for (unsigned int i = 0; i < caches_ready.size(); i++) {
                if (caches_ready.at(i) == false) {
                    missing_caches_html += std::to_string(i + 1) + ", ";
                    any_missing = true;
                }
            }
            missing_caches_html = missing_caches_html.substr(0, missing_caches_html.size() - 2) + "</p>";
            if (any_missing)
                std::cout << missing_caches_html;

            std::cout << "</div>\n";

            std::cout << "<div id=\"user_caches\" class=\"pageTabContent\">\n";

            std::cout << "<h2 style=\"text-align:center\">User submitted caches</h2>\n";
            std::cout << "<p>These are caches that have been submitted via the online form. Review each entry to check for errors and contact the owner if necessary to clarify details. These caches will NOT appear in the GPX file until you review them.</p>\n";
            std::cout << "<p><table class=\"reg_table\" align=\"center\"><tr>\n";
            std::cout << "<th>Cache</th><th>Team</th><th>Phone number</th><th>Coordinates</th><th>Public hint</th><th></th>\n"; //<th>Public hint</th>
            std::cout << "</tr>\n";

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT id_number,cache_number,cache_name,team_name,phone_number,latitude,longitude,public_hint,detailed_hint,camo,permanent FROM user_hidden_caches WHERE status = 'R' ORDER BY id_number;");
            while (res->next()){
                std::cout << "<tr>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(JlweUtils::makeFullCacheName(res->getInt(2), res->getString(3).substr(0, 30), res->getInt(10), res->getInt(11))) << "</td>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(4).substr(0, 30)) << "</td>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(5).substr(0, 15)) << "</td>\n";
                std::cout << "<td>" << JlweUtils::makeCoordString(res->getDouble(6), res->getDouble(7), true) << "</td>\n";
                std::cout << "<td>" << Encoder::htmlEntityEncode(res->getString(8).substr(0, 30)) << "</td>\n";

                std::cout << "<td><button onclick=\"location.href='edit_caches.cgi?reviewCacheId=" << res->getString(1) << "'\" type=\"button\">Review</button></td>\n";
                std::cout << "</tr>\n";
            }
            delete res;
            delete stmt;
            std::cout << "</table></p>\n";

            std::cout << "</div>\n";

            std::cout << "<div id=\"cache_handout\" class=\"pageTabContent\">\n";

            std::cout << "<h2 style=\"text-align:center\">Cache handout</h2>\n";

            //cout << "<p>This feature is disabled since it relies on registration data which is currently disabled.</p>\n";
            std::cout << "<p>This page allows you to record who was given which cache. You can choose names from the list below or enter any name into the box. To set a cache to un-owned, just remove all text in the name box and click link. When caches are returned on Sunday afternoon, use the Set Returned button to record the caches returned (this data then goes into the scoring). The owner list can be printed by using the \"Download Cache List\" button above.</p>\n";
            std::cout << "<p>The meaning of the colors for the caches are: Yellow = un-allocated cache, Grey = allocated to an owner, Green = cache returned.<br />\n";
            std::cout << "And for the teams/cachers: Yellow: no caches given to this team, Grey: one or more caches given to this team.</p>\n";


            // this has to be done first so caches_allocated and owner_list are set
            std::string handout_table_html = "";
            handout_table_html += "<table id=\"handoutTable\" class=\"reg_table\" align=\"center\"><tr>\n";
            handout_table_html += "<th>Cache</th><th>Owner</th><th>Handout</th><th>Coordinates</th><th>Cache return</th><th></th>\n"; //<th>Creative?</th><th>Permanent?</th>
            handout_table_html += "</tr>\n";

            std::vector<bool> caches_allocated;
            caches_allocated.reserve(number_game_caches);
            for (int i = 0; i < number_game_caches; i++)
                caches_allocated.push_back(false);
            std::vector<bool> caches_returned;
            caches_returned.reserve(number_game_caches);
            for (int i = 0; i < number_game_caches; i++)
                caches_returned.push_back(false);

            std::vector<std::string> owner_list;
            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT cache_number,owner_name,returned FROM cache_handout ORDER BY cache_number;");
            while (res->next()){
                int cache_number = res->getInt(1);
                std::string owner_name = res->getString(2);
                JlweUtils::trimString(owner_name);
                handout_table_html += "<tr id=\"handout_row_" + std::to_string(cache_number) + "\">\n";

                handout_table_html += "<td>" + std::to_string(cache_number) + "</td>\n";
                handout_table_html += "<td>" + Encoder::htmlEntityEncode(owner_name) + "</td>\n";

                if (owner_name.size()) {
                    if (cache_number > 0 && cache_number <= caches_allocated.size())
                        caches_allocated.at(cache_number - 1) = true;
                    owner_list.push_back(owner_name);
                    handout_table_html += "<td style=\"text-align:center\">\u2714</td>\n";
                } else {
                    handout_table_html += "<td style=\"text-align:center\"></td>\n";
                }
                if (cache_number > 0 && cache_number <= caches_ready.size() && caches_ready.at(cache_number - 1)) {
                    handout_table_html += "<td style=\"text-align:center\">\u2714</td>\n";
                } else {
                    handout_table_html += "<td style=\"text-align:center\"></td>\n";
                }
                if (res->getInt(3)) {
                    if (cache_number > 0 && cache_number <= caches_returned.size())
                        caches_returned.at(cache_number - 1) = true;
                    handout_table_html += "<td style=\"text-align:center\">\u2714</td>\n";
                } else {
                    handout_table_html += "<td style=\"text-align:center\"></td>\n";
                }

                std::vector<FormElements::dropDownMenuItem> menuItems;
                menuItems.push_back({"setCacheOwner(" + std::to_string(cache_number) + ", '')", "Clear Owner", true});
                menuItems.push_back({"setCacheReturned(" + std::to_string(cache_number) + ", false)", "Clear Returned", true});
                handout_table_html += "<td>" + FormElements::dropDownMenu(2000 + res->getInt(1), menuItems) + "</td>\n";

                handout_table_html += "</tr>\n";
            }
            delete res;
            delete stmt;
            handout_table_html += "</table>\n";

            std::cout << "<table class=\"no_border\" align=\"center\"><tr>\n";
            std::cout << "<th style=\"text-align:right;\"><input type=\"text\" id=\"handout_owner_textbox\" style=\"width:300px;\" /></th>\n";
            std::cout << "<th><input type=\"button\" value=\"&lt;- Link -&gt;\" onclick=\"linkCacheToOwner()\" /></th>\n";
            std::cout << "<th style=\"text-align:left;\"><input type=\"number\" id=\"handout_cache_textbox\" min=\"1\" max=\"" << number_game_caches << "\" /></th>\n";
            std::cout << "<th style=\"text-align:right;\"><input type=\"button\" value=\"Set Returned\" onclick=\"linkCacheReturned()\" /></th></tr>\n";

            std::cout << "<tr>\n";
            std::cout << "<td style=\"text-align:center;vertical-align:top;\">\n";

            stmt = jlwe.getMysqlCon()->createStatement();
            res = stmt->executeQuery("SELECT DISTINCT(gc_username) FROM event_registrations WHERE status = 'S' ORDER BY gc_username;");
            while (res->next()) {
                std::string owner_name = res->getString(1);
                JlweUtils::trimString(owner_name);
                bool owner_allocated = false;
                for (int i = 0; i < owner_list.size(); i++)
                    if (JlweUtils::compareStringsNoCase(owner_list.at(i), owner_name))
                        owner_allocated = true;
                std::cout << "<button class=\"ownerIcon" << (owner_allocated ? " allocatedIcon" : "") << "\" onclick=\"setOwnerToLink('" << Encoder::javascriptAttributeEncode(owner_name) << "')\">" << Encoder::htmlEntityEncode(owner_name) << "</button><br />";
            }
            delete res;
            delete stmt;

            std::cout << "</td>\n";
            std::cout << "<td></td>\n";

            std::cout << "<td style=\"text-align:center;vertical-align:top;\"  colspan=\"2\">\n";
            for (int i = 0; i < (number_game_caches / 10 + ((number_game_caches % 10) > 0 ? 1 : 0)); i++) {
                for (int j = 1; j <= 10; j++) {
                    int cache_number = i * 10 + j;
                    if (cache_number <= number_game_caches)
                        std::cout << "<button class=\"cacheIcon" << (caches_allocated.at(cache_number - 1) ? " allocatedIcon" : "") << (caches_returned.at(cache_number - 1) ? " returnedIcon" : "") << "\" id=\"cache_icon_" << cache_number << "\" onclick=\"setCacheToLink(" << cache_number << ")\">" << cache_number << "</button>";
                }
                std::cout << "<br />";
            }
            std::cout << "</td>\n";

            std::cout << "</tr>\n";

            std::cout << "</table>\n";

            std::cout << handout_table_html;

            std::cout << "</div>\n";

            std::cout << FormElements::includeJavascript("/js/menu.js");

            std::cout << "<script type=\"text/javascript\">\n";
            std::cout << "document.getElementsByClassName(\"defaultPageTab\")[0].click();\n";


            std::cout << "function setCacheToLink(cache_number) {\n";
            std::cout << "    document.getElementById(\"handout_cache_textbox\").value = cache_number;\n";
            std::cout << "}\n\n";
            std::cout << "function setOwnerToLink(owner_name) {\n";
            std::cout << "    document.getElementById(\"handout_owner_textbox\").value = owner_name;\n";
            std::cout << "}\n\n";

            std::cout << "function linkCacheToOwner() {\n";
            std::cout << "    var new_cache_number = parseInt(document.getElementById(\"handout_cache_textbox\").value);\n";
            std::cout << "    var new_owner = document.getElementById(\"handout_owner_textbox\").value;\n";
            std::cout << "    setCacheOwner(new_cache_number, new_owner);";
            std::cout << "}\n\n";

            std::cout << "function setCacheOwner(cache_number, new_owner) {\n";
            std::cout << "    var jsonObj = {\n";
            std::cout << "        \"cache_number\":cache_number,\n";
            std::cout << "        \"linkCache\":{\n";
            std::cout << "            \"owner\":new_owner\n";
            std::cout << "        }\n";
            std::cout << "    };\n";

            std::cout << "        postUrl('set_handout_cache.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "        function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, function(jsonResponse) {\n";
            std::cout << "                        if (jsonResponse.cache_number && (typeof jsonResponse.owner === 'string' || jsonResponse.owner instanceof String)) {\n";
            std::cout << "                            var tableRow = document.getElementById(\"handout_row_\" + jsonResponse.cache_number.toString());\n";
            std::cout << "                            tableRow.cells[1].innerText = jsonResponse.owner;\n";
            std::cout << "                            tableRow.cells[2].innerText = (jsonResponse.owner.length > 0) ? \"\\u2714\" : \"\";\n";
            std::cout << "                            var cacheIcon = document.getElementById(\"cache_icon_\" + jsonResponse.cache_number.toString());\n";
            std::cout << "                            if (jsonResponse.owner.length > 0) {\n";
            std::cout << "                                cacheIcon.classList.add(\"allocatedIcon\");\n";
            std::cout << "                            } else {\n";
            std::cout << "                                cacheIcon.classList.remove(\"allocatedIcon\");\n";
            std::cout << "                            }\n";
            std::cout << "                        }\n";
            std::cout << "                    }, null);\n";
            std::cout << "         }, httpErrorResponseHandler);\n";
            std::cout << "}\n\n";

            std::cout << "function linkCacheToOwner() {\n";
            std::cout << "    var new_cache_number = parseInt(document.getElementById(\"handout_cache_textbox\").value);\n";
            std::cout << "    var new_owner = document.getElementById(\"handout_owner_textbox\").value;\n";
            std::cout << "    setCacheOwner(new_cache_number, new_owner);\n";
            std::cout << "}\n\n";

            std::cout << "function setCacheReturned(cache_number, new_returned) {\n";
            std::cout << "    var jsonObj = {\n";
            std::cout << "        \"cache_number\":cache_number,\n";
            std::cout << "        \"returnCache\":{\n";
            std::cout << "            \"returned\":new_returned\n";
            std::cout << "        }\n";
            std::cout << "    };\n";

            std::cout << "        postUrl('set_handout_cache.cgi', JSON.stringify(jsonObj), null,\n";
            std::cout << "        function(data, responseCode) {\n";
            std::cout << "                httpResponseHandler(data, responseCode, false, function(jsonResponse) {\n";
            std::cout << "                        if (jsonResponse.cache_number && typeof jsonResponse.returned === 'boolean') {\n";
            std::cout << "                            var tableRow = document.getElementById(\"handout_row_\" + jsonResponse.cache_number.toString());\n";
            std::cout << "                            tableRow.cells[4].innerText = jsonResponse.returned ? \"\\u2714\" : \"\";\n";
            std::cout << "                            var cacheIcon = document.getElementById(\"cache_icon_\" + jsonResponse.cache_number.toString());\n";
            std::cout << "                            if (jsonResponse.returned) {\n";
            std::cout << "                                cacheIcon.classList.add(\"returnedIcon\");\n";
            std::cout << "                            } else {\n";
            std::cout << "                                cacheIcon.classList.remove(\"returnedIcon\");\n";
            std::cout << "                            }\n";
            std::cout << "                        }\n";
            std::cout << "                    }, null);\n";
            std::cout << "         }, httpErrorResponseHandler);\n";
            std::cout << "}\n\n";

            std::cout << "function linkCacheReturned() {\n";
            std::cout << "    var new_cache_number = parseInt(document.getElementById(\"handout_cache_textbox\").value);\n";
            std::cout << "    setCacheReturned(new_cache_number, true);\n";
            std::cout << "}\n\n";

            /*cout << "function setCacheIconStatus(cache_number, allocated) {\n";
            cout << "    if (allocated) {\n";
            cout << "         document.getElementById(\"cache_icon_\" + cache_number.toString()).classList.add(\"allocatedIcon\");\n";
            cout << "    } else {\n";
            cout << "         document.getElementById(\"cache_icon_\" + cache_number.toString()).classList.remove(\"allocatedIcon\");\n";
            cout << "    }\n";
            cout << "}\n\n";*/

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
