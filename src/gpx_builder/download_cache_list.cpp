/**
  @file    download_cache_list.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the download at /cgi-bin/gpx_builder/download_cache_list.cgi
  This creates a MS Word document with a list of the caches. Also create the cache handout list for printing.

  This should be changed to use a docx libary of some sort.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/Encoder.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/KeyValueParser.h"

#include "WriteCacheListDOCX.h"

int main () {
    try {
        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        JlweCore jlwe;

        if (jlwe.getPermissionValue("perm_gpxbuilder")) { //if logged in

            std::string format = urlQueries.getValue("format");
            std::string type = urlQueries.getValue("type");

            if (format == "word") { // .docx format

                WriteCacheListDOCX docx(jlwe.config.at("ooxmlTemplatePath"));
                if (type == "owner") {
                    docx.makeDocumentOwnerList(&jlwe, &urlQueries);
                } else {
                    docx.makeDocumentCacheList(&jlwe, &urlQueries);
                }

                // Save the file
                std::string docx_file = docx.saveDocxFile("JLWE Cache List", jlwe.config.at("websiteDomain"));

                FILE *file = fopen(docx_file.c_str(), "rb");
                if (file) { //if file exists in filesystem
                    //output header
                    std::cout << "Content-type:application/vnd.openxmlformats-officedocument.wordprocessingml.document\r\n";
                    std::cout << "Content-Disposition: attachment; filename=jlwe_cache_" << (type == "owner" ? "owner_" : "") << "list_" << JlweUtils::getCurrentYearString() << ".docx\r\n\r\n";

                    JlweUtils::readFileToOStream(file, std::cout);
                    fclose(file);
                } else {
                    HtmlTemplate::outputPageWithMessage(&jlwe, "Error: unable to read temp docx file", "JLWE Admin area");
                }

            //} else if (format == "pdf") { // .pdf format
                // TODO: code this

            } else { // unrecognized format
                HtmlTemplate::outputPageWithMessage(&jlwe, "Unknown file format: " + format, "JLWE Admin area");
            }
        } else {
            HtmlTemplate::outputPageWithMessage(&jlwe, "You need to be logged in to view this area.", "JLWE Admin area");
        }
    } catch (sql::SQLException &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << e.what() << " (MySQL error code: " << std::to_string(e.getErrorCode()) << ")\n";
    } catch (const std::exception &e) {
        HtmlTemplate::outputHttpHtmlHeader();
        std::cout << e.what();
    }

    return 0;
}
