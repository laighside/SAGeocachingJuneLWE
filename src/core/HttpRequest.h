/**
  @file    HttpRequest.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A class that allows the server to make HTTP Requests to 3rd party websites.
  This is essentially a C++ wrapper around the curl library.

  These are the steps to make a HTTP request:
   - Create the HttpRequest object
   - Set any headers and options as required
   - Call get() or post() to make the request (this will block while waiting for the request to happen)
   - Handle the response data

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <string>
#include <vector>

#include <curl/curl.h>

class HttpRequest {

public:
    /*!
     * \brief Constructor.
     *
     * \param url The URL to make the request to
     */
    HttpRequest(const std::string &url);
    /*!
     * \brief Destructor.
     */
    ~HttpRequest();

    /*!
     * \brief Sets the User-Agent header.
     *
     * \param userAgent The string to set the User-Agent header to
     */
    void setUserAgent(const std::string &userAgent);

    /*!
     * \brief Sets the Cookies header.
     *
     * \param cookies The string to set the Cookies header to
     */
    void setCookies(const std::string &cookies);

    /*!
     * \brief Sets the Referer header.
     *
     * \param referer The string to set the referer header to
     */
    void setReferer(const std::string &referer);

    /*!
     * \brief Sets a generic header.
     *
     * \param header The full header line (name and value) to set, eg. "User-Agent: curl"
     */
    void setHeader(const std::string &header);

    /*!
     * \brief Sets whether to follow redirects or not.
     *
     * \param followRedirects Set to true to follow redirects, false otherwise
     */
    void setFollowRedirects(bool followRedirects);

    /*!
     * \brief Sends a GET request and blocks until the response is received (or timeout occurs).
     *
     * \return True if the request was successful, false otherwise
     */
    bool get();

    /*!
     * \brief Sends a POST request and blocks until the response is received (or timeout occurs).
     *
     * \param data The data to send in the post request
     * \param content_type The content type of the data (this ends up in the Content-Type header)
     * \return True if the request was successful, false otherwise
     */
    bool post(const std::string &data, const std::string &content_type);

    /*!
     * \brief Sends a POST request and blocks until the response is received (or timeout occurs).
     *
     * \param data The data to send in the post request
     * \param size The size of the data
     * \param content_type The content type of the data (this ends up in the Content-Type header)
     * \return True if the request was successful, false otherwise
     */
    bool post(const char *data, size_t size, const std::string &content_type);

    /*!
     * \brief Gets the response data.
     *
     * \return The response data as a string
     */
    std::string responseAsString();

    /*!
     * \brief Gets the response code (eg. 200 OK).
     *
     * \return The response code
     */
    long responseCode();

    /*!
     * \brief Gets the error message if an error has occurred.
     *
     * \return The error message, or an empty string if no error has occurred
     */
    std::string errorMessage();

private:
    // curl object
    CURL *curl;

    bool m_followRedirects;
    std::vector<std::string> m_headers;

    size_t responseBufferSize;
    bool responseBufferValid;
    char *responseBuffer;

    char *postData;
    size_t postDataSize;

    std::string m_errorMessage;
    long m_responseCode;

    static size_t callbackFunction(void *buffer, size_t size, size_t n, void *f);
    size_t dataReceived(void *buffer, size_t size, size_t n);

};

#endif // HTTPREQUEST_H
