/**
  @file    Encoder.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions that are used to encode/decode strings and data
  Implements the encoding described at https://cheatsheetseries.owasp.org/cheatsheets/Cross_Site_Scripting_Prevention_Cheat_Sheet.html
  All functions are static so there is no need to create instances of the Encoder object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "Encoder.h"

#include <cstdint>

// tables used in base64 encoding
char Encoder::base64_encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                         'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                         'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                         'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                         'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                         'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                         'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                         '4', '5', '6', '7', '8', '9', '+', '/'};
unsigned int Encoder::base64_mod_table[] = {0, 2, 1};

// Base 64 encoder
std::string Encoder::base64encode(const unsigned char *data, size_t inputLength) {
    std::string encodedData = "";

    if (inputLength == 0)
        return encodedData;

    for (unsigned int i = 0; i < inputLength;) {
        uint32_t octet_a = i < inputLength ? data[i++] : 0;
        uint32_t octet_b = i < inputLength ? data[i++] : 0;
        uint32_t octet_c = i < inputLength ? data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encodedData += base64_encoding_table[(triple >> 3 * 6) & 0x3F];
        encodedData += base64_encoding_table[(triple >> 2 * 6) & 0x3F];
        encodedData += base64_encoding_table[(triple >> 1 * 6) & 0x3F];
        encodedData += base64_encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (unsigned int i = 0; i < base64_mod_table[inputLength % 3]; i++)
        encodedData[encodedData.size() - 1 - i] = '=';

    return encodedData;
}

std::string Encoder::base64encode(const std::string &data) {
    return base64encode(reinterpret_cast<const unsigned char*>(data.c_str()), data.size());
}

// remove \n and \r from strings
std::string Encoder::removeNewLines(const std::string &text) {
    std::string result;
    result.reserve(text.size());

    for (unsigned int i = 0; i < text.length(); i++) {
        char ch = text.at(i);
        if (ch == '\n' || ch == '\r') {
            continue;
        } else {
            result.push_back(ch);
        }
    }

    return result;
}

// https://cheatsheetseries.owasp.org/cheatsheets/Cross_Site_Scripting_Prevention_Cheat_Sheet.html#rule-1-html-encode-before-inserting-untrusted-data-into-html-element-content
// Encode &, <, >, ", ', /
std::string Encoder::htmlEntityEncode(const std::string &text) {

    std::string result;
    result.reserve(text.size());

    for (unsigned int i = 0; i < text.length(); i++) {
        char ch = text.at(i);
        int utf8length = utf8Length(ch);
        if (utf8length == 1) { // basic ascii characters
            if (ch == '\n' || ch == '\r') { // new lines are allowed
                result.push_back(ch);
                continue;
            }
            if (ch >= 0 && ch < ' ') continue; // skip all other non-printable characters

            switch (ch) {
            case '&':  result.append("&amp;"); break;
            case '<':  result.append("&lt;");  break;
            case '>':  result.append("&gt;");  break;
            case '\'': result.append("&#39;"); break;
            case '\"': result.append("&#34;"); break;
            case '/':  result.append("&#47;"); break;
            default:
                result.push_back(ch); // No encoding needed for other ASCII characters
            }
            continue;
        }
        if (utf8length < 1) continue; // skip invalid bytes

        if (i + static_cast<unsigned int>(utf8length) > text.length()) break; // check there are enough bytes

        uint32_t code_point = utf8CodePoint(utf8length, text.c_str() + i);

        if (code_point) {
            std::string charOut = text.substr(i, static_cast<size_t>(utf8length));
            i = i + static_cast<size_t>(utf8length) - 1; // Skip over the extra bytes
            result.append(charOut); // Leave all non-ASCII characters intact if the encoding is valid
        }
    }

    return result;
}

// https://cheatsheetseries.owasp.org/cheatsheets/Cross_Site_Scripting_Prevention_Cheat_Sheet.html#rule-2-attribute-encode-before-inserting-untrusted-data-into-html-common-attributes
// Basically the same as HTML Entity Encode but there are some special cases where this can't be used - see OWASP website
// New lines within attributes don't make much sense either so remove them
std::string Encoder::htmlAttributeEncode(const std::string &text) {
    return removeNewLines(htmlEntityEncode(text));
}

// https://cheatsheetseries.owasp.org/cheatsheets/Cross_Site_Scripting_Prevention_Cheat_Sheet.html#rule-3-javascript-encode-before-inserting-untrusted-data-into-javascript-data-values
// Encoding for placing strings in javascript attribute values
// eg. <div onclick="myFunction('Encode the data that goes here')"></div>
// Port of this Java code:
// https://github.com/OWASP/owasp-java-encoder/blob/main/core/src/main/java/org/owasp/encoder/JavaScriptEncoder.java
std::string Encoder::javascriptAttributeEncode(const std::string &text) {

    std::string result;
    result.reserve(text.size());

    for (unsigned int i = 0; i < text.length(); i++) {
        char ch = text.at(i);
        int utf8length = utf8Length(ch);
        if (utf8length == 1) { // basic ascii characters
            // Encode the following characters
            if ((ch >= 0 && ch < 32) || ch == '&' || ch == '\'' || ch == '\"' || ch == '\\') {
                switch (ch) {
                case '\b': result.append("\\b"); break;
                case '\t': result.append("\\t"); break;
                case '\n': result.append("\\n"); break;
                case '\f': result.append("\\f"); break;
                case '\r': result.append("\\r"); break;
                default:
                    // Hex encoding
                    result.append("\\x");
                    result.append(charToHex(ch));
                    break;
                }
            } else {
                result.push_back(ch); // No encoding needed for other ASCII characters
            }
            continue;
        }
        if (utf8length < 1) continue; // skip invalid bytes

        if (i + static_cast<unsigned int>(utf8length) > text.length()) break; // check there are enough bytes

        uint32_t code_point = utf8CodePoint(utf8length, text.c_str() + i);

        if (code_point) {
            std::string charOut = text.substr(i, static_cast<size_t>(utf8length));
            i = i + static_cast<size_t>(utf8length) - 1;

            if (code_point == 0x2028 || code_point == 0x2029) { // LINE_SEPARATOR or PARAGRAPH_SEPARATOR
                result.append("\\u");
                result.append(charToHex(static_cast<char>(code_point >> 8 & 0xff)));
                result.append(charToHex(static_cast<char>(code_point & 0xff)));

            } else {
                result.append(charOut); // Leave all other non-ASCII characters intact
            }
        }

    }

    return result;
}

// From cgicc
std::string Encoder::urlEncode(const std::string &text) {
    std::string result;
    std::string::const_iterator iter;

    for(iter = text.begin(); iter != text.end(); ++iter) {
      switch(*iter) {
      case ' ':
        result.append(1, '+');
        break;
        // alnum
      case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
      case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
      case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
      case 'V': case 'W': case 'X': case 'Y': case 'Z':
      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
      case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
      case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
      case 'v': case 'w': case 'x': case 'y': case 'z':
      case '0': case '1': case '2': case '3': case '4': case '5': case '6':
      case '7': case '8': case '9':
        // mark
      case '-': case '_': case '.': case '!': case '~': case '*': case '\'':
      case '(': case ')':
        result.append(1, *iter);
        break;
        // escape
      default:
        result.append(1, '%');
        result.append(charToHex(*iter));
        break;
      }
    }

    return result;
}

// From cgicc
std::string Encoder::urlDecode(const std::string& urlValue) {
  std::string result;
  std::string::const_iterator iter;
  char c;

  for(iter = urlValue.begin(); iter != urlValue.end(); ++iter) {
    switch(*iter) {
    case '+':
      result.append(1, ' ');
      break;
    case '%':
      // Don't assume well-formed input
      if(std::distance(iter, urlValue.end()) >= 2
         && std::isxdigit(*(iter + 1)) && std::isxdigit(*(iter + 2))) {
        c = *++iter;
        result.append(1, hexToChar(c, *++iter));
      }
      // Just pass the % through untouched
      else {
        result.append(1, '%');
      }
      break;

    default:
      result.append(1, *iter);
      break;
    }
  }

  return result;
}

// remove all characters except 0-9, A-Z, a-z, -, _, .
std::string Encoder::filterSafeCharsOnly(const std::string &input) {
    std::string result;
    result.reserve(input.size());

    for (unsigned int i = 0; i < input.length(); i++) {
        char ch = input.at(i);
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) {
            result.push_back(ch);
            continue;
        }
        if (ch == '_' || ch == '-') {
            result.push_back(ch);
            continue;
        }
        // dots are allowed but not two sequentual dots
        if (ch == '.') {
            if (i == 0) {
                result.push_back(ch);
            } else {
                if (input.at(i - 1) != '.')
                    result.push_back(ch);
            }
            continue;
        }
        if (ch == ' ') {
            result.push_back('_');
            continue;
        }
    }

    // just a single dot is not allowed
    if (result == ".") return "";

    return result;
}

std::string Encoder::ROT13Encode(const std::string &text) {
    std::string result;
    result.reserve(text.size());

    for (unsigned int i = 0; i < text.length(); i++) {
        char ch = text.at(i);
        if ((ch >= 'A' && ch <= 'M') || (ch >= 'a' && ch <= 'm')) {
            result.push_back(ch + 13);
            continue;
        }
        if ((ch >= 'N' && ch <= 'Z') || (ch >= 'n' && ch <= 'z')) {
            result.push_back(ch - 13);
            continue;
        }
        // everything else is unchanged
        result.push_back(ch);
    }

    return result;
}

std::string Encoder::charToHex(char c) {
  std::string result;
  char first, second;

  first = (c & 0xF0) / 16;
  first += first > 9 ? 'A' - 10 : '0';
  second = c & 0x0F;
  second += second > 9 ? 'A' - 10 : '0';

  result.append(1, first);
  result.append(1, second);

  return result;
}

char Encoder::hexToChar(char first, char second) {
  int digit;

  digit = (first >= 'A' ? ((first & 0xDF) - 'A') + 10 : (first - '0'));
  digit *= 16;
  digit += (second >= 'A' ? ((second & 0xDF) - 'A') + 10 : (second - '0'));
  return static_cast<char>(digit);
}


int Encoder::utf8Length(const char ch) {
    if ((ch & 0b10000000) == 0b00000000)
        return 1;
    if ((ch & 0b11100000) == 0b11000000)
        return 2;
    if ((ch & 0b11110000) == 0b11100000)
        return 3;
    if ((ch & 0b11111000) == 0b11110000)
        return 4;

    return -1; // invalid UTF-8 byte
}

uint32_t Encoder::utf8CodePoint(int code_point_length, const char *ch) {
    if (code_point_length == 2) {
        return static_cast<uint32_t>(ch[0] & 0b00011111) << 6 | static_cast<uint32_t>(ch[1] & 0b00111111);
    }
    if (code_point_length == 3) {
        return static_cast<uint32_t>(ch[0] & 0b00001111) << 12 | static_cast<uint32_t>(ch[1] & 0b00111111) << 6 | static_cast<uint32_t>(ch[2] & 0b00111111);
    }
    if (code_point_length == 4) {
        return static_cast<uint32_t>(ch[0] & 0b00000111) << 18 | static_cast<uint32_t>(ch[1] & 0b00111111) << 12 | static_cast<uint32_t>(ch[2] & 0b00111111) << 6 | static_cast<uint32_t>(ch[3] & 0b00111111);
    }
    return 0;
}

