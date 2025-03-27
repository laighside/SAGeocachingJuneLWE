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
     * \brief Takes the body XML, puts it into the docuemnt file and saves it
     *
     * \param bodyXML The <w:body> element and it's inner text
     */
    void makeDocumentFromXML(const std::string &bodyXML);

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
