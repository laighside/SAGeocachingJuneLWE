/**
  @file    HtmlTemplate.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This class creates the HTML header and footer on every page of the website
  The template is read from a file

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef HTMLTEMPLATE_H
#define HTMLTEMPLATE_H

#include <string>

#include "JlweCore.h"

class HtmlTemplate {

public:

    /*!
     * \brief Constructor for HtmlTemplate.
     *
     * \param allowMobile If set to true the user-agent will be used to determine if the mobile website is used, if set to false the desktop website will be used
     */
    HtmlTemplate(bool allowMobile = true);

    /*!
     * \brief Destructor for HtmlTemplate.
     */
    ~HtmlTemplate();

    /*!
     * \brief Writes the page header to cout.
     *
     * \param jlwe JlweCore object
     * \param title The title of the page
     * \param note Set to true to include the "header-note" div on the page
     * \return true if successful, false if errors occurred
     */
    bool outputHeader(JlweCore *jlwe, const std::string &title, bool note);

    /*!
     * \brief Writes the page footer to cout.
     */
    void outputFooter();

    /*!
     * \brief Makes the output required to set the Content-type header to text/html.
     */
    static void outputHttpHtmlHeader();

    /*!
     * \brief Writes a complete HTML page with the given message on it.
     *
     * For making pages with just a single paragraph of text such as error pages.
     *
     * \param jlwe JlweCore object
     * \param message The plain text message to display on the page
     * \param title The title of the page
     */
    static void outputPageWithMessage(JlweCore *jlwe, const std::string &message, const std::string &title);

    /*!
     * \brief Writes the HTML to create the admin menu to cout.
     */
    static void outputAdminMenu();

private:

    static inline std::string checkBlankLink(const std::string &url) {
        if (url.size())
            return url;
        return "javascript:void(0);";
    }

    std::string makeMenuHTML(JlweCore *jlwe, bool mobile);
    static bool isMobileBrowser();
    static std::string getLoginHtml(const std::string &username);

    bool useMobile;
    std::string templatePath;
    std::string html;

};

#endif // HTMLTEMPLATE_H
