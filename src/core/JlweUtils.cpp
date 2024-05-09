/**
  @file    JlweUtils.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions that are used in various places across the website
  All functions are static so there is no need to create instances of the JlweUtils object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

#include "JlweUtils.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <stdexcept>

#include <cstdio>   // for popen
#include <iostream> // cout

#include "../ext/csprng/csprng.hpp"

#ifdef HAVE_MAXMINDDB
#include <maxminddb.h>
#endif

char JlweUtils::random_char_list[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                      'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                      'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                      'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                      'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                      'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                      'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                      '4', '5', '6', '7', '8', '9'};

std::string JlweUtils::makeRandomToken(size_t length) {
    duthomhas::csprng rng;

    std::string result = "";
    for (unsigned int i = 0; i < length; i++) {
        unsigned int ch;
        ch = rng(ch) % 62;
        result.push_back(random_char_list[ch]);
    }
    return result;
}

int JlweUtils::getCurrentYear() {
    time_t curTime = time(nullptr);
    struct tm * timeS;
    timeS = localtime(&curTime);
    return 1900 + timeS->tm_year;
}

std::string JlweUtils::getCurrentYearString() {
    return std::to_string(getCurrentYear());
}


std::string JlweUtils::timeToW3CDTF(const time_t unixtime) {
    struct tm * timeS;
    timeS = gmtime(&unixtime);
    char timeBuffer[100];
    strftime(timeBuffer, 100, "%Y-%m-%dT%H:%M:%SZ", timeS);
    return std::string(timeBuffer);
}

std::string JlweUtils::toString3Decimal(double number){
    char buffer [50];
    sprintf (buffer, "%2.3f", number);
    return std::string(buffer);
}

int JlweUtils::roundDown(double number) {
    if (number < 0) {
        return static_cast<int>(ceil(number));
    }
    return static_cast<int>(floor(number));
}

std::string JlweUtils::makeCoordString(double lat, double lon, bool useHtml){
    std::string result = "";
    std::string space = (useHtml ? "&nbsp;" : " ");
    if (lat >= 0) {
        result = result + "N";
    } else {
        result = result + "S";
    }
    int intPart = roundDown(lat);
    double minutes = std::abs(lat - intPart) * 60;
    result = result + std::to_string(abs(intPart)) + "°" + space + toString3Decimal(minutes) + space;

    if (lon < 0) {
        result = result + "W";
    } else {
        result = result + "E";
    }
    intPart = roundDown(lon);
    minutes = std::abs(lon - intPart) * 60;
    result = result + std::to_string(abs(intPart)) + "°" + space + toString3Decimal(minutes);

    return result;
}

std::string JlweUtils::makeFullCacheName(int number, const std::string &name, bool isCreative, bool isPermanent) {
    std::string result = std::to_string(number);
    if (result.length() == 1)
        result = "0" + result;
    if (isCreative || isPermanent) {
        result += "(";
        if (isCreative)
            result += "C";
        if (isCreative && isPermanent)
            result += ",";
        if (isPermanent)
            result += "P";
        result += ")";
    }
    result += ": " + name;
    return result;
}

std::string JlweUtils::numberToOrdinal(int number) {
  if (number < 0)
      return std::to_string(number);

  std::string suffix = "th";
  if (number % 100 < 11 || number % 100 > 13) {
    switch (number % 10) {
      case 1:
        suffix = "st";
        break;
      case 2:
        suffix = "nd";
        break;
      case 3:
        suffix = "rd";
        break;
    }
  }
  return std::to_string(number) + suffix;
}

std::string JlweUtils::readFileToString(const char *filename) {
    FILE * file = fopen(filename, "r");
    if (!file)
        throw std::runtime_error("Unable to open file: " + std::string(filename));
    std::string result = "";
    char buffer[1024];
    size_t size = 1024;
    while (size == 1024){
        size = fread(buffer, 1, 1024, file);
        if (size)
            result.append(buffer, size);
    }
    fclose(file);
    return result;
}

void JlweUtils::readFileToOStream(FILE * file, std::ostream& stream, size_t buffer_size) {
    if (!file) return;
    char buffer[buffer_size];
    size_t size = buffer_size;
    while (size == buffer_size) {
        size = fread(buffer, 1, buffer_size, file);
        stream.write(buffer, static_cast<long>(size));
    }
}

bool JlweUtils::compareStringsNoCase(const std::string& s1, const std::string& s2) {
    std::string::const_iterator p1 = s1.begin();
    std::string::const_iterator p2 = s2.begin();
    std::string::const_iterator l1 = s1.end();
    std::string::const_iterator l2 = s2.end();

    while(p1 != l1 && p2 != l2) {
        if(std::toupper(*(p1++)) != std::toupper(*(p2++)))
            return false;
    }

    return (s2.size() == s1.size()) ? true : false;
}

bool JlweUtils::compareTeamNames(const std::string& s1, const std::string& s2) {
    std::string s1_simple;
    for (unsigned int i = 0; i < s1.length(); i++) {
        char ch = s1.at(i);
        if (ch <= ' ') continue;
        if (ch == '*') continue;
        if (ch == '_') continue;

        // everything else is unchanged
        s1_simple.push_back(ch);
    }
    std::string s2_simple;
    for (unsigned int i = 0; i < s2.length(); i++) {
        char ch = s2.at(i);
        if (ch <= ' ') continue;
        if (ch == '*') continue;
        if (ch == '_') continue;

        // everything else is unchanged
        s2_simple.push_back(ch);
    }

    return compareStringsNoCase(s1_simple, s2_simple);
}

// locate data between separators, and return it
std::string JlweUtils::extractBetween(const std::string& data, const std::string& separator1, const std::string& separator2)
{
  std::string result;
  std::string::size_type start, limit;

  start = data.find(separator1, 0);
  if(std::string::npos != start) {
    start += separator1.length();
    limit = data.find(separator2, start);
    if(std::string::npos != limit)
      result = data.substr(start, limit - start);
  }

  return result;
}


std::vector<std::string> JlweUtils::splitString(std::string subject, char delimiter) {
    std::vector<std::string> result;
    size_t index = subject.find(delimiter);
    while (index != std::string::npos) {
        result.push_back(subject.substr(0, index));
        subject = subject.substr(index + 1);
        index = subject.find(delimiter);
    }
    result.push_back(subject);
    return result;
}

void JlweUtils::trimString(std::string &str) {
    JlweUtils::trimStringLeft(str);
    JlweUtils::trimStringRight(str);
}

void JlweUtils::trimStringLeft(std::string &str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

void JlweUtils::trimStringRight(std::string &str) {
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), str.end());
}

std::string JlweUtils::getMIMEType(const std::string &filename){
    try {

        // This returns the wrong type for GPX files
        /*std::string output = JlweUtils::runCommand("file -i " + filename);
        size_t index = output.find(": ") + 2;
        if (index == std::string::npos)
            return "";
        size_t index2 = output.find("; ");
        if (index2 == std::string::npos)
            return "";
        return output.substr(index, index2 - index);*/

        std::string output = JlweUtils::runCommand("mimetype " + filename);
        // remove new lines
        output.erase(std::remove(output.begin(), output.end(), '\n'), output.cend());
        output.erase(std::remove(output.begin(), output.end(), '\r'), output.cend());

        size_t index = output.find(": ") + 2;
        if (index == std::string::npos)
            return "";
        return output.substr(index);

    } catch (...) {
        return "";
    }
}

