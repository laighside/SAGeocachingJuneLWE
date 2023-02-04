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
#include "JsonUtils.h"

#include "../ext/nlohmann/json.hpp"

std::string JsonUtils::makeJsonHeader() {
    return "Content-type:application/json\r\n\r\n";
}

std::string JsonUtils::makeJsonSuccess(const std::string &success_message) {
    nlohmann::json jsonDocument;

    jsonDocument["success"] = true;
    jsonDocument["message"] = success_message;

    return makeJsonHeader() + jsonDocument.dump();
}

std::string JsonUtils::makeJsonError(const std::string &error_message) {
    nlohmann::json jsonDocument;

    jsonDocument["success"] = false;
    jsonDocument["error"] = error_message;

    return makeJsonHeader() + jsonDocument.dump();
}
