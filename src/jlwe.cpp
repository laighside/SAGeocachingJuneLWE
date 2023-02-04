/**
  @file    jlwe.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This script handles all the requests to the public pages of the website
  ModRewrite redirects most requests to this script
  The public HTML content is mostly stored in the MySQL database, but can be in files to

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <string>
#include <dirent.h>

#include "core/CgiEnvironment.h"
#include "core/HtmlTemplate.h"
#include "core/JlweCore.h"
#include "core/JlweUtils.h"
#include "core/KeyValueParser.h"

int main () {
    try {
        JlweCore jlwe;

        std::string doc_root = CgiEnvironment::getDocumentRoot();
        std::string page_request = CgiEnvironment::getRequestUri();

        // remove any url arguments (like fbclid)
        if (page_request.find("?") != std::string::npos)
            page_request = page_request.substr(0, page_request.find("?"));

        // default to index.html page
        if (page_request.length() < 1)
            page_request = "/";
        if (page_request.substr(page_request.length() - 1, 1) == "/"){
            page_request = page_request + "index.html";
        }

        // stop directory traversal attacks - this could be done better
        while (page_request.find("../") != std::string::npos) {
            page_request = JlweUtils::replaceString(page_request, "../", "");
            page_request = JlweUtils::replaceString(page_request, "..\\", "");
            page_request = JlweUtils::replaceString(page_request, "..", "");
        }
        while (page_request.find("..") != std::string::npos) {
            page_request = JlweUtils::replaceString(page_request, "..", "");
        }


        sql::PreparedStatement *prep_stmt;
        sql::ResultSet *res;

        std::string html_page = "";
        std::string title = "June LWE - Page not found";
        bool note = false;
        bool login_only = false;

        HtmlTemplate html(true);
        html.outputHttpHtmlHeader();


        // look on filesystem before mysql (it needs to be this way to make laighside site still work)
        // other way around would be better for security (ie. even get rid of file system option all togeather?)

        // get list of files in the folder and check if one of them matches the request
        std::string page_filename = "";
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(doc_root.c_str())) != nullptr) {
          // loop through all the files and directories within directory
          while ((ent = readdir(dir)) != nullptr) {
              std::string d_name_str;
              if (ent->d_name)
                  d_name_str = std::string(ent->d_name);

              if (d_name_str.size() > 0 && d_name_str == JlweUtils::replaceString(JlweUtils::replaceString(page_request, "/", ""), "\\", "")) {
                  page_filename = doc_root + "/" + d_name_str;
              }
          }
          closedir (dir);
        } else {
          // could not open directory
        }

        // Problem: index.html is an empty file - the file needs to exist to get Apache to run this code.
        // But we want index.html to come from the MySQL database
        // Need to read the empty file then check for length of html_page before going to MySQL ?

        // if the file is in the folder, then read the data from it
        if (page_filename.size())
            html_page = JlweUtils::readFileToString(page_filename.c_str());
        if (html_page.size()) {
            size_t t_index = html_page.find("<!--NAME=");
            if (t_index != std::string::npos){
                title = html_page.substr(t_index + 9, html_page.find("-->", t_index) - t_index - 9);
            }
            if (html_page.find("<!--SHOW_NOTE-->") != std::string::npos)
                note = true;
            if (html_page.find("<!--LOGIN_ONLY-->") != std::string::npos)
                login_only = true;
        } else {
            // page is not on filesystem so look in mysql

            prep_stmt = jlwe.getMysqlCon()->prepareStatement("SELECT page_name, html, login_only FROM webpages WHERE path = ? AND special_page = 0;");
            prep_stmt->setString(1, page_request);
            res = prep_stmt->executeQuery();

            if (res->next()){
                title = res->getString(1);
                html_page = res->getString(2);
                login_only = res->getInt(3);
            }

            delete res;
            delete prep_stmt;
        }

        if (!html.outputHeader(&jlwe, title, note))
            return 0;

        if (login_only && jlwe.isLoggedIn() == false){
            std::cout << "<p>You need to be logged in to view this area.</p>";
        } else {
            std::cout << html_page;
        }

        html.outputFooter();

    } catch (const sql::SQLException &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << e.what() << " (MySQL error code: " << std::to_string(e.getErrorCode()) << ")\n";
    } catch (const std::exception &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << e.what() << "\n";
    }
    return 0;
}
