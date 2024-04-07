/**
  @file    FormElements.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions that create the HTML to make form elements (textboxes, radiobuttons, etc.)
  No encoding is done on values so all user input must be encoded before calling these functions
  All functions are static so there is no need to create instances of the FormElements object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "FormElements.h"

std::string FormElements::includeJavascript(const std::string &url) {
    return "<script type=\"text/javascript\" src=\"" + url + "\"></script>\n";
}

std::string FormElements::formButtons() {
    std::string result = "<div class=\"progress_buttons\"><div>\n";
    result += "<div><div><input type=\"button\" id=\"prevBtn\" onclick=\"nextPrev(-1)\" value=\"Previous\" />\n";
    result += "</div></div><div><div>\n";
    result += "<input type=\"button\" id=\"nextBtn\" onclick=\"nextPrev(1)\" value=\"Next\" /></div></div>\n";
    result += "</div></div>\n";
    return result;
}

std::string FormElements::formMessages() {
    return "<div><ul id=\"formMessages\" class=\"formMessages\"></ul></div>\n";
}

std::string FormElements::pageTabs(const std::vector<pageTab> &tabs) {
    std::string result = "<div class=\"pageTabs\">\n";

    for (unsigned int i = 0; i < tabs.size(); i++) {
        result += "<button id=\"page_tab_button_" + tabs.at(i).id + "\" class=\"pageTabLinks" + (i == 0 ? " defaultPageTab" : "") + "\" onclick=\"openPageTab(event, '" + tabs.at(i).id + "')\"" /*+ (i == 0 ? " id=\"defaultTabOpen\"" : "")*/ + ">" + tabs.at(i).name + "</button>\n";
    }

    result += "</div>\n";
    return result;
}

std::string FormElements::textInput(const std::string &id, const std::string &type, const std::string &question, const std::string &placeholder) {
    return "<p>" + question + "</p><div class=\"margin\"><input type=\"" + type + "\" id=\"" + id + "\" name=\"" + id + "\" placeholder=\"" + placeholder + "\" oninput=\"this.className = ''\" /></div>\n";
}

std::string FormElements::textArea(const std::string &id, const std::string &question) {
    return "<p>" + question + "</p><div class=\"margin\"><textarea id=\"" + id + "\" name=\"" + id + "\" rows=\"3\" cols=\"35\" oninput=\"this.className = ''\"></textarea></div>\n";
}

std::string FormElements::numberInput(const std::string &id, const std::string &question, int value, int min, int max) {
    return "<p>" + question + "</p><div class=\"margin\"><input type=\"number\" id=\"" + id + "\" name=\"" + id + "\" min=\"" + std::to_string(min) + "\" max=\"" + std::to_string(max) + "\" value=\"" + std::to_string(value) + "\" /></div>\n";
}

std::string FormElements::checkbox(const std::string &id, const std::string &label, bool checked) {
    std::string result = "";
    result += "<span class=\"checkbox_container\"><label>" + label + "\n";
    result += "<input type=\"checkbox\" id=\"" + id + "\" name=\"" + id + "\" value=\"true\" " + (checked ? "checked " : "") + "/>\n";
    result += "<span class=\"checkmark\"></span>\n";
    result += "</label></span>\n";
    return result;
}

std::string FormElements::htmlSwitch(const std::string &id, bool checked) {
    std::string result = "<label class=\"switch\">\n";
    result += "<input id=\"" + id + "\" name=\"" + id + "\" type=\"checkbox\" " + (checked ? "checked " : "") + " />\n";
    result += "<span></span></label>\n";
    return result;
}

std::string FormElements::radioButtons(const std::string &name, const std::string &question, const std::vector<radiobutton> &options) {
    std::string result = "";
    result += "<p style=\"line-height:2em\">" + question + "<br />\n";

    for (unsigned int i = 0; i < options.size(); i++) {
        radiobutton button = options.at(i);
        result += radioButton(button.id, name, button.label, button.value, button.onInput, button.checked, button.disabled, button.sub_comment);
        if (i + 1 >= options.size()) {
            result += "</p>\n";
        } else {
            result += "<br />\n";
        }
    }
    return result;
}

std::string FormElements::radioButton(const std::string &id, const std::string &name, const std::string &label, const std::string &value, const std::string &oninput, bool checked, bool disabled, const std::string &sub_comment) {
    std::string result = "<span class=\"checkbox_container\"><label id=\"" + id + "_label\">\n";
    result += "    <span id=\"" + id + "_label_text\">" + label + "</span>\n";
    if (sub_comment.size()) {
        result += "    <br/><span id=\"" + id + "_label_comment\" class=\"checkbox_container_comment\">" + sub_comment + "</span>\n";
    }
    result += "    <input type=\"radio\" name=\"" + name + "\" id=\"" + id + "\" value=\"" + value + "\"" + (oninput.size() > 0 ? " oninput=\"" + oninput + "\"" : "") + (checked ? " checked" : "") + (disabled ? " disabled" : "") + " />\n";
    result += "    <span class=\"radiobox\"></span>\n";
    result += "  </label></span>\n";
    return result;
}

std::string FormElements::dropDownList(const std::string &name, const std::string &question, const std::vector<dropDownOption> &options) {
    std::string result = "";
    result += "<p>" + question + "</p>\n";
    result += "<div class=\"margin\"><select id=\"" + name + "\" name=\"" + name + "\">\n";

    for (unsigned int i = 0; i < options.size(); i++) {
        dropDownOption option = options.at(i);
        result += "<option value=\"" + option.value + "\"" + (option.selected ? " selected" : "") + ">" + option.text + "</option>\n";
    }

    result += "</select></div>\n";
    return result;
}

std::string FormElements::dropDownMenu(int id, const std::vector<dropDownMenuItem> &items) {
    std::string result = "";
    result += "<div id=\"menu_row_" + std::to_string(id) + "\"><button id=\"more_button_row_" + std::to_string(id) + "\" class=\"more_button\" onclick=\"showMenu(" + std::to_string(id) + ")\"><span></span><span></span><span></span></button>\n";
    result += "<div class=\"more_menu\">\n";
    result += "<div class=\"more_menu_caret\"><div class=\"more_menu_caret_outer\"></div><div class=\"more_menu_caret_inner\"></div></div>\n";
    result += "<ul class=\"more_menu_items\">\n";

    for (unsigned int i = 0; i < items.size(); i++) {
        dropDownMenuItem item = items.at(i);
        result += "<li><button type=\"button\" class=\"more_menu_btn\" onclick=\"" + item.javascript + "\"" + (item.enabled ? "" : " disabled") + " />" + item.text + "</button></li>\n";
    }

    result += "</ul>\n";
    result += "</div></div>\n";
    return result;
}

std::string FormElements::emailUsernamePhoneBoxes(bool withGPXcheckbox) {
    std::string result = "";
    result += FormElements::textInput("email", "email", "Email:", "E-mail...");
    if (withGPXcheckbox)
        result += "&nbsp;&nbsp;" + FormElements::checkbox("email_gpx", "I would like to sign up to the mailing list to have the game day GPX file sent to this email address", false);
    result += FormElements::textInput("gc_username", "text", "Geocaching username:", "Username...");
    result += FormElements::textInput("phone", "tel", "Phone number:", "Phone...");
    return result;
}

std::string FormElements::timeToDateString(time_t time) {
    char strBuffer[100];
    strftime(strBuffer, 100, "%A %B %d", localtime(&time));
    return std::string(strBuffer);
}
