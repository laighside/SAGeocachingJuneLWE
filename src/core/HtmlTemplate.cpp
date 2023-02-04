/**
  @file    HtmlTemplate.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This class creates the HTML header and footer on every page of the website
  The template is read from a file

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "HtmlTemplate.h"

#include <iostream>  // cout
#include <regex>     // regex for detecting mobile browsers

#include "CgiEnvironment.h"
#include "Encoder.h"
#include "JlweUtils.h"


// html template file for making the header and footer of every page
#define TEMPLATE_PATH "/template.html"

// html template file for mobile browsers
#define TEMPLATE_MOBLIE "/mobile_template.html"

// Regex for detecting moblie browers. A match on either 1 OR 2 means it is a mobile browser.
#define MOBILE_REGEX_1  "(android|bb\\d+|meego).+mobile|avantgo|bada\\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|iris|kindle|lge\\ |maemo|midp|mmp|mobile.+firefox|netfront|opera\\ m(ob|in)i|palm(\\ os)?|phone|p(ixi|re)\\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\\.(browser|link)|vodafone|wap|windows\\ ce|xda|xiino"
#define MOBILE_REGEX_2  "^(1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a\\ wa|abac|ac(er|oo|s\\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\\-m|r\\ |s\\ )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\\-(n|u)|c55\\/|capi|ccwa|cdm\\-|cell|chtm|cldc|cmd\\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\\-s|devi|dica|dmob|do(c|p)o|ds(12|\\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\\-|_)|g1\\ u|g560|gene|gf\\-5|g\\-mo|go(\\.w|od)|gr(ad|un)|haie|hcit|hd\\-(m|p|t)|hei\\-|hi(pt|ta)|hp(\\ i|ip)|hs\\-c|ht(c(\\-|\\ |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\\-(20|go|ma)|i230|iac(\\ |\\-|\\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt(\\ |\\/)|klon|kpt\\ |kwc\\-|kyo(c|k)|le(no|xi)|lg(\\ g|\\/(k|l|u)|50|54|\\-[a-w])|libw|lynx|m1\\-w|m3ga|m50\\/|ma(te|ui|xo)|mc(01|21|ca)|m\\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\\-|\\ |o|v)|zz)|mt(50|p1|v\\ )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\\-2|po(ck|rt|se)|prox|psio|pt\\-g|qa\\-a|qc(07|12|21|32|60|\\-[2-7]|i\\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\\-|oo|p\\-)|sdk\\/|se(c(\\-|0|1)|47|mc|nd|ri)|sgh\\-|shar|sie(\\-|m)|sk\\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\\-|v\\-|v\\ )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\\-|tdg\\-|tel(i|m)|tim\\-|t\\-mo|to(pl|sh)|ts(70|m\\-|m3|m5)|tx\\-9|up(\\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\\-|\\ )|webc|whit|wi(g\\ |nc|nw)|wmlb|wonu|x700|yas\\-|your|zeto|zte\\-)"

struct menuItem {
    std::string text;
    std::string url;
};

HtmlTemplate::HtmlTemplate(bool allowMobile) {
    this->useMobile = (isMobileBrowser() && allowMobile);
    this->templatePath = CgiEnvironment::getDocumentRoot() + (this->useMobile ? TEMPLATE_MOBLIE : TEMPLATE_PATH);
}

HtmlTemplate::~HtmlTemplate() {
    // do nothing
}

std::string HtmlTemplate::makeMenuHTML(JlweCore *jlwe, bool mobile) {
    std::vector<menuItem> rootList;

    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;
    prep_stmt = jlwe->getMysqlCon()->prepareStatement("SELECT link_text, link_url FROM webpage_menu WHERE parent = ? ORDER BY menu_order;");
    prep_stmt->setString(1, "*root*");
    res = prep_stmt->executeQuery();
    while (res->next()){
        rootList.push_back({res->getString(1), res->getString(2)});
    }
    delete res;

    std::string result = mobile ? "<div id=\"page_menu\">\n" : "<ul>";
    for (unsigned int i = 0; i < rootList.size(); i++) {
        std::vector<menuItem> innerList;
        prep_stmt->setString(1, rootList.at(i).text);
        res = prep_stmt->executeQuery();
        while (res->next()){
            innerList.push_back({res->getString(1), res->getString(2)});
        }
        delete res;

        result += mobile ? "" : "<li>";
        if (innerList.size()) {
            std::string elementID = "sub_menu_" + std::to_string(i);
            if (mobile) {
                result += "<a href=\"" + checkBlankLink("") + "\" onclick=\"toggleSubMenu('" + elementID + "')\">" + Encoder::htmlEntityEncode(rootList.at(i).text) + " <span id=\"" + elementID + "_arrow\">&#9660;</span></a>\n";
                result += "<div id=\"" + elementID + "\">\n";
                if (rootList.at(i).url.size())
                    result += "<a href=\"" + checkBlankLink(rootList.at(i).url) + "\">&nbsp;&nbsp;&nbsp;" + Encoder::htmlEntityEncode(rootList.at(i).text) + "</a>\n";
            } else {
                result += "<a href=\"" + checkBlankLink(rootList.at(i).url) + "\">" + Encoder::htmlEntityEncode(rootList.at(i).text) + "</a>";
                result += "<ul>";
            }

            for (unsigned int j = 0; j < innerList.size(); j++) {
                if (mobile) {
                    result += "<a href=\"" + checkBlankLink(innerList.at(j).url) + "\">&nbsp;&nbsp;&nbsp;" + Encoder::htmlEntityEncode(innerList.at(j).text) + "</a>\n";
                } else {
                    result += "<li>";
                    result += "<a href=\"" + checkBlankLink(innerList.at(j).url) + "\">" + Encoder::htmlEntityEncode(innerList.at(j).text) + "</a>";
                    result += "</li>";
                }

            }
            result += mobile ? "</div>\n" : "</ul>";
        } else {
            result += "<a href=\"" + checkBlankLink(rootList.at(i).url) + "\">" + Encoder::htmlEntityEncode(rootList.at(i).text) + "</a>";
        }
        result += mobile ? "" : "</li>";

    }
    if (mobile) {
        if (jlwe->isLoggedIn()) {
            result += "<a href=\"/cgi-bin/admin_index.cgi\">Admin Area</a>\n";
        } else {
            result += "<a href=\"/login.html\">Admin Login</a>\n";
        }
    }
    result += mobile ? "</div>\n" : "</ul>";

    delete prep_stmt;
    return result;
}

std::string HtmlTemplate::getLoginHtml(const std::string &username) {
    if (username.size())
        return "Logged in as " + Encoder::htmlEntityEncode(username) + " - <a href=\"/change_password.html\">Change PW</a> - <a href=\"/cgi-bin/admin_index.cgi\">Admin area</a> - <a href=\"/cgi-bin/logout.cgi\">Logout</a>";
    return "<a href=\"/login.html\">Login</a>";
}

bool HtmlTemplate::isMobileBrowser() {
    std::string user_agent = CgiEnvironment::getUserAgent();
    std::regex regex1(MOBILE_REGEX_1, std::regex_constants::icase);
    std::regex regex2(MOBILE_REGEX_2, std::regex_constants::icase);
    if (std::regex_search(user_agent, regex1) || std::regex_search(user_agent, regex2)) {
        return true;
    }
    return false;
}

void HtmlTemplate::outputHttpHtmlHeader() {
    std::cout << "Content-type:text/html\r\n\r\n";
}

bool HtmlTemplate::outputHeader(JlweCore *jlwe, const std::string &title, bool note) {
    this->html = JlweUtils::readFileToString(this->templatePath.c_str());
    if (this->html.size() == 0) {
        std::cout << "<html>\nFile not found on server: " << this->templatePath << "\n</html>";
        return false;
    }

    this->html = JlweUtils::replaceString(this->html, "**TITLE**", Encoder::htmlEntityEncode(title));
    this->html = JlweUtils::replaceString(this->html, "**LOGIN**", this->getLoginHtml(jlwe->getCurrentUsername()));
    this->html = JlweUtils::replaceString(this->html, "**MENU**", this->makeMenuHTML(jlwe, this->useMobile));

    size_t content_index = this->html.find("<div id=\"content\">") + 18;
    size_t note_id_index = this->html.find("id=\"header-note\"");
    if (note || note_id_index == std::string::npos) {
        std::cout << this->html.substr(0, content_index) << "\n";
    } else {
        size_t note_index = this->html.rfind("<ul", note_id_index);
        size_t note_index_end = this->html.find("</ul>", note_id_index) + 5;
        std::cout << this->html.substr(0, note_index) << "\n";
        std::cout << this->html.substr(note_index_end, content_index - note_index_end) << "\n";
    }
    return true;
}

void HtmlTemplate::outputFooter() {
    size_t content_index = this->html.find("<div id=\"content\">") + 18;
    std::cout << this->html.substr(content_index);
}

void HtmlTemplate::outputPageWithMessage(JlweCore *jlwe, const std::string &message, const std::string &title) {
    HtmlTemplate::outputHttpHtmlHeader();
    HtmlTemplate instance;
    if (!instance.outputHeader(jlwe, title, false))
        return;

    std::cout << "<p>" << Encoder::htmlEntityEncode(message) << "</p>";

    instance.outputFooter();
}

void HtmlTemplate::outputAdminMenu() {
    std::cout << "<h1 style=\"text-align:center;\">JLWE Admin area</h1>\n";
    std::cout << "<div id=\"admin_menu\">\n<ul>\n";
    std::cout << "<li><a href=\"/cgi-bin/admin_index.cgi\">Admin Index</a></li>\n";
    std::cout << "<li><a href=\"/cgi-bin/files/files.cgi\">File Manager</a></li>\n";
    std::cout << "<li><a href=\"/cgi-bin/mailing_list/mailing_list.cgi\">Mailing List</a></li>\n";
    std::cout << "<li><a href=\"/cgi-bin/gpx_builder/gpx_builder.cgi\">GPX Builder</a></li>\n";
    std::cout << "<li><a href=\"/cgi-bin/scoring/scoring.cgi\">Scoring</a></li>\n";
    std::cout << "<li><a href=\"/cgi-bin/registration/registration.cgi\">Event Registrations</a></li>\n";
    std::cout << "<li><a href=\"/cgi-bin/merch/merch.cgi\">Merchandise Orders</a></li>\n";
    std::cout << "<li><a href=\"/cgi-bin/settings/settings.cgi\">Website Settings</a></li>\n";
    std::cout << "<li><a href=\"/cgi-bin/website_edit/website_edit.cgi\">Edit Website</a></li>\n";
    std::cout << "<li><a href=\"/cgi-bin/email_forward/email_forward.cgi\">Email Forwarding</a></li>\n";
    std::cout << "<li><a href=\"/cgi-bin/notes/notes.cgi\">Notes</a></li>\n";
    std::cout << "<li><a href=\"/cgi-bin/users/users.cgi\">Users</a></li>\n";
    std::cout << "</ul>\n</div>\n";
    std::cout << "<p style=\"color:red;text-align:center;\"><noscript>You need javascript enabled to use the admin tools on this site.</noscript></p>\n";
}

