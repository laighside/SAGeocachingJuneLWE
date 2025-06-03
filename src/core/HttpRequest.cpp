/**
  @file    HttpRequest.cpp
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
#include "HttpRequest.h"

#include <cstring> // memcpy

HttpRequest::HttpRequest(const std::string &url) {

    curl_global_init(CURL_GLOBAL_DEFAULT);
    this->curl = curl_easy_init();
    this->m_followRedirects = true;

    if (curl) {
        curl_easy_setopt(this->curl, CURLOPT_URL, url.c_str());
    } else {
        this->m_errorMessage = "Error initializing curl";
    }

    this->responseBuffer = (char*)malloc(1);
    this->responseBufferSize = 0;
    this->responseBufferValid = true;

    this->postData = nullptr;
    this->postDataSize = 0;

    this->m_responseCode = 0;
}

HttpRequest::~HttpRequest() {
    if (this->curl)
        curl_easy_cleanup(this->curl);
    if (this->responseBuffer)
        free(this->responseBuffer);
    if (this->postData)
        free(postData);

    curl_global_cleanup();
}

void HttpRequest::setUserAgent(const std::string &userAgent) {
    this->m_headers.push_back("User-Agent: " + userAgent);
}
void HttpRequest::setCookies(const std::string &cookies) {
    this->m_headers.push_back("Cookie: " + cookies);
}
void HttpRequest::setReferer(const std::string &referer) {
    this->m_headers.push_back("Referer: " + referer);
}
void HttpRequest::setHeader(const std::string &header) {
    this->m_headers.push_back(header);
}
void HttpRequest::setFollowRedirects(bool followRedirects) {
    this->m_followRedirects = followRedirects;
}

bool HttpRequest::get() {
    bool success = false;
    if (this->curl && this->responseBuffer) {

        if (this->m_followRedirects) {
            curl_easy_setopt(this->curl, CURLOPT_FOLLOWLOCATION, 1L);
        }

#ifdef SKIP_PEER_VERIFICATION
        /*
         * If you want to connect to a site who isn't using a certificate that is
         * signed by one of the certs in the CA bundle you have, you can skip the
         * verification of the server's certificate. This makes the connection
         * A LOT LESS SECURE.
         *
         * If you have a CA cert for the server stored someplace else than in the
         * default bundle, then the CURLOPT_CAPATH option might come handy for
         * you.
         */
        curl_easy_setopt(this->curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
        /*
         * If the site you're connecting to uses a different host name that what
         * they have mentioned in their server certificate's commonName (or
         * subjectAltName) fields, libcurl will refuse to connect. You can skip
         * this check, but this will make the connection less secure.
         */
        curl_easy_setopt(this->curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

        // set the callback function to handle data received
        curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, HttpRequest::callbackFunction);
        curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, this);

        // set the headers
        struct curl_slist *list = nullptr;
        for (unsigned int i = 0; i < this->m_headers.size(); i++) {
            struct curl_slist *temp_list = curl_slist_append(list, this->m_headers.at(i).c_str());
            if (temp_list)
                list = temp_list;
        }
        if (list)
            curl_easy_setopt(this->curl, CURLOPT_HTTPHEADER, list);

        // make the request
        CURLcode res = curl_easy_perform(this->curl);

        // handle the result
        if (res == CURLE_OK) {
            curl_easy_getinfo (this->curl, CURLINFO_RESPONSE_CODE, &(this->m_responseCode));
            success = true;

        } else {
            const char *error = curl_easy_strerror(res);
            if (error)
                this->m_errorMessage = std::string(error);
        }

        // free memory
        curl_slist_free_all(list);
    }
    return success;
}

bool HttpRequest::post(const std::string &data, const std::string &content_type, bool use_put, bool use_patch) {
    return this->post(data.c_str(), data.size(), content_type, use_put, use_patch);
}

bool HttpRequest::post(const char *data, size_t size, const std::string &content_type, bool use_put, bool use_patch) {
    if (!this->curl)
        return false;

    if (this->postData)
        free(postData);

    this->postData = (char*)malloc(size + 1);
    this->postDataSize = size;
    if (!this->postData) {
        this->m_errorMessage = "Unable to allocate memory for post data";
        return false;
    }

    memcpy(this->postData, data, size);
    this->postData[this->postDataSize] = 0;

    this->m_headers.push_back("Content-Type: " + content_type);
    this->m_headers.push_back("Content-Length: " + std::to_string(size));

    // this makes sure POST is still used after any redirects
    curl_easy_setopt(this->curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);

    curl_easy_setopt(this->curl, CURLOPT_POSTFIELDS, this->postData);
    curl_easy_setopt(this->curl, CURLOPT_POSTFIELDSIZE, size);

    if (use_put)
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    if (use_patch)
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");

    return this->get();
}

std::string HttpRequest::responseAsString() {
    if (this->responseBuffer)
        return std::string(this->responseBuffer, this->responseBufferSize);
    return "";
}

long HttpRequest::responseCode() {
    return this->m_responseCode;
}

std::string HttpRequest::errorMessage() {
    return this->m_errorMessage;
}

size_t HttpRequest::callbackFunction(void *buffer, size_t size, size_t n, void *f) {
    // Call non-static member function.
    return static_cast<HttpRequest*>(f)->dataReceived(buffer, size, n);
}

size_t HttpRequest::dataReceived(void *buffer, size_t size, size_t n) {
    size_t realsize = size * n;

    char *tempPtr = (char*)realloc(this->responseBuffer, this->responseBufferSize + realsize + 1);
    if (!tempPtr) {
        // out of memory!
        this->responseBufferValid = false;
        return 0;
    }

    this->responseBuffer = tempPtr;
    memcpy(&(this->responseBuffer[this->responseBufferSize]), buffer, realsize);
    this->responseBufferSize += realsize;
    this->responseBuffer[this->responseBufferSize] = 0;

    return realsize;
}


