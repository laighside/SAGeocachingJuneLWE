/**
  @file    prices.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This file defines the prices of event registration, camping and dinner
  This should be changed so that the prices aren't hard coded. But the complicated definition of the camping prices makes this difficult.

  NOTE: THIS CAN NOT BE CHANGED AFTER REGISTRATIONS HAVE BEEN RECEIVED
  Doing so will cause incorrect prices to be shown on past registrations

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef PRICES_H
#define PRICES_H

#include <string>

// prices are all in cents (NOT dollars!)

// event attendance prices
#define PRICE_EVENT_ADULT 2000
#define PRICE_EVENT_CHILD 0000

// camping prices
// This function calculates the camping price
static inline int getCampingPrice(const std::string &type, int number_people, int number_nights) {
    if (type == "powered")
        return (number_people <= 2 ? 2500 : (number_people > 5 ? 4000 : 1500 + number_people * 500)) * number_nights;
    if (type == "unpowered")
        return (number_people <= 2 ? 2000 : (number_people > 5 ? 3500 : 1000 + number_people * 500)) * number_nights;
    return 0;
}

// This function creates the Javascript for calculating the camping price
static inline std::string getCampingPriceJS() {
    std::string result = "function getCampingPrice(type, number_people, number_nights) {\n";
    result += "  if (type == \"powered\")\n";
    result += "    return (number_people <= 2 ? 2500 : (number_people > 5 ? 4000 : 1500 + number_people * 500)) * number_nights;\n";
    result += "  if (type == \"unpowered\")\n";
    result += "    return (number_people <= 2 ? 2000 : (number_people > 5 ? 3500 : 1000 + number_people * 500)) * number_nights;\n";
    result += "  return 0;\n";
    result += "}\n";
    return result;
}

// This function returns the camping price explained in English for the users to read
static inline std::string getCampingPriceHTML() {
    return "Powered sites: $25 per night twinshare, $5 per additional person (to max of $40)<br />Unpowered sites: $20 per night twinshare, $5 per additional person (to max of $35)";
}

// dinner prices
#define PRICE_DINNER_ADULT 2400
#define PRICE_DINNER_CHILD 1200

#endif // PRICES_H
