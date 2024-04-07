/**
  @file    FormElements.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions that create the HTML to make form elements (textboxes, radiobuttons, etc.)
  No encoding is done on values so all user input must be encoded before calling these functions
  All functions are static so there is no need to create instances of the FormElements object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef FORMELEMENTS_H
#define FORMELEMENTS_H

#include <ctime>   // time_t
#include <string>
#include <vector>

class FormElements {

public:

    /*!
     * \brief Object that carries the data needed to create a radiobutton
     */
    struct radiobutton {
        std::string id;
        std::string label;
        std::string value;
        std::string onInput;
        bool checked;
        bool disabled;
        std::string sub_comment;
    };

    /*!
     * \brief Object that carries the data needed to create an option in a \<select\> element (drop down combo box)
     */
    struct dropDownOption {
        std::string value;
        std::string text;
        bool selected;
    };

    /*!
     * \brief Object that carries the data needed to create a tab for a page with multiple tabs
     */
    struct pageTab {
        std::string id;
        std::string name;
    };

    /*!
     * \brief Object that carries the data needed to create a menu item in a drop down menu
     */
    struct dropDownMenuItem {
        std::string javascript;
        std::string text;
        bool enabled;
    };

    /*!
     * \brief Creates the HTML \<script\> element to include a Javascript file
     *
     * \param url The URL to the Javascript
     * \return The HTML to go in the webpage
     */
    static std::string includeJavascript(const std::string &url);

    /*!
     * \brief Creates the HTML for the Next/Previous buttons at the bottem of multi-page forms
     *
     * \return The HTML to go in the webpage
     */
    static std::string formButtons();

    /*!
     * \brief Creates the HTML to display input errors on forms before allowing the user to go to the next page
     *
     * \return The HTML to go in the webpage
     */
    static std::string formMessages();

    /*!
     * \brief Creates the HTML to make the tabs at the top of a page with multiple tabs
     *
     * \param tabs A vector of tab names/ids to put on the page
     * \return The HTML to go in the webpage
     */
    static std::string pageTabs(const std::vector<pageTab> &tabs);

    /*!
     * \brief Creates the HTML to make a \<input\> element
     *
     * \param id The value to go in the id attribute
     * \param type The input type attribute, "text" for a plain, single line text input
     * \param question The question to ask the user above the input box
     * \param placeholder The placeholder value to go in the input box
     * \return The HTML to go in the webpage
     */
    static std::string textInput(const std::string &id, const std::string &type, const std::string &question, const std::string &placeholder);

    /*!
     * \brief Creates the HTML to make a \<textarea\> element
     *
     * \param id The value to go in the id attribute
     * \param question The question to ask the user above the text box
     * \return The HTML to go in the webpage
     */
    static std::string textArea(const std::string &id, const std::string &question);

    /*!
     * \brief Creates the HTML to make a \<input type=\"number\"\> element
     *
     * \param id The value to go in the id attribute
     * \param question The question to ask the user above the input box
     * \param value The default value in the number input
     * \param min The minimum allowed value for the number input
     * \param max The maximum allowed value for the number input
     * \return The HTML to go in the webpage
     */
    static std::string numberInput(const std::string &id, const std::string &question, int value, int min, int max);

    /*!
     * \brief Creates the HTML to make a checkbox
     *
     * \param id The value to go in the id attribute
     * \param label The text to display next to the checkbox
     * \param checked The default state of the checkbox
     * \return The HTML to go in the webpage
     */
    static std::string checkbox(const std::string &id, const std::string &label, bool checked);

    /*!
     * \brief Creates the HTML to make a slider switch
     *
     * \param id The value to go in the id attribute
     * \param checked The default state of the switch
     * \return The HTML to go in the webpage
     */
    static std::string htmlSwitch(const std::string &id, bool checked);

    /*!
     * \brief Creates the HTML to make a set of radiobuttons
     *
     * \param name The value to go in the name attribute
     * \param question The question to ask the user above the radiobuttons
     * \param options A vector containing a list of radiobutton to put in the list of options
     * \return The HTML to go in the webpage
     */
    static std::string radioButtons(const std::string &name, const std::string &question, const std::vector<radiobutton> &options);

    /*!
     * \brief Creates the HTML to make a single radiobutton
     *
     * \param id The value to go in the id attribute
     * \param name The value to go in the name attribute
     * \param label The text to display alongside the radiobutton
     * \param value The value to go in the value attribute (if the form gets submitted this ends up as name=value in the URL encoded data)
     * \param oninput The javascript to go in the oninput attribute
     * \param checked The default state of the radiobutton
     * \param disabled If set to true, the radiobutton will be disabled
     * \param sub_comment Text displayed in a smaller font below the main label text
     * \return The HTML to go in the webpage
     */
    static std::string radioButton(const std::string &id, const std::string &name, const std::string &label, const std::string &value, const std::string &oninput, bool checked, bool disabled, const std::string &sub_comment = "");

    /*!
     * \brief Creates the HTML to make a \<select\> element (a drop down combo box)
     *
     * \param name The value to go in the name and id attributes
     * \param question The question to ask the user above the combo box
     * \param options A vector containing a list of options the user can choose from in the combo box
     * \return The HTML to go in the webpage
     */
    static std::string dropDownList(const std::string &name, const std::string &question, const std::vector<dropDownOption> &options);

    /*!
     * \brief Creates the HTML to make the 3 dots drop down menu that is usually used at the end of table rows
     *
     * \param id A number that is unique to all other drop down menus on the page, used to make the id attribute
     * \param items A vector containing a list of menu items
     * \return The HTML to go in the webpage
     */
    static std::string dropDownMenu(int id, const std::vector<dropDownMenuItem> &items);

    /*!
     * \brief Creates the HTML to make three input boxes for email, username and phone number
     *
     * This is used on all registration pages and the merch order page
     *
     * \param withGPXcheckbox If set to true, a checkbox asking if the user would like to signup to the GPX mailing list is included in the output
     * \return The HTML to go in the webpage
     */
    static std::string emailUsernamePhoneBoxes(bool withGPXcheckbox);

    /*!
     * \brief Convert a time_t value to a human readable date format
     *
     * eg. Monday June 12
     *
     * \param time The date/time to convert
     * \return The date as a string
     */
    static std::string timeToDateString(time_t time);

};

#endif // FORMELEMENTS_H
