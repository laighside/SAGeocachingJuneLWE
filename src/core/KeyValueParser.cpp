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
#include "KeyValueParser.h"

#include "Encoder.h"
#include "JlweUtils.h"

KeyValueParser::KeyValueParser(const std::string &keyValueList, bool decodeUrls) {
    if (keyValueList.size())
        this->parseFromString(keyValueList, decodeUrls);
}

KeyValueParser::KeyValueParser() {
    // do nothing
}

KeyValueParser::~KeyValueParser() {
    // do nothing
}

void KeyValueParser::parseFromString(const std::string &keyValueList, bool decodeUrls) {
    // Don't waste time on empty input
    if(true == keyValueList.empty())
      return;

    std::string name, value;
    std::string::size_type pos;
    std::string::size_type oldPos = 0;

    // Parse the data in one fell swoop for efficiency
    while(true) {
      // Find the '=' separating the name from its value, also have to check for '&' as its a common misplaced delimiter but is a delimiter none the less
      pos = keyValueList.find_first_of( "&=", oldPos);

      // If no '=', we're finished
      if(std::string::npos == pos)
        break;

      // Decode the name
        // pos == '&', that means whatever is in name is the only name/value
      if( keyValueList.at( pos ) == '&' )
          {
                const char * pszData = keyValueList.c_str() + oldPos;
                while( *pszData == '&' ) // eat up extraneous '&'
                {
                        ++pszData; ++oldPos;
                }
                if( oldPos >= pos )
                { // its all &'s
                        oldPos = ++pos;
                        continue;
                }
                // this becomes an name with an empty value
                name = removeWhiteSpaces(keyValueList.substr(oldPos, pos - oldPos));
                if (decodeUrls)
                    name = Encoder::urlDecode(name);
                this->addValue(name, "");
                oldPos = ++pos;
                continue;
          }
      name = removeWhiteSpaces(keyValueList.substr(oldPos, pos - oldPos));
      if (decodeUrls)
          name = Encoder::urlDecode(name);
      oldPos = ++pos;

      // Find the '&' or ';' separating subsequent name/value pairs
      pos = keyValueList.find_first_of(";&", oldPos);

      // Even if an '&' wasn't found the rest of the string is a value
      value = removeWhiteSpaces(keyValueList.substr(oldPos, pos - oldPos));
      if (decodeUrls)
          value = Encoder::urlDecode(value);

      // Store the pair
      this->addValue(name, value);

      if(std::string::npos == pos)
        break;

      // Update parse position
      oldPos = ++pos;
    }
}

void KeyValueParser::addValue(const std::string &key, const std::string &value) {
    this->keyValues.push_back({key, value});
}

std::string KeyValueParser::getValue(const std::string &key, std::string defaultValue) {
    //return std::find_if(this->keyValues.begin(), this->keyValues.end(),FE_nameCompare(name));

    for (unsigned int i = 0; i < this->keyValues.size(); i++){
        if (JlweUtils::compareStringsNoCase(this->keyValues.at(i).key, key)){
            return this->keyValues.at(i).value;
        }
    }
    return defaultValue;
}

void KeyValueParser::clear() {
    this->keyValues.clear();
}

bool KeyValueParser::isEmpty() {
    return (this->keyValues.size() == 0);
}

std::string KeyValueParser::removeWhiteSpaces(const std::string& src) {

    // skip leading whitespace - " \f\n\r\t\v"
    std::string::size_type wscount_start = 0;
    std::string::size_type wscount_end = 0;
    std::string::const_iterator data_iter;

    for (data_iter = src.begin(); data_iter != src.end(); ++data_iter,++wscount_start)
      if(0 == std::isspace(*data_iter))
        break;

    // if the string is all white spaces;
    if (data_iter == src.end())
        return "";;

    data_iter = src.end();
    while (data_iter != src.begin()) {
        --data_iter;
       if(0 == std::isspace(*data_iter))
         break;
       ++wscount_end;
    }

    return src.substr(wscount_start, src.length() - wscount_start - wscount_end);

}
