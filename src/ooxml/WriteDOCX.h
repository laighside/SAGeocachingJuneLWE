/**
  @file    WriteDOCX.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Base class for creating DOCX (MS Word) files
  The DOCX file is built by modifying a template DOCX file, which is located in the "templates" directory
  The templates directory is set by the config file "ooxmlTemplatePath" setting

  Structure of an DOCX zip file:
   |- word/
   |  |- theme/
   |  |  |- theme1.xml  (optional)
   |  |- _rels/
   |  |  |- document.xml.rels
   |  |- document.xml
   |- _rels/
   |  |- .rels
   |- docProps/
   |  |- core.xml
   |  |- app.xml
   |- [Content_Types].xml

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef WRITEDOCX_H
#define WRITEDOCX_H

#include <ctime>
#include <string>
#include <vector>

class WriteDOCX
{
public:

    /*!
     * \brief WriteDOCX Constructor.
     *
     * This creates a temporary directory with a copy of the template file
     *
     * \param template_dir Path to the template directory (this is copied to the tmp directory)
     */
    WriteDOCX(const std::string &template_dir);

    /*!
     * \brief WriteDOCX Destructor.
     *
     * This deletes the temporary directory
     */
    ~WriteDOCX();

    /*!
     * \brief Finalises the DOCX file, then does the ZIP compression
     *
     * The DOCX file is saved in the temporary directory and the filename is returned
     * This file will be deleted when the WriteXLSX object is destroyed
     *
     * \param creatorName Value for the "title" field in the document properties
     * \param creatorName Value for the "creator" field in the document properties
     * \return The filename of the complete DOCX file
     */
    std::string saveDocxFile(const std::string &title, const std::string &creatorName);

    /*!
     * \brief Takes the body XML, puts it into the document file and saves it
     *
     * \param bodyXML The <w:body> element and it's inner text
     */
    void makeDocumentFromXML(const std::string &bodyXML);

    /*!
     * \brief Adds a media file to the document
     *
     * The file is copied into the /word/media folder
     *
     * \param filename The full filename of the file to add to the document
     * \return The relationship ID for the file (to be used with the makeImageXML() function)
     */
    std::string addMediaFile(const std::string &filename);

    /*!
     * \brief Adds a type to the list in the [Content_Types].xml file
     *
     * \param extension The file extension
     * \param mimeType The MIME type
     */
    void addMimeType(const std::string &extension, const std::string &mimeType);

    /*!
     * \brief Makes the XML for a section properties element <w:sectPr>
     *
     * \param pageWidth Page width in "twentieths of a point" or 1/1440 of an inch
     * \param pageHeight Page height in "twentieths of a point" or 1/1440 of an inch
     * \param isLandscape True for a landscape page, False for a portrait page
     * \param columns Number of columns to divide the page into
     * \param margins Page margin width in "twentieths of a point" or 1/1440 of an inch
     * \param header Height of the header in "twentieths of a point" or 1/1440 of an inch
     * \param footer Height of the footer in "twentieths of a point" or 1/1440 of an inch
     * \return The XML
     */
    std::string makeSectionPropertiesXML(int pageWidth, int pageHeight, bool isLandscape = false, int columns = 1, int margins = 400, int header = 400, int footer = 400);

    /*!
     * \brief Makes the XML for a run properties element <w:rPr>
     *
     * \param bold Set to true to make the text bold
     * \param italic Set to true to make the text italic
     * \param setFont Set to true to include a font size element (<w:sz>) in the XML
     * \param fontSize The font size to put in the <w:sz> element
     * \return The XML
     */
    std::string makeRunPropertiesXML(bool bold = false, bool italic = false, bool setFont = false, double fontSize = 12.0);

    /*!
     * \brief Makes the XML for a text element <w:t>
     *
     * \param text The text to put on the page
     * \return The XML
     */
    std::string makeTextXML(const std::string &text);

    /*!
     * \brief Makes the XML to place an image on the page, inside a <w:drawing> element
     *
     * \param imageRId Relationship ID for the image file
     * \param id Unique string to use as the id for the image
     * \param name The name of the image
     * \param width The width to display the image on the page, unit is EMUs
     * \param height The height to display the image on the page, unit is EMUs
     * \return The XML
     */
    std::string makeImageXML(const std::string &imageRId, const std::string &id, const std::string &name, int width, int height);

private:

    struct content_type_default {
        std::string extension;
        std::string type;
    };

    struct content_type_override {
        std::string partname;
        std::string type;
    };

    struct relationship {
        std::string id;
        std::string type;
        std::string target;
    };

    std::string tmp_dir; // the temporary directory used for storing parts of the XLSX file during construction
    std::string zip_dir; // the directory that is the base of the ZIP archive (will be inside the temporary directory)

    // List of content types for the [Content_Types].xml file
    std::vector<content_type_default> defaultContentTypes;
    std::vector<content_type_override> overrideContentTypes;

    // A list of media files in the /word/media folder
    std::vector<std::string> media_files;

    /*!
     * \brief Makes the XML for the [Content_Types].xml file
     *
     * \param default_types The list of default content types to put in the file
     * \param override_types The list of override content types to put in the file
     * \return The XML encoded data
     */
    static std::string makeContentTypesXML(std::vector<content_type_default> *default_types, std::vector<content_type_override> *override_types);

    /*!
     * \brief Makes the XML for a relationships file
     *
     * \param relationships The list of relationships to put in the file
     * \return The XML encoded data
     */
    static std::string makeRelationshipXML(std::vector<relationship> *relationships);

    /*!
     * \brief Makes the XML for the core.xml file
     *
     * \param creatorName Value for the "title" field in the document properties
     * \param creatorName Value for the "creator" field in the document properties
     * \return The XML encoded data
     */
    static std::string makeCoreXML(const std::string &title, const std::string &creatorName);

    /*!
     * \brief Writes a string to a file
     *
     * Will overwrite any existing file with the same name
     * Throws an error if it fails to write to the file
     *
     * \param data The string to write
     * \param filename The file to write to
     */
    static void writeStringToFile(const std::string &data, const std::string &filename);
};

#endif // WRITEDOCX_H
