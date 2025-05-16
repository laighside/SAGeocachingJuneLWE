/**
  @file    download_cache_photos.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Makes the download at /cgi-bin/gpx_builder/download_cache_photos.cgi
  This creates a MS Word document with the photos of the creative caches. Or a ZIP file with all the photos.

  This should be changed to use a docx libary of some sort.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#include "../core/CgiEnvironment.h"
#include "../core/Encoder.h"
#include "../core/HtmlTemplate.h"
#include "../core/JlweCore.h"
#include "../core/JlweUtils.h"
#include "../core/KeyValueParser.h"

#include "WriteCachePhotosDOCX.h"

int main () {
    try {
        KeyValueParser urlQueries(CgiEnvironment::getQueryString(), true);

        JlweCore jlwe;

        if (jlwe.getPermissionValue("perm_gpxbuilder")) { //if logged in

            std::string format = urlQueries.getValue("format");
            std::string type = urlQueries.getValue("type");
            bool full_size = JlweUtils::compareStringsNoCase(urlQueries.getValue("full_size"), "true");
            bool all_photos = JlweUtils::compareStringsNoCase(urlQueries.getValue("all_photos"), "true");
            bool one_per_cache = JlweUtils::compareStringsNoCase(urlQueries.getValue("one_per_cache"), "true");

            if (format == "word") { // .docx format

                WriteCachePhotosDOCX docx(jlwe.config.at("ooxmlTemplatePath"));
                docx.makeDocumentCachePhotos(&jlwe, &urlQueries);

                // Save the file
                std::string docx_file = docx.saveDocxFile("JLWE Cache Photos", jlwe.config.at("websiteDomain"));

                FILE *file = fopen(docx_file.c_str(), "rb");
                if (file) { //if file exists in filesystem
                    //output header
                    std::cout << "Content-type:application/vnd.openxmlformats-officedocument.wordprocessingml.document\r\n";
                    std::cout << "Content-Disposition: attachment; filename=jlwe_cache_photos_" << JlweUtils::getCurrentYearString() << ".docx\r\n\r\n";

                    JlweUtils::readFileToOStream(file, std::cout);
                    fclose(file);
                } else {
                    HtmlTemplate::outputPageWithMessage(&jlwe, "Error: unable to read temp docx file", "JLWE Admin area");
                }

            //} else if (format == "pdf") { // .pdf format
                // TODO: code this

            } else if (format == "zip") { // .zip format

                sql::Statement *stmt;
                sql::ResultSet *res;

                std::string public_upload_dir = jlwe.config.at("publicFileUpload").at("directory");
                size_t number_game_caches = 0;
                try {
                    number_game_caches = std::stoul(jlwe.getGlobalVar("number_game_caches"));
                } catch (...) {}

                // Get a list of photos from the database
                std::string query = "SELECT p.server_filename,p.cache_number FROM public_file_upload AS p";

                // If we only want one photo of each cache, then select the biggest file (or maybe this should be something else?)
                if (one_per_cache) {
                    query = "SELECT p.server_filename,p.cache_number FROM ("
                              "SELECT public_file_upload.cache_number, public_file_upload.server_filename FROM public_file_upload "
                              "INNER JOIN ("
                                "SELECT cache_number, max(file_size) AS max_file_size FROM public_file_upload GROUP BY cache_number"
                              ") t on t.cache_number = public_file_upload.cache_number and t.max_file_size = public_file_upload.file_size"
                            ") AS p";
                }
                query += std::string(all_photos ? ";" : " WHERE p.cache_number != 0;");

                std::vector<bool> caches_with_photos(number_game_caches, false);
                stmt = jlwe.getMysqlCon()->createStatement();
                res = stmt->executeQuery(query);

                // make temp folder and filenames
                char dir_template[] = "/tmp/tmpdir.XXXXXX";
                char *dir_name = mkdtemp(dir_template);
                if (dir_name == nullptr)
                    throw std::runtime_error("Unable to create temporary directory");
                std::string tmp_dir = std::string(dir_name);
                std::string zip_dir = tmp_dir + "/photos/";
                if (!std::filesystem::create_directory(zip_dir))
                    throw std::runtime_error("Error while creating photos directory");

                while (res->next()) {
                    unsigned int cache_number = res->getUInt(2);

                    // These should be filtered out in the SQL query so this is just an extra check
                    if (cache_number == 0 && !all_photos)
                        continue;

                    // The SQL query should only return one photo per cache but sometimes it doesn't if there are two photos with the same file size
                    // So this just picks one at random
                    if (one_per_cache) {
                        if (cache_number < caches_with_photos.size()) {
                            if (caches_with_photos.at(cache_number))
                                continue;
                            caches_with_photos[cache_number] = true;
                        }
                    }

                    std::string filename = res->getString(1);
                    std::string full_filename = public_upload_dir + "/" + filename;
                    if (!full_size)
                        full_filename = WriteCachePhotosDOCX::getResizedImage(filename, public_upload_dir);

                    std::filesystem::copy(full_filename, zip_dir + filename);
                }
                delete res;
                delete stmt;

                // zip the folder to make single file
                std::string zip_filename = tmp_dir + "/photos.zip";
                if (system(("cd " + zip_dir + " ; zip -q " + zip_filename + " -r *").c_str()))
                    throw std::runtime_error("Error while running zip compression");

                FILE *file = fopen(zip_filename.c_str(), "rb");
                if (file) { //if file exists in filesystem
                    //output header
                    std::cout << "Content-type:application/zip\r\n";
                    std::cout << "Content-Disposition: attachment; filename=jlwe_cache_photos_" << JlweUtils::getCurrentYearString() << ".zip\r\n\r\n";

                    JlweUtils::readFileToOStream(file, std::cout);
                    fclose(file);
                } else {
                    HtmlTemplate::outputPageWithMessage(&jlwe, "Error: unable to read temp zip file", "JLWE Admin area");
                }

                std::filesystem::remove_all(tmp_dir);

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
