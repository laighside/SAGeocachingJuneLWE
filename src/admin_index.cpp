/**
  @file    admin_index.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the webpage at /cgi-bin/admin_index.cgi
  This page is the landing page after logging in, it contains links to other admin tools and the contents of a user guide markdown file.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream> // cout
#include <string>

#include "core/CgiEnvironment.h"
#include "core/HtmlTemplate.h"
#include "core/JlweCore.h"
#include "core/JlweUtils.h"
#include "ext/md4c/md4c-html.h"

// This function takes the output from the markdown parser and appends it to a string
static void md4c_process_output(const MD_CHAR* text, MD_SIZE size, void* userdata)
{
    std::string *str_out = reinterpret_cast<std::string*>(userdata);
    *str_out += std::string(text, size);
}

int main () {
    try {
        JlweCore jlwe;

        HtmlTemplate html(false);
        html.outputHttpHtmlHeader();
        if (!html.outputHeader(&jlwe, "JLWE Admin area", false))
            return 0;

        if (jlwe.isLoggedIn()){ // if logged in

            html.outputAdminMenu();

            // Read markdown file, process it into HTML and output it
            std::string md_text = JlweUtils::readFileToString(std::string(std::string(jlwe.config.at("files").at("directory")) + jlwe.getGlobalVar("admin_index_md")).c_str());
            std::string str_out = "";

            int ret = md_html(md_text.c_str(), md_text.size(), md4c_process_output, reinterpret_cast<void*>(&str_out), 0, 0);
            if (ret != 0) {
                str_out = "<p>Markdown Parsing failed</p>";
            }

            std::cout << "<div id=\"md_content\">\n";
            std::cout << str_out;
            std::cout << "</div>\n";

            // Output server information
            std::string mysql_version;
            sql::Statement *stmt = jlwe.getMysqlCon()->createStatement();
            sql::ResultSet *res = stmt->executeQuery("SELECT VERSION();");
            if (res->next()){
                mysql_version = res->getString(1);
            }
            delete res;
            delete stmt;

            std::cout << "<h3>Server Info</h3>\n";
            std::cout << "<p>HTTP Server: " << CgiEnvironment::getServerSoftware() << "<br />\n";
            std::cout << "Server name: " << CgiEnvironment::getServerName() << "<br />\n";
            std::cout << "MySQL version: " << mysql_version << "<br />\n";
            std::cout << "Database name: " << std::string(jlwe.config.at("mysql").at("database")) << "<br />\n";
            std::cout << "JLWE Software Build: " << __DATE__ << "</p>\n";

            // Links to log files
            std::cout << "<p style=\"text-align:center;\"><a href=\"/cgi-bin/log/sendmail.cgi\">View sendmail log</a></p>";
            std::cout << "<p style=\"text-align:center;\"><a href=\"/cgi-bin/log/user.cgi\">View user access log</a></p>";
            std::cout << "<p style=\"text-align:center;\"><a href=\"/cgi-bin/log/server_status.cgi\">Apache Server Status</a></p>";
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
