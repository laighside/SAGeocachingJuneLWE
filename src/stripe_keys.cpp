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
#include <string>

#include "core/Encoder.h"
#include "core/JlweCore.h"
#include "prices.h"

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


        int powered_camping_sites = 0;
        try {
            powered_camping_sites = std::stoi(jlwe.getGlobalVar("powered_camping_sites"));
        } catch (...) {}
        std::cout << "var powered_camping_sites = " << powered_camping_sites << ";\n";

        stmt = jlwe.getMysqlCon()->createStatement();
        res = stmt->executeQuery("SELECT COUNT(*) FROM camping WHERE camping_type = 'powered' AND (status = 'S' OR status = 'P');");
        if (res->next()){
            std::cout << "var powered_camping_sites_taken = " << res->getInt(1) << ";\n";
        } else {
            // this should never happen, but assume all sites are taken if query fails
            std::cout << "var powered_camping_sites_taken = " << powered_camping_sites << ";\n";
        }
        delete res;
        delete stmt;

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
