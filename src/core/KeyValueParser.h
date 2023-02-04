/**
  @file    keyValueParser.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A class that parses strings of key-value pairs, such as URL query parameters or cookies
  Example: key1=value1&key2=value2&key3=value3
  or: key1=value1;key2=value2;key3=value3
  Delimiters supported are & and ;
  The keys are not necessarily unique
  URL decoding of keys and values is also provided
  This class reuses a lot of code from the cgicc library: https://www.gnu.org/software/cgicc/index.html

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef KEYVALUEPARSER_H
#define KEYVALUEPARSER_H

#include <string>
#include <vector>

class KeyValueParser {
public:

    /**
      Constructer for keyValueParser class.

      @param keyValueList is the string containing the key-value data to be parsed.
      @param decodeUrls is set to true if URL decoding is required, use when parsing URL query parameters.
    */
    KeyValueParser(const std::string &keyValueList, bool decodeUrls = false);

    KeyValueParser();

    /**
      Destructor for keyValueParser class.
    */
    ~KeyValueParser();

    struct KeyValuePair {
        std::string key;
        std::string value;
    };

    void parseFromString(const std::string &keyValueList, bool decodeUrls);
    std::string getValue(const std::string &key, std::string defaultValue = "");
    void clear();
    bool isEmpty();

protected:
    void addValue(const std::string &key, const std::string &value);

private:
    static std::string removeWhiteSpaces(const std::string& src);

    std::vector<KeyValuePair> keyValues;
};

#endif // KEYVALUEPARSER_H