std::string JlweUtils::runCommand(const std::string &cmd) {
    char buffer[1024];
    std::string result = "";
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed");
    while (fgets(buffer, 1024, pipe) != nullptr) {
        result.append(buffer);
    }
    int exit_code = pclose(pipe);
    if (exit_code)
        throw std::runtime_error("Commmand exited with code " + std::to_string(exit_code));
    return result;
}

std::string JlweUtils::getGeoIPCountry(const std::string &ipAddress, const std::string& mmdb_filename) {
    std::string ip_country = "";
#ifdef HAVE_MAXMINDDB
    // load GeoIP database
    MMDB_s mmdb;
    int mmdb_status = MMDB_open(mmdb_filename.c_str(), MMDB_MODE_MMAP, &mmdb);

    if (mmdb_status == MMDB_SUCCESS) {
        int gai_error, mmdb_error;
        MMDB_lookup_result_s result = MMDB_lookup_string(&mmdb, ipAddress.c_str(), &gai_error, &mmdb_error);
        if (gai_error == 0 && mmdb_error == MMDB_SUCCESS) {
            MMDB_entry_data_s entry_data;
            int status = MMDB_get_value(&result.entry, &entry_data, "country", "names", "en", NULL);
            if (status == MMDB_SUCCESS && entry_data.has_data && entry_data.type == MMDB_DATA_TYPE_UTF8_STRING) {
                ip_country = std::string(entry_data.utf8_string, entry_data.data_size);
            }
        }
        MMDB_close(&mmdb);
    }
#endif
    return ip_country;
}



