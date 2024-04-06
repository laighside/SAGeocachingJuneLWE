/**
  @file    stripe_keys.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This outputs the data required by the javascript on the registration pages.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <map>
#include <string>

#include "core/Encoder.h"
#include "core/JlweCore.h"
#include "prices.h"

#include "ext/nlohmann/json.hpp"

int main () {
    try {
        JlweCore jlwe;

        sql::Statement *stmt;
	sql::ResultSet *res;

        //output header
        std::cout << "Cache-Control: no-store\r\n";
        std::cout << "Content-type:application/javascript\r\n\r\n";

        if (jlwe.config.at("stripe").value("testMode", false)) {
            std::cout << "var stripePublicKey = '" << std::string(jlwe.config.at("stripe").at("publicKeyTest")) << "';\n";
        } else {
            std::cout << "var stripePublicKey = '" << std::string(jlwe.config.at("stripe").at("publicKey")) << "';\n";
        }
        std::cout << "var stripeSessionURL = '" << "/cgi-bin/registration/submit_reg.cgi" << "';\n";
        std::cout << "var stripeMerchSessionURL = '" << "/cgi-bin/merch/submit_merch.cgi" << "';\n";

        // Get a list of camping sites already sold
        std::map<std::string, int> camping_sites_taken;
        stmt = jlwe.getMysqlCon()->createStatement();
        res = stmt->executeQuery("SELECT camping_type, COUNT(*) FROM camping WHERE (status = 'S' OR status = 'P') GROUP BY camping_type;");
        while (res->next()){
            camping_sites_taken[res->getString(1)] = res->getInt(2);
        }
        delete res;
        delete stmt;

        // Make list of camping options
        nlohmann::json camping_options = nlohmann::json::array();
        stmt = jlwe.getMysqlCon()->createStatement();
        res = stmt->executeQuery("SELECT id_string,display_name,price_code,total_available FROM camping_options WHERE active != 0;");
        while (res->next()){
            nlohmann::json jsonObject;
            jsonObject["id_string"] = res->getString(1);
            jsonObject["display_name"] = res->getString(2);
            jsonObject["price_code"] = res->getString(3);

            if (res->isNull(4)) {
                jsonObject["available"] = nullptr;
            } else {
                int sites_taken = 0;
                try {
                    sites_taken = camping_sites_taken[res->getString(1)];
                } catch (...) {}
                jsonObject["available"] = res->getInt(4) - sites_taken;
            }

            camping_options.push_back(jsonObject);
        }
        delete res;
        delete stmt;
        std::cout << "var camping_options = " << camping_options.dump() << ";\n";

        // TODO: this needs to be javascript escaped
        std::cout << "var price_camping_html = \"" << Encoder::javascriptAttributeEncode(getCampingPriceHTML()) << "\";\n";

        std::cout << getCampingPriceJS();

        std::cout << "var price_event_adult = " << std::to_string(PRICE_EVENT_ADULT / 100) << ";\n";
        std::cout << "var price_event_child = " << std::to_string(PRICE_EVENT_CHILD / 100) << ";\n";

        std::cout << "var price_dinner_adult = " << std::to_string(PRICE_DINNER_ADULT / 100) << ";\n";
        std::cout << "var price_dinner_child = " << std::to_string(PRICE_DINNER_CHILD / 100) << ";\n";

    } catch (sql::SQLException &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what() << " (MySQL error code: " << std::to_string(e.getErrorCode()) << ")\n";
    } catch (const std::exception &e) {
        std::cout << "Content-type:text/plain\r\n\r\n";
        std::cout << e.what();
    }

    return 0;
}
