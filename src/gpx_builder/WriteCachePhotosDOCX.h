/**
  @file    WriteCachePhotosDOCX.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Class for creating a DOCX (MS Word) file containing the list of photos of the creative caches
  The DOCX file is built by modifying a template DOCX file, which is located in the "templates" directory
  The templates directory is set by the config file "ooxmlTemplatePath" setting

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef WRITECACHEPHOTOSDOCX_H
#define WRITECACHEPHOTOSDOCX_H

#include <string>

#include "../ooxml/WriteDOCX.h"
#include "../core/JlweCore.h"
#include "../core/KeyValueParser.h"

class WriteCachePhotosDOCX : public WriteDOCX
{
public:

    /*!
     * \brief WriteCachePhotosDOCX Constructor.
     *
     * \param template_dir Path to the template directory (this is copied to the tmp directory)
     */
    WriteCachePhotosDOCX(const std::string &template_dir);

    /*!
     * \brief WriteCachePhotosDOCX Destructor.
     */
    ~WriteCachePhotosDOCX();

    /*!
     * \brief Creates the document xml file with a list of photos
     *
     * \param jlwe pointer to JlweCore object
     * \param options URL query string parameters
     */
    void makeDocumentCachePhotos(JlweCore *jlwe, KeyValueParser *options);

    /*!
     * \brief Gets the size of an image in a file using ImageMagick identify
     *
     * \param filename The full filename of the image
     * \param width Pointer to where the width value will be stored
     * \param height Pointer to where the height value will be stored
     */
    static void getImageSize(const std::string &filename, int * width, int * height);

    /*!
     * \brief Gets a smaller version of an image that's in the public_upload folder
     *
     * It downsizes the images to 0.7 MP using ImageMagick convert on the first call, then caches the result
     *
     * \param filename The filename of the image (no path, just name and extension)
     * \param public_upload_dir Directory where the public_upload folder is stored
     * \return The full filename resized image
     */
    static std::string getResizedImage(const std::string &filename, const std::string &public_upload_dir);

private:

    // The info for one photo
    struct photo_s {
        int cache_number;
        std::string full_filename;
        std::string name;
        std::string full_name;
        int width;
        int height;
        int best_orientation;
    };

    /*!
     * \brief Makes the XML for an image to fill a given space
     *
     * \param photo The image
     * \param image_id Unique ID string to put on the image
     * \param max_photo_width Maximum width the image can be
     * \param max_photo_height Maximum height the image can be
     * \return The XML to put inside a paragraph element
     */
    std::string makeMaxSizeImageRunXML(photo_s * photo, const std::string &image_id, double max_photo_width, double max_photo_height);

    /*!
     * \brief Makes the XML for an image with title
     *
     * \param photo The image
     * \param image_id Unique ID string to put on the image
     * \param max_width Maximum width the image and title can be
     * \param max_height Maximum height the image and title can be
     * \param title_font_size The font size of the title
     * \param insert_centre_align_on_photo If true, a centre alignment tag is put before the photo (this seems to be needed when there are multiple columns)
     * \return The XML to put inside a paragraph element
     */
    std::string makeTitleAndPhotoXML(photo_s * photo, const std::string &image_id, double max_width, double max_height, double title_font_size, bool insert_centre_align_on_photo);

};

#endif // WRITECACHEPHOTOSDOCX_H
