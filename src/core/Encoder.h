/**
  @file    Encoder.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions that are used to encode/decode strings and data
  Implements the encoding described at https://cheatsheetseries.owasp.org/cheatsheets/Cross_Site_Scripting_Prevention_Cheat_Sheet.html
  All functions are static so there is no need to create instances of the Encoder object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef ENCODER_H
#define ENCODER_H

#include <cstdint>
#include <string>

class Encoder {

public:

    /*!
     * \brief Encodes some data as base64.
     *
     * \param data The array of data to encode
     * \param inputLength The length of the data array
     * \return The base64 encoded data as a string
     */
    static std::string base64encode(const unsigned char *data, size_t inputLength);

    /*!
     * \brief Encodes some data as base64.
     *
     * \param data The string data to encode
     * \return The base64 encoded data as a string
     */
    static std::string base64encode(const std::string &data);

    /*!
     * \brief This removes all \\n and \\r characters from some text
     *
     * \param text The text to remove new lines from
     * \return The text without new lines
     */
    static std::string removeNewLines(const std::string &text);

    /*!
     * \brief This is for encoding data that goes between HTML tags
     *
     * eg. </p>Encode the data that goes here</p>
     *
     * \param text The text to encode
     * \return The HTML encoded text
     */
    static std::string htmlEntityEncode(const std::string &text);

    /*!
     * \brief This is for encoding data that goes in HTML attributes
     *
     * eg. <div name="Encode the data that goes here"></div>
     *
     * \param text The text to encode
     * \return The HTML attribute encoded text
     */
    static std::string htmlAttributeEncode(const std::string &text);

    /*!
     * \brief This is for encoding data that goes in JavaScript data values
     *
     * eg. <div onclick="myFunction('Encode the data that goes here')"></div>
     *
     * \param text The text to encode
     * \return The Javascript data encoded text
     */
    static std::string javascriptAttributeEncode(const std::string &text);

    /*!
     * \brief This is for encoding data that goes in URL values
     *
     * eg. <a href="http://www.example.com?key=Encode the data that goes here">
     *
     * \param text The text to encode
     * \return The URL encoded text
     */
    static std::string urlEncode(const std::string &text);

    /*!
     * \brief This is for decoding data from URL query values
     *
     * eg. key1=Decode+this&key2=Decode+this+to
     *
     * \param urlValue The value to decode
     * \return The decoded text
     */
    static std::string urlDecode(const std::string& urlValue);

    /*!
     * \brief This removes any characters that aren't in 0-9, A-Z, a-z, -, _, .
     *
     * Spaces are replaced with underscores
     * Fullstops are allowed but two sequential fullstops are not
     *
     * \param input The text to filter
     * \return The text with safe characters only
     */
    static std::string filterSafeCharsOnly(const std::string &input);

    /*!
     * \brief Encode some text with ROT13
     *
     * ROT13 is symmetric so encode and decode are the same function
     *
     * \param text The text to encode
     * \return The ROT13 encoded text
     */
    static std::string ROT13Encode(const std::string &text);

private:
    // tables used in base64 encoding
    static char base64_encoding_table[];
    static unsigned int base64_mod_table[];

    // functions for representing characters in hexadecimal format
    static std::string charToHex(char c);
    static char hexToChar(char first, char second);

    // UTF-8 decoding functions
    static int utf8Length(const char ch);
    static uint32_t utf8CodePoint(int code_point_length, const char *ch);
};

#endif // ENCODER_H
