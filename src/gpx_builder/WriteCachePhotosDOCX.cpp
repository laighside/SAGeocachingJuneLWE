/**
  @file    WriteCachePhotosDOCX.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Class for creating a DOCX (MS Word) file containing the list of caches
  The DOCX file is built by modifying a template DOCX file, which is located in the "templates" directory
  The templates directory is set by the config file "ooxmlTemplatePath" setting

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "WriteCachePhotosDOCX.h"
#include <filesystem>
#include <stdexcept>
#include <vector>

#include "../core/JlweUtils.h"

WriteCachePhotosDOCX::WriteCachePhotosDOCX(const std::string &template_dir) :
    WriteDOCX(template_dir + "/cache_list")
{
    this->addMimeType("jpg", "image/jpeg");
}

WriteCachePhotosDOCX::~WriteCachePhotosDOCX() {
    // do nothing
}

void WriteCachePhotosDOCX::getImageSize(const std::string &filename, int * width, int * height) {
    if (!std::filesystem::is_regular_file(filename))
        throw std::invalid_argument("Invalid filename: " + filename);

    std::string command = "identify -ping -format '%w,%h' " + filename;
    std::vector<std::string> result_str = JlweUtils::splitString(JlweUtils::runCommand(command), ',');
    if (result_str.size() != 2)
        throw std::runtime_error("identify did not return 2 values for file: " + filename);
    *width = std::stoi(result_str.at(0));
    *height = std::stoi(result_str.at(1));
}

std::string WriteCachePhotosDOCX::getResizedImage(const std::string &filename, const std::string &public_upload_dir) {
    // Smaller versions should be cached, not dynamically generated
    std::string resizedFile = public_upload_dir + "/.resize/" + filename;

    // check if resized file aready exists
    bool resizedCached = std::filesystem::is_regular_file(resizedFile);

    if (!resizedCached) {
        // Check orignal file exists
        if (!std::filesystem::is_regular_file(public_upload_dir + "/" + filename))
            throw std::invalid_argument("File not found: " + filename);

        // Resize the image to 0.7 megapixels
        // 0.7 MP is about the biggest that will fit on an A4 page
        std::string command = "convert -resize '700000@>' ";
        command += " " + public_upload_dir + "/" + filename;
        command += " " + resizedFile;
        if (system(command.c_str()))
            throw std::runtime_error("Failed to run ImageMagick convert on file: " + filename);
    }

    return resizedFile;
}

void WriteCachePhotosDOCX::makeDocumentCachePhotos(JlweCore *jlwe, KeyValueParser *options) {

    sql::Statement *stmt;
    sql::ResultSet *res;

    std::string page_size = options->getValue("page_size");
    int photos_per_page = 1;
    try {
        photos_per_page = std::stoi(options->getValue("photos_per_page"));
    } catch (...) {}
    bool full_size = JlweUtils::compareStringsNoCase(options->getValue("full_size"), "true");
    bool all_photos = JlweUtils::compareStringsNoCase(options->getValue("all_photos"), "true");
    bool one_per_cache = JlweUtils::compareStringsNoCase(options->getValue("one_per_cache"), "true");

    if (photos_per_page < 1 || photos_per_page > 2)
        throw std::invalid_argument("Only 1 or 2 photos per page is supported");

    // The unit for these is "twentieth of a point" or 1/1440 of an inch
    int page_width = 11906; // This the A4, the default
    int page_height = 16838;
    if (JlweUtils::compareStringsNoCase(page_size, "a3")) {
        page_width = 16838;
        page_height = 23812;
    }

    double title_font_size = 16.0; // The unit is "point"
    try {
        title_font_size = std::stod(options->getValue("title_font_size"));
    } catch (...) {}
    int margins = 400; // The unit is "twentieth of a point"
    try {
        margins = std::stoi(options->getValue("margins"));
    } catch (...) {}

    if (title_font_size < 1 || title_font_size > 200)
        throw std::invalid_argument("Font size must be between 1 and 200");
    if (margins < 0)
        throw std::invalid_argument("Margins must be >= 0");

    std::string public_upload_dir = jlwe->config.at("publicFileUpload").at("directory");
    size_t number_game_caches = 0;
    try {
        number_game_caches = std::stoul(jlwe->getGlobalVar("number_game_caches"));
    } catch (...) {}

    // Get a list of photos from the database
    std::string query = "SELECT p.server_filename,p.cache_number,caches.cache_name,caches.camo,caches.permanent FROM public_file_upload AS p "
                        "LEFT OUTER JOIN caches ON p.cache_number=caches.cache_number WHERE p.status = 'S'";

    // If we only want one photo of each cache, then select the biggest file (or maybe this should be something else?)
    if (one_per_cache) {
        query = "SELECT p.server_filename,p.cache_number,caches.cache_name,caches.camo,caches.permanent FROM ("
                  "SELECT public_file_upload.cache_number, public_file_upload.server_filename FROM public_file_upload "
                  "INNER JOIN ("
                    "SELECT cache_number, max(file_size) AS max_file_size FROM public_file_upload WHERE status = 'S' GROUP BY cache_number"
                  ") t ON t.cache_number = public_file_upload.cache_number and t.max_file_size = public_file_upload.file_size WHERE public_file_upload.status = 'S'"
                ") AS p LEFT OUTER JOIN caches ON p.cache_number=caches.cache_number";
    }
    query += std::string(all_photos ? ";" : " WHERE p.cache_number != 0;");

    std::vector<photo_s> photos;
    std::vector<bool> caches_with_photos(number_game_caches, false);
    stmt = jlwe->getMysqlCon()->createStatement();
    res = stmt->executeQuery(query);
    while (res->next()) {
        photo_s photo;
        photo.cache_number = res->getInt(2);

        // These should be filtered out in the SQL query so this is just an extra check
        if (photo.cache_number == 0 && !all_photos)
            continue;

        // The SQL query should only return one photo per cache but sometimes it doesn't if there are two photos with the same file size
        // So this just picks one at random
        if (one_per_cache) {
            if (photo.cache_number < caches_with_photos.size()) {
                if (caches_with_photos.at(photo.cache_number))
                    continue;
                caches_with_photos[photo.cache_number] = true;
            }
        }

        photo.full_filename = public_upload_dir + "/" + res->getString(1);
        if (!full_size)
            photo.full_filename = getResizedImage(res->getString(1), public_upload_dir);

        photo.width = 0;
        photo.height = 0;
        getImageSize(photo.full_filename, &photo.width, &photo.height);

        // Ignore photos with no height or width
        if (photo.width <= 0 || photo.height <= 0)
            continue;

        if (photo.cache_number && !res->isNull(3))
            photo.full_name = JlweUtils::makeFullCacheName(photo.cache_number, res->getString(3), res->getInt(4) != 0, res->getInt(5) != 0);
        if (photo.full_name.size() > 60)
            photo.full_name = photo.full_name.substr(0, 58) + "...";

        // Figure out which page orientation the photo would best fit on
        double best_ratio = 1.0;
        if (photos_per_page == 1) {
            if (photo.full_name.size())
                best_ratio = (static_cast<double>(page_width - (margins * 2)) - (title_font_size * 20)) / static_cast<double>(page_width - (margins * 2));
        } else if (photos_per_page == 2) {
            if (photo.full_name.size())
                best_ratio = static_cast<double>((page_height / 2) - (margins * 2)) / (static_cast<double>((page_height / 2) - (margins * 2)) - (title_font_size * 20));
        }

        if (photos_per_page == 1) {
            if ((static_cast<double>(photo.height) / static_cast<double>(photo.width)) >= best_ratio) { // portrait page
                photo.best_orientation = -1;
            } else { // landscape page
                photo.best_orientation = 1;
            }
        } else if (photos_per_page == 2) {
            if ((static_cast<double>(photo.width) / static_cast<double>(photo.height)) >= best_ratio) { // portrait page
                photo.best_orientation = -1;
            } else { // landscape page
                photo.best_orientation = 1;
            }
        } else {
            photo.best_orientation = 0; // zero means either page orientation is ok
        }
        /*if (photo.width > photo.height) { // portrait photo
            photo.best_orientation = -1;
        } else if (photo.width < photo.height) { // landscape photo
            photo.best_orientation = 1;
        } else { // square photo
            photo.best_orientation = 0;
        }*/

        if (!res->isNull(3))
            photo.name = res->getString(3);
        photos.push_back(photo);
    }
    delete res;
    delete stmt;

    if (photos.size() == 0)
        throw std::runtime_error("There are no photos to put in the document");

    // sort the list so the portrait photos are next to the other portrait photos
    std::sort(photos.begin(), photos.end(), [](const photo_s &a, const photo_s &b) {
        if (a.best_orientation < b.best_orientation) {
            return true;
        } else if (a.best_orientation > b.best_orientation) {
            return false;
        } else {
            return a.cache_number < b.cache_number;
        }
    });

    int first_landscape_idx = -1;
    for (unsigned int i = 0; i < photos.size(); i++) {
        if (photos.at(i).best_orientation == 1) {
            first_landscape_idx = i;
            break;
        }
    }

    size_t first_photo_on_landscape_page_idx = (first_landscape_idx >= 0) ? first_landscape_idx : photos.size();

    // These are to deal with the cases when there are zero photos in one of the options
    bool has_portrait_photos = (first_photo_on_landscape_page_idx > 0);
    bool has_landscape_photos = (first_photo_on_landscape_page_idx < photos.size());

    // all the things that need to be true to move an extra photo into the landscape section
    if (has_portrait_photos && has_landscape_photos) { // must be at least 1 photo in each section
        if (photos_per_page == 2) { // must be 2 photos per page
            if ((first_photo_on_landscape_page_idx % 2 == 1) && ((photos.size() - first_photo_on_landscape_page_idx) % 2 == 1)) { // must be an odd number of photos in both sections
                size_t idx_of_move = first_photo_on_landscape_page_idx - 1;
                if (photos.at(idx_of_move).best_orientation == 0) { // photo that would move must fit well in both sections
                    first_photo_on_landscape_page_idx--;
                }
            }
        }
    }
    // this might've changed, so re-calculate
    has_portrait_photos = (first_photo_on_landscape_page_idx > 0);

    // Start the XML
    std::string result = "";
    result += "<w:body>\n";

    // For portrait pages
    // unit: EMU  (1/914400 of an inch)
    double max_photo_width = static_cast<double>(page_width - (margins * 2)) / 1440 * 914400;
    double max_photo_height = static_cast<double>((page_height / photos_per_page) - (margins * 2)) / 1440 * 914400;

    int image_id_counter = 1; // This make unique ID strings for each image

    if (has_portrait_photos) {
        // Add the photos for the portrait pages
        for (unsigned int i = 0; i < first_photo_on_landscape_page_idx; i++) {
            result += "<w:p>\n";
            result += "<w:pPr><w:spacing w:after=\"200\"/><w:jc w:val=\"center\"/></w:pPr>\n"; // Align to center

            result += makeTitleAndPhotoXML(&photos.at(i), std::to_string(image_id_counter++), max_photo_width, max_photo_height, title_font_size, false);

            if (photos_per_page == 1) {
                result += "<w:r><w:br w:type=\"page\"/></w:r>";
            } else if (photos_per_page == 2) {
                if ((i % 2 == 1) || ((i == first_photo_on_landscape_page_idx - 1) && has_landscape_photos))
                    result += "<w:r><w:br w:type=\"page\"/></w:r>";
            }
            result += "</w:p>\n";
        }

        // Insert section break if needed
        if (has_landscape_photos) {
            result += "<w:p><w:pPr>\n";
            result += makeSectionPropertiesXML(page_width, page_height, false, 1, margins);
            result += "</w:pPr></w:p>\n";
        } else {
            result += makeSectionPropertiesXML(page_width, page_height, false, 1, margins);
        }
    }

    if (has_landscape_photos) {
        // For landscape pages
        // unit: EMU  (1/914400 of an inch)
        max_photo_width = static_cast<double>((page_height / photos_per_page) - (margins * 2)) / 1440 * 914400;
        max_photo_height = static_cast<double>(page_width - (margins * 2)) / 1440 * 914400;

        // Add the photos for the landscape pages
        if (photos_per_page == 1) {
            for (unsigned int i = first_photo_on_landscape_page_idx; i < photos.size(); i++) {
                result += "<w:p>\n";
                result += "<w:pPr><w:spacing w:after=\"200\"/><w:jc w:val=\"center\"/></w:pPr>\n"; // Align to center

                result += makeTitleAndPhotoXML(&photos.at(i), std::to_string(image_id_counter++), max_photo_width, max_photo_height, title_font_size, false);

                if (i != photos.size() - 1)
                    result += "<w:r><w:br w:type=\"page\"/></w:r>";
                result += "</w:p>\n";
            }
        } else if (photos_per_page == 2) {
            result += "<w:p>\n";
            result += "<w:pPr><w:spacing w:after=\"200\"/><w:jc w:val=\"center\"/></w:pPr>\n"; // Align to center
            for (unsigned int i = first_photo_on_landscape_page_idx; i < photos.size(); i++) {

                result += makeTitleAndPhotoXML(&photos.at(i), std::to_string(image_id_counter++), max_photo_width, max_photo_height, title_font_size, true);

                if (i != photos.size() - 1)
                    result += "<w:r><w:br w:type=\"column\"/></w:r>";
            }
            result += "</w:p>\n";
        }
        result += makeSectionPropertiesXML(page_height, page_width, true, photos_per_page, margins);
    }

    result += " </w:body>\n";
    this->makeDocumentFromXML(result);
}

