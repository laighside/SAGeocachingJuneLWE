/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 * This is a modififed version of the file CgiEnvironment.cpp from cgicc
 * ---------------------------------------------------------------------
 *
 *  $Id: CgiEnvironment.cpp,v 1.31 2017/06/22 20:26:35 sebdiaz Exp $
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
 *
 */


#include "CgiEnvironment.h"

// ========== Constructor/Destructor

CgiEnvironment::CgiEnvironment() {
    // do nothing
}

CgiEnvironment::~CgiEnvironment() {
    // do nothing
}

std::string CgiEnvironment::getServerSoftware() {
    return getenvAsString("SERVER_SOFTWARE");
}

std::string CgiEnvironment::getServerName() {
    return getenvAsString("SERVER_NAME");
}

std::string CgiEnvironment::getGatewayInterface() {
    return getenvAsString("GATEWAY_INTERFACE");
}

std::string CgiEnvironment::getDocumentRoot() {
    return getenvAsString("DOCUMENT_ROOT");
}

std::string CgiEnvironment::getServerProtocol() {
    return getenvAsString("SERVER_PROTOCOL");
}

unsigned long CgiEnvironment::getServerPort() {
    std::string port = getenvAsString("SERVER_PORT");
    try {
        return std::stoul(port);
    }catch (...) {}
    return 0;
}

bool CgiEnvironment::usingHTTPS() {
    return (getenvAsString("HTTPS") == "on");
}

std::string CgiEnvironment::getCookies() {
    return getenvAsString("HTTP_COOKIE");
}

std::string CgiEnvironment::getRequestMethod() {
    return getenvAsString("REQUEST_METHOD");
}

std::string CgiEnvironment::getPathInfo() {
    return getenvAsString("PATH_INFO");
}

std::string CgiEnvironment::getPathTranslated() {
    return getenvAsString("PATH_TRANSLATED");
}

std::string CgiEnvironment::getScriptName() {
    return getenvAsString("SCRIPT_NAME");
}

std::string CgiEnvironment::getRequestUri() {
    return getenvAsString("REQUEST_URI");
}

std::string CgiEnvironment::getQueryString() {
    return getenvAsString("QUERY_STRING");
}

unsigned long CgiEnvironment::getContentLength() {
    std::string length = getenvAsString("CONTENT_LENGTH");
    try {
        return std::stoul(length);
    }catch (...) {}
    return 0;
}

std::string CgiEnvironment::getContentType() {
    return getenvAsString("CONTENT_TYPE");
}

std::string CgiEnvironment::getReferrer() {
    return getenvAsString("HTTP_REFERER");
}

std::string CgiEnvironment::getRemoteHost() {
    return getenvAsString("REMOTE_HOST");
}

std::string CgiEnvironment::getRemoteAddr() {
    return getenvAsString("REMOTE_ADDR");
}

std::string CgiEnvironment::getAuthType() {
    return getenvAsString("AUTH_TYPE");
}

std::string CgiEnvironment::getRemoteUser() {
    return getenvAsString("REMOTE_USER");
}

std::string CgiEnvironment::getRemoteIdent() {
    return getenvAsString("REMOTE_IDENT");
}

std::string CgiEnvironment::getAccept() {
    return getenvAsString("HTTP_ACCEPT");
}

std::string CgiEnvironment::getUserAgent() {
    return getenvAsString("HTTP_USER_AGENT");
}

std::string CgiEnvironment::getRedirectRequest() {
    return getenvAsString("REDIRECT_REQUEST");
}

std::string CgiEnvironment::getRedirectURL() {
    return getenvAsString("REDIRECT_URL");
}

std::string CgiEnvironment::getRedirectStatus() {
    return getenvAsString("REDIRECT_STATUS");
}

std::string CgiEnvironment::getenvAsString(const char *varName) {
  char *var = std::getenv(varName);
  return (nullptr == var) ? std::string("") : std::string(var);
}
