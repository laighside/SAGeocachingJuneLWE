/**
  @file    PostDataParser.cpp
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
#include "PostDataParser.h"

#include <iostream>  //cout, cin
#include <limits>

#include "CgiEnvironment.h"
#include "JlweUtils.h"
#include "Encoder.h"

PostDataParser::PostDataParser(long maxContentLength) :
    KeyValueParser()
{
    this->dataBuffer = nullptr;
    this->mParseError = false;
    this->mErrorText = "";
    this->mContentLength = CgiEnvironment::getContentLength();
    this->mRequestMethod = CgiEnvironment::getRequestMethod();
    this->mContentType = CgiEnvironment::getContentType();

    std::string multipart_type = "multipart/form-data";

    try {
        if (JlweUtils::compareStringsNoCase(this->mRequestMethod, "post") == false && JlweUtils::compareStringsNoCase(this->mRequestMethod, "put") == false)
            throw std::invalid_argument("Not a POST or PUT request");

        if (this->mContentLength <= 0)
            throw std::invalid_argument("Content-Length header is zero");

        if (this->mContentLength > maxContentLength)
            throw std::invalid_argument("Content-Length is too large, limit is " + std::to_string(maxContentLength) + " bytes");

        this->dataBuffer = new char[static_cast<unsigned int>(this->mContentLength)];
        if (!this->dataBuffer)
            throw std::invalid_argument("Unable to allocate " + std::to_string(this->mContentLength) + " bytes of memory for post data");

        std::cin.read(this->dataBuffer, this->mContentLength);
        long readLength = std::cin.gcount();
        if (readLength != mContentLength)
            throw std::invalid_argument("Only able to read " + std::to_string(readLength) + " of " + std::to_string(this->mContentLength) + " bytes");

        // if it is a multipart post (ie. a file upload) then parse it
        if (JlweUtils::compareStringsNoCase(this->mContentType.substr(0, multipart_type.size()), multipart_type))
            this->parseMultiPart();

    } catch (std::exception& e) {
        this->mParseError = true;
        this->mErrorText = e.what();
    }

    // clear and ignore anything left in the cin buffer (usually only happens if there is a error)
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max());
}

PostDataParser::~PostDataParser() {
    if (this->dataBuffer)
        delete [] this->dataBuffer;
}

void PostDataParser::parseUrlEncodedForm() {
    this->parseFromString(std::string(this->dataBuffer, this->mContentLength), true);
}

bool PostDataParser::hasError() {
    return this->mParseError;
}

std::string PostDataParser::errorText() {
    return this->mErrorText;
}

char * PostDataParser::data() {
    return this->dataBuffer;
}

std::string PostDataParser::dataAsString() {
    if (this->mParseError == false && this->mContentLength > 0 && this->dataBuffer)
        return std::string(this->dataBuffer, static_cast<size_t>(this->mContentLength));
    return "";
}

long PostDataParser::contentLength() {
    return this->mContentLength;
}

std::string PostDataParser::contentType() {
    return this->mContentType;
}

std::vector<PostDataParser::FormFile> * PostDataParser::getFiles() {
    return &(this->mFiles);
}

void PostDataParser::parseMultiPart() {
    std::string data(this->dataBuffer, this->mContentLength);

    // Find out what the separator is
    std::string 		bType 	= "boundary=";
    std::string::size_type 	pos 	= this->mContentType.find(bType);

    // Remove next sentence
    std::string                 commatek=";";

    // generate the separators
    std::string sep1 = this->mContentType.substr(pos + bType.length());
    if (sep1.find(";")!=std::string::npos)
       sep1=sep1.substr(0,sep1.find(";"));
    sep1.append("\r\n");
    sep1.insert(0, "--");

    std::string sep2 = this->mContentType.substr(pos + bType.length());
    if (sep2.find(";")!=std::string::npos)
       sep2=sep2.substr(0,sep2.find(";"));
    sep2.append("--\r\n");
    sep2.insert(0, "--");

    // Find the data between the separators
    std::string::size_type start  = data.find(sep1);
    std::string::size_type sepLen = sep1.length();
    std::string::size_type oldPos = start + sepLen;

    while(true) {
      pos = data.find(sep1, oldPos);

      // If sep1 wasn't found, the rest of the data is an item
      if(std::string::npos == pos)
        break;

      // parse the data
      parseMIME(data.substr(oldPos, pos - oldPos));

      // update position
      oldPos = pos + sepLen;
    }

    // The data is terminated by sep2
    pos = data.find(sep2, oldPos);
    // parse the data, if found
    if(std::string::npos != pos) {
      parseMIME(data.substr(oldPos, pos - oldPos));
    }
}

void PostDataParser::parseMIME(const std::string& data) {
  // Find the header
  std::string end = "\r\n\r\n";
  std::string::size_type headLimit = data.find(end, 0);

  // Detect error
  if(std::string::npos == headLimit)
    throw std::runtime_error("Malformed input");

  // Extract the value - there is still a trailing CR/LF to be subtracted off
  std::string::size_type valueStart = headLimit + end.length();
  std::string value = data.substr(valueStart, data.length() - valueStart - 2);

  // Parse the header - pass trailing CR/LF x 2 to parseHeader
  MultipartHeader head = parseHeader(data.substr(0, valueStart));

  if (head.filename.empty()) {
    this->addValue(head.name, value);
  } else {
    this->mFiles.push_back({head.name, head.filename, head.contentType, value});
  }
}

PostDataParser::MultipartHeader PostDataParser::parseHeader(const std::string& data)
{
  std::string disposition;
  disposition = JlweUtils::extractBetween(data, "Content-Disposition: ", ";");

  std::string name;
  name = JlweUtils::extractBetween(data, "name=\"", "\"");

  std::string filename;
  filename = JlweUtils::extractBetween(data, "filename=\"", "\"");

  std::string cType;
  cType = JlweUtils::extractBetween(data, "Content-Type: ", "\r\n\r\n");

  // This is hairy: Netscape and IE don't encode the filenames
  // The RFC says they should be encoded, so I will assume they are.
  filename = Encoder::urlDecode(filename);

  return {disposition, name, filename, cType};
}