std::string WriteCachePhotosDOCX::makeMaxSizeImageRunXML(WriteCachePhotosDOCX::photo_s * photo, const std::string &image_id, double max_photo_width, double max_photo_height) {
    // Figure out the size that makes the image as big as possible, while keeping the aspect ratio correct
    double scale_factor = std::min(max_photo_width / static_cast<double>(photo->width), max_photo_height / static_cast<double>(photo->height));
    int actual_width = static_cast<int>(static_cast<double>(photo->width) * scale_factor);
    int actual_height = static_cast<int>(static_cast<double>(photo->height) * scale_factor);

    std::string result = "";
    result += "<w:r>\n";
    result += makeImageXML(this->addMediaFile(photo->full_filename), image_id, photo->name, actual_width, actual_height);
    result += "</w:r>\n";
    return result;
}

std::string WriteCachePhotosDOCX::makeTitleAndPhotoXML(WriteCachePhotosDOCX::photo_s * photo, const std::string &image_id, double max_width, double max_height, double title_font_size, bool insert_centre_align_on_photo) {
    std::string result = "";
    if (photo->full_name.size()) {
        result += "<w:r>\n";
        result += makeRunPropertiesXML(true, false, true, title_font_size);
        result += makeTextXML(photo->full_name);
        result += "</w:r>\n";
        result += "<w:br/>\n";

        max_height -= title_font_size * 20 / 1440 * 914400 * 1.414; // Not sure what the 1.414 represents?
    }

    if (insert_centre_align_on_photo)
        result += "<w:pPr><w:spacing w:after=\"200\"/><w:jc w:val=\"center\"/></w:pPr>\n";
    result += makeMaxSizeImageRunXML(photo, image_id, max_width, max_height);

    return result;
}
