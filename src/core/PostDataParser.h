/**
  @file    PostDataParser.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A class that reads the data sent to the server in POST or PUT requests

  This can be any of the following:
   * A single un-encoded file (with file type defined in Content-Type)
   * The Url encoded data from a submitted form (Content-Type: application/x-www-form-urlencoded)
   * A multipart encoded form data eg. file upload (Content-Type: multipart/form-data)

  This class reuses a lot of code from the cgicc library: https://www.gnu.org/software/cgicc/index.html

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

#ifndef POSTDATAPARSER_H
#define POSTDATAPARSER_H

#include <string>
#include <vector>

#include "KeyValueParser.h"

class PostDataParser : public KeyValueParser {

public:
    struct FormFile {
        std::string name;
        std::string filename;
        std::string dataType;
        std::string data;
    };

    /**
      Constructer for PostDataParser class. The post data is parsed on construction.

      @param maxContentLength is maxium maximum content to accept. If a request larger than this is received, it will be ignored and a error will occur.
    */
    PostDataParser(long maxContentLength);

    /**
      Destructor for PostDataParser class.
    */
    ~PostDataParser();


    /*!
     * \brief Parses the URL encoded form data. Call this after the constructor when URL encoded data is expected.
     */
    void parseUrlEncodedForm();

    /*!
     * \brief Returns if an error occurred while parsing the post data.
     *
     * \return true if an error occurred, false otherwise
     */
    bool hasError();

    /*!
     * \brief Returns a description of the error that occurred while parsing the post data.
     *
     * \return A description of the error, or an empty string if no error occurred.
     */
    std::string errorText();

    /*!
     * \brief Returns the content type of the post data received.
     *
     * \return The content type. (the Content-Type header)
     */
    std::string contentType();

    /*!
     * \brief Returns a length of the post data received.
     *
     * \return The length of the recived data. (the Content-Length header)
     */
    long contentLength();

    /*!
     * \brief Returns a pointer to the post data received.
     *
     * \return A pointer to the data, call contentLength() to get the size of this array
     */
    char * data();

    /*!
     * \brief Returns the the post data received as a string.
     *
     * \return A string containing the post data
     */
    std::string dataAsString();

    /*!
     * \brief Gets the list of files that was in a multipart/form-data post.
     *
     * \return A pointer to a vector containing the list of files
     */
    std::vector<FormFile> * getFiles();


private:

    std::string mRequestMethod;
    std::string mContentType;
    long mContentLength;
    char *dataBuffer;
    std::vector<FormFile> mFiles;

    bool mParseError;
    std::string mErrorText;

    struct MultipartHeader {
        std::string disposition;
        std::string name;
        std::string filename;
        std::string contentType;
    };

    void parseMultiPart();
    void parseMIME(const std::string& data);
    PostDataParser::MultipartHeader parseHeader(const std::string& data);
};

#endif // POSTDATAPARSER_H
