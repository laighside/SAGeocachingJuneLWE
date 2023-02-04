/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *
 * This is a modififed version of the file CgiEnvironment.h from cgicc
 * -------------------------------------------------------------------
 *
 *  $Id: CgiEnvironment.h,v 1.21 2014/04/23 20:55:03 sebdiaz Exp $
 *
 *  Copyright (C) 1996 - 2004 Stephen F. Booth <sbooth@gnu.org>
 *                       2007 Sebastien DIAZ <sebastien.diaz@gmail.com>
 *  Part of the GNU cgicc library, http://www.gnu.org/software/cgicc
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 */

#ifndef CGIENVIRONMENT_H
#define CGIENVIRONMENT_H

#ifdef __GNUG__
#  pragma interface
#endif

/*! \file CgiEnvironment.h
 * \brief Class encapsulating the CGI runtime environment
 *
 * The \c CgiEnvironment class encapsulates the  environment of 
 * the CGI application as described by the HTTP server.  \c CgiEnvironment
 * contains the \c GET or \c POST data along with all environment variables 
 * set by the HTTP server specified in the CGI specification.
 */

#include <string>

  // ============================================================
  // Class CgiEnvironment
  // ============================================================
  
  /*! \class CgiEnvironment CgiEnvironment.h cgicc/CgiEnvironment.h 
   * \brief Class encapsulating the CGI runtime environment
   *
   * The \c CgiEnvironment class encapsulates the  environment of 
   * the CGI application as described by the HTTP server.  \c CgiEnvironment
   * contains the \c GET or \c POST data along with all environment variables 
   * set by the HTTP server specified in the CGI specification.
   */
class CgiEnvironment {
  public:

    CgiEnvironment();

    /*!
     * \brief Destructor 
     *
     * Delete this CgiEnvironment object
     */
    ~CgiEnvironment();

    // ============================================================

    /*! \name Server Information
     * Information on the server handling the HTTP/CGI request
     */
    //@{
    
    /*!
     * \brief Get the name and version of the HTTP server software
     *
     * For example, \c Apache/1.3.4
     * \return The name of the server software
     */
    static std::string getServerSoftware();
    
    /*!
     * \brief Get the hostname, DNS name or IP address of the HTTP server
     *
     * This is \e not a URL, for example \c www.gnu.org (no leading http://)
     * \return The name of the server
     */
    static std::string getServerName();
    
    /*!
     * \brief Get the name and version of the gateway interface.
     *
     * This is usually \c CGI/1.1
     * \return The name and version of the gateway interface
     */
    static std::string getGatewayInterface();

    /*!
     * \brief Get the path of the document root on the server.
     *
     * This is usually \c /var/www/html
     * \return The document root path
     */
    static std::string getDocumentRoot();
    
    /*!
     * \brief Get the name and revision of the protocol used for this request.
     *
     * This is usually \c HTTP/1.0 or \c HTTP/1.1
     * \return The protocol in use
     */
    static std::string
    getServerProtocol();
    
    /*!
     * \brief Get the port number on the server to which this request was sent.
     *
     * This will usually be 80.
     * \return The port number
     */
    static unsigned long
    getServerPort();
    
    /*!
     * \brief Determine if this is a secure request
     * 
     * A secure request is usually made using SSL via HTTPS
     * \return \c true if this connection is via https, \c false otherwise
     */
    static bool
    usingHTTPS();
    //@}
    
    // ============================================================
    
    /*! \name CGI Query Information
     * Information specific to this CGI query
     */
    //@{
    
    /*!
     * \brief Get the HTTP cookies associated with this query, if any.  
     *
     * Get the list of HTTP cookies.
     * \return The HTTP cookies
     */
    static std::string getCookies();
    
    /*!
     * \brief Get the request method used for this query.
     *
     * This is usually one of \c GET or \c POST
     * \return The request method
     */
    static std::string getRequestMethod();
    
    /*!
     * \brief Get the extra path information for this request, given by the 
     * client.
     *
     * For example, in the string \c foo.cgi/cgicc the path information is
     * \c cgicc.
     * \return The absolute path info
     */
    static std::string getPathInfo();
    
    /*!
     * \brief Get the translated path information (virtual to physical mapping).
     *
     * For example, \c www.gnu.org may be translated to \c /htdocs/index.html
     * \return The translated path info
     */
    static std::string getPathTranslated();
    
    /*!
     * \brief Get the full path to this CGI application
     *
     * This is useful for self-referencing URIs
     * \return The full path of this application
     */
    static std::string getScriptName();

    /*!
     * \brief Get the request path
     *
     * The path of the HTTP request, such as \c /foo/bar.html?key=value
     * \return The request path
     */
    static std::string getRequestUri();
    
