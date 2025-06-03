/**
  @file    ImageUtils.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Functions for dealing with resized versions of images
  All functions are static so there is no need to create instances of the ImageUtils object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include <string>

class ImageUtils
{
public:

    /*!
     * \brief Gets a smaller version of an image that's in the public_upload folder
     *
     * It downsizes the images to 0.7 MP using ImageMagick convert on the first call, then caches the result
     *
     * \param filename The filename of the image (no path, just name and extension)
     * \param public_upload_dir Directory where the public_upload folder is stored
     * \param re_convert Setting to true will always re-convert the original file, even if a resized version already exists
     * \return The full filename resized image
     */
    static std::string getResizedImage(const std::string &filename, const std::string &public_upload_dir, bool re_convert = false);

};

#endif // IMAGEUTILS_H
