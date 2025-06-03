/**
  @file    ImageUtils.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Functions for dealing with resized versions of images
  All functions are static so there is no need to create instances of the ImageUtils object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "ImageUtils.h"

#include <filesystem>

std::string ImageUtils::getResizedImage(const std::string &filename, const std::string &public_upload_dir, bool re_convert) {
    // Smaller versions should be cached, not dynamically generated
    std::string resizedFile = public_upload_dir + "/.resize/" + filename;

    // check if resized file aready exists
    bool resizedCached = std::filesystem::is_regular_file(resizedFile);

    if (!resizedCached || re_convert) {
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
