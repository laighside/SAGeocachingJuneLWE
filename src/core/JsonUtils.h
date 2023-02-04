/**
  @file    JsonUtils.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions for making common JSON structures
  All functions are static so there is no need to create instances of the JsonUtils object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef JSONUTILS_H
#define JSONUTILS_H

#include <string>

class JsonUtils {
public:

    /*!
     * \brief Creates the Content-type header for JSON (application/json).
     *
     * \return The HTTP header
     */
    static std::string makeJsonHeader();

    /*!
     * \brief Creates a JSON error response with the error message provided
     *
     * \param error_message The error message
     * \return The JSON with HTTP header
     */
    static std::string makeJsonError(const std::string &error_message);

    /*!
     * \brief Creates a JSON succeess response with the message provided
     *
     * \param success_message The message to show the user on success
     * \return The JSON with HTTP header
     */
    static std::string makeJsonSuccess(const std::string &success_message);

};

#endif // JSONUTILS_H
