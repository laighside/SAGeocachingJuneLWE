/**
  @file    JlweUtils.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions that are used in various places across the website
  All functions are static so there is no need to create instances of the JlweUtils object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

#ifndef JLWEUTILS_H
#define JLWEUTILS_H

#include <string>
#include <vector>

class JlweUtils {

public:

    /*!
     * \brief Returns the current year according to the server clock.
     *
     * \return The current year
     */
    static int getCurrentYear();

    /*!
     * \brief Returns the current year according to the server clock.
     *
     * \return The current year as a string
     */
    static std::string getCurrentYearString();

    /*!
     * \brief Converts time_t value to a string in W3CDTF format.
     *
     * This is commonly used to store date/time values in XML files
     *
     * \param unixtime The time_t value to convert
     * \return The date/time in W3CDTF format
     */
    static std::string timeToW3CDTF(const time_t unixtime);

    /*!
     * \brief Creates a string of random characters.
     *
     * The string will contain only alphanumeric characters.
     *
     * \param length The desired length of the string
     * \return A string of random numbers
     */
    static std::string makeRandomToken(size_t length);

    /*!
     * \brief Converts a lat/lon coordinate to human readable decimal minutes format.
     *
     * Decimal minutes format is N/S hh° mm.mmm E/W hhh° mm.mmm
     * \param lat The latitude
     * \param lon The longitude
     * \param useHtml Set to true to output the spaces as HTML &nbsp;
     * \return The coordinates in decimal minutes format
     */
    static std::string makeCoordString(double lat, double lon, bool useHtml = false);

    /*!
     * \brief Formats a cache name for display.
     *
     * eg. "53(C): Trojan Rabbit"
     * (C) for creative caches
     * (P) for permanent caches
     *
     * \param number The cache number
     * \param name The cache name in the database
     * \param isCreative Set to true if it is a creative cache
     * \param isPermanent Set to true if it is a permanent cache
     * \return The full cache name
     */
    static std::string makeFullCacheName(int number, const std::string &name, bool isCreative, bool isPermanent);

    /*!
     * \brief Converts a number to string with correct suffix, eg. 1st, 2nd, 3rd, etc.
     *
     * Negative numbers have no suffix.
     *
     * \param number The number to convert to string
     * \return The number as a string plus the ordinal suffix
     */
    static std::string numberToOrdinal(int number);

    /*!
     * \brief Reads an entire file and returns the contents as a string.
     *
     * Will throw an runtime exception if the file doesn't exist or can't be opened.
     *
     * \param filename The full name of the file to read
     * \return The file's contents as a string
     */
    static std::string readFileToString(const char *filename);

    /*!
     * \brief Reads an entire file and writes it to an ostream (eg. cout).
     *
     * \param file Open file pointer to read from
     * \param stream The ostream to write to
     * \param buffer_size How many bytes in each block of data read then written
     */
    static void readFileToOStream(FILE * file, std::ostream& stream, size_t buffer_size = 1024);

    /*!
     * \brief A case insensitive comparison of two strings.
     *
     * \param s1 The first string
     * \param s2 The second string
     * \return true if the strings match, false otherwise
     */
    static bool compareStringsNoCase(const std::string& s1, const std::string& s2);

    /*!
     * \brief A case insensitive comparison of two strings after removing spaces, underscores and asterisks.
     *
     * Used for comparing and matching team names
     *
     * \param s1 The first string
     * \param s2 The second string
     * \return true if the strings match, false otherwise
     */
    static bool compareTeamNames(const std::string& s1, const std::string& s2);

    /*!
     * \brief Extract a substring contained within two separators.
     *
     * For example, after the call
     * \code
     * std::string data = "11foo22";
     * std::string res;
     * res = extractBetween(data, "11", "22");
     * \endcode
     * \c res will be "foo".
     * \param data The data to search.
     * \param separator1 The first logical separator.
     * \param separator2 The second logical separator.
     * \return The substring between the separators.
     */
    static std::string extractBetween(const std::string& data, const std::string& separator1, const std::string& separator2);

    /*!
     * \brief Extract a substring contained between a separator.
     *
     * This function is used internally to decode \c multipart/form-data
     *
     * \param data The data to search.
     * \param separator The separator.
     * \return The substring between the separator.
     */
    static inline std::string extractBetween(const std::string& datas, const std::string& separators)
    { return extractBetween(datas, separators, separators); }


    static inline std::string replaceString(std::string subject, const std::string& search,
                              const std::string& replace) {
        if (search.size() == 0) // This will cause an infinite loop so just return if the search string is empty
            return subject;

        size_t pos = 0;
        while ((pos = subject.find(search, pos)) != std::string::npos) {
             subject.replace(pos, search.length(), replace);
             pos += replace.length();
        }
        return subject;
    }

    /*!
     * \brief Splits a string at the given delimiters.
     *
     * eg. splitting this "abc-def-ghi" on delimiter '-' gives a vector of 3 strings "abc", "def", "ghi"
     *
     * \param subject The string to split
     * \param delimiter The character to split on
     * \return A vector of each sub string
     */
    static std::vector<std::string> splitString(std::string subject, char delimiter);

    /*!
     * \brief Removes the whitespace/new line characters from the start and end of a string
     *
     * \param str The string to trim
     */
    static void trimString(std::string &str);

    /*!
     * \brief Removes the whitespace/new line characters from the start of a string
     *
     * \param str The string to trim
     */
    static void trimStringLeft(std::string &str);

    /*!
     * \brief Removes the whitespace/new line characters from the end of a string
     *
     * \param str The string to trim
     */
    static void trimStringRight(std::string &str);

    /*!
     * \brief Gets the MIME type of a file.
     *
     * Uses the 'file' command.
     *
     * \param filename The full name of the file to get the MIME type of
     * \return The MIME type or an empty string if the MIME type can not be determined
     */
    static std::string getMIMEType(const std::string &filename);

    /*!
     * \brief Runs a command and returns whatever was written to the standard output.
     *
     * Throws an error if the command doesn't exit with code 0.
     *
     * \param cmd The command to run
     * \return The standard output from the command
     */
    static std::string runCommand(const std::string &cmd);

    /*!
     * \brief Gets the country of an IP address using the MaxMind GeoIP database.
     *
     * Only available if HAVE_MAXMINDDB is defined at compile time, will always return a empty string if it isn't.
     *
     * \param ipAddress The IP address to lookup in the database
     * \param mmdb_filename The full filename of the MaxMind GeoIP database file
     * \return The country or an empty string if the country can not be determined
     */
    static std::string getGeoIPCountry(const std::string &ipAddress, const std::string &mmdb_filename);

private:
    // List of characters to use in the random strings
    static char random_char_list[];

    /*!
     * \brief Rounds a number to an integer by rounding towards zero.
     *
     * eg. 4.7 goes to 4
     *     -3.8 goes to 3
     * Used in converting coordinates to hh° mm.mmm format.
     *
     * \param val The number to round
     * \return The rounded number
     */
    static int roundDown(double number);

    /*!
     * \brief Convert a double to a string with 3 decimal places.
     *
     * Used in converting coordinates to hh° mm.mmm format.
     *
     * \param number The number as a double
     * \return The number as a string
     */
    static std::string toString3Decimal(double number);
};

#endif // JLWEUTILS_H