    /*!
     * \brief Get the query string for this request.
     *
     * The query string follows the <tt>?</tt> in the URI which called this
     * application. This is usually only valid for scripts called with 
     * the \c GET method. For example, in the string \c foo.cgi?cgicc=yes 
     * the query string is \c cgicc=yes.
     * @return The query string
     */
    static std::string getQueryString();
    
    /*!
     * \brief Get the length of the data read from standard input, in chars.
     *
     * This is usually only valid for scripts called with the POST method.
     * \return The data length
     */
    static unsigned long getContentLength();
    
    /*!
     * \brief Get the content type of the submitted information.
     *
     * For applications called via the GET method, this information is
     * irrelevant.  For applications called with the POST method, this is
     * specifies the MIME type of the information, 
     * usually \c application/x-www-form-urlencoded or as specified by
     * getContentType().
     * \return The content type
     * \see getContentType
     */
    static std::string getContentType();

    
    // ============================================================
    
    /*! \name Server Specific Information
     * Information dependent on the type of HTTP server in use
     */
    //@{
    
    /*!
     * \brief Get the URL of the page which called this CGI application.
     *
     * Depending on the HTTP server software, this value may not be set.
     * \return The URI which called this application.
     */
    static std::string getReferrer();
    //@}
    
    // ============================================================
    
    /*! \name Remote User Information
     * Information about the user making the CGI request
     */
    //@{
    
    /*!
     * \brief Get the hostname of the remote machine making this request
     *
     * This may be either an IP address or a hostname
     * \return The remote host
     */
    static std::string getRemoteHost();
    
    /*!
     * \brief Get the IP address of the remote machine making this request
     *
     * This is a standard IP address of the form \c 123.123.123.123
     * \return The remote IP address
     */
    static std::string getRemoteAddr();
    
    /*!
     * \brief Get the protocol-specific user authentication method used.
     *
     * This is only applicable if the server supports user authentication,
     * and the user has authenticated.
     * \return The authorization type
     */
    static std::string getAuthType();
    
    /*!
     * \brief Get the authenticated remote user name.
     *
     * This is only applicable if the server supports user authentication,
     * and the user has authenticated.
     * \return The remote username 
     */
    static std::string getRemoteUser();
    
    /*!
     * \brief Get the remote user name retrieved from the server.
     *
     * This is only applicable if the server supports RFC 931 
     * identification.  This variable should \e only be used
     * for logging purposes.
     * \return The remote identification
     * \see RFC 1431 at 
     * http://info.internet.isi.edu:80/in-notes/rfc/files/rfc1413.txt
     */
    static std::string getRemoteIdent();
    
    /*!
     * \brief Get the MIME data types accepted by the client's browser.
     *
     * For example <TT>image/gif, image/x-xbitmap, image/jpeg, image/pjpeg</TT>
     * \return The accepted data types
     */
    static std::string getAccept();
    
    /*!
     * \brief Get the name of the browser used for this CGI request.
     *
     * For example <TT>Mozilla/5.0 (X11; U; Linux 2.4.0 i686; en-US; 0.8.1) 
     * Gecko/20010421</TT>
     * \return The browser name
     */
    static std::string getUserAgent();
    //@}
    
    // ============================================================
    
    /*! \name ErrorDocument Handling
     * For a tutorial on ErrorDocument handling, see 
     * http://hoohoo.ncsa.uiuc.edu/cgi/ErrorCGI.html
     */
    //@{
    
    /*!
     * \brief Get the redirect request.
     *
     * This will only be valid if you are using this script as a script
     * to use in place of the default server messages.
     * @return The redirect request.
     */
    static std::string getRedirectRequest();
    
    /*!
     * \brief Get the redirect URL.
     *
     * This will only be valid if you are using this script as a script
     * to use in place of the default server messages.
     * \return The redirect URL.
     * \see http://hoohoo.ncsa.uiuc.edu/docs/setup/srm/ErrorDocument.html
     */
    static std::string getRedirectURL();
    
    /*!
     * \brief Get the redirect status.
     *
     * This will only be valid if you are using this script as a script
     * to use in place of the default server messages.
     * \return The redirect status.
     */
    static std::string getRedirectStatus();
    //@}

    /*!
     * \brief Get an Enviroment Variable.
     *
     * This will get the enviroment variable given by varName, convert it to a std::string and return it.
     * \param varName The name of the Enviroment Variable
     * \return The Enviroment Variable as a string.
     */
    static std::string getenvAsString(const char *varName);

};

#endif /* ! CGIENVIRONMENT_H */
