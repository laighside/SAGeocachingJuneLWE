/**
  @file    WriteXLSX.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Base class for creating XLSX (Excel) files
  The XLSX file is built by modifying a template XLSX file, which is located in the "templates" directory
  The templates directory is set by the config file "ooxmlTemplatePath" setting

  Structure of an XLSX zip file:
   |- xl/
   |  |- worksheets/
   |  |  |- sheet1.xml
   |  |  |- sheet2.xml
   |  |  |- sheet3.xml
   |  |- theme/
   |  |  |- theme1.xml  (optional)
   |  |- _rels/
   |  |  |- workbook.xml.rels
   |  |- workbook.xml
   |  |- styles.xml
   |  |- sharedStrings.xml
   |- _rels/
   |  |- .rels
   |- docProps/
   |  |- core.xml
   |  |- app.xml
   |- [Content_Types].xml

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef WRITEXLSX_H
#define WRITEXLSX_H

#include <ctime>
#include <string>
#include <vector>

class WriteXLSX
{
public:

    /*!
     * \brief WriteXLSX Constructor.
     *
     * This creates a temporary directory with a copy of the template file
     *
     * \param template_dir Path to the template directory (this is copied to the tmp directory)
     */
    WriteXLSX(const std::string &template_dir);

    /*!
     * \brief WriteXLSX Destructor.
     *
     * This deletes the temporary directory
     */
    ~WriteXLSX();

    /*!
     * \brief Finalises the XLSX file, then does the ZIP compression
     *
     * The XLSX file is saved in the temporary directory and the filename is returned
     * This file will be deleted when the WriteXLSX object is destroyed
     *
     * \param creatorName Value for the "title" field in the document properties
     * \param creatorName Value for the "creator" field in the document properties
     * \return The filename of the complete XLSX file
     */
    std::string saveXlsxFile(const std::string &title, const std::string &creatorName);

    /*!
     * \brief Takes the sheetData XML, puts it into a worksheet file and saves it
     *
     * \param sheetDataXML The <sheetData> element and it's inner text
     * \param sheetDisplayName Name of the sheet (as displayed at bottom of MS Excel)
     * \param filename The file name to write the XML data to
     */
    void addWorksheetFromXML(const std::string &sheetDataXML, const std::string &sheetDisplayName, const std::string &filename);

    /*!
     * \brief Takes the sheetData XML, puts it into a worksheet file and saves it (overload function with default filename)
     *
     * \param sheetDataXML The <sheetData> element and it's inner text
     * \param sheetDisplayName Name of the sheet (as displayed at bottom of MS Excel)
     */
    void addWorksheetFromXML(const std::string &sheetDataXML, const std::string &sheetDisplayName);

protected:
    /*!
     * \brief Makes the XML for an empty cell (used for applying a style to a cell without content)
     *
     * \param x The x coordinate of the cell (starting at 1)
     * \param y The y coordinate of the cell (starting at 1)
     * \param style The style to apply to the cell
     * \return The XML encoded cell
     */
    static std::string makeEmptyCell(unsigned int x, unsigned int y, unsigned int styleId = 0);

    /*!
     * \brief Makes the XML for a cell containing a string
     *
     * \param x The x coordinate of the cell (starting at 1)
     * \param y The y coordinate of the cell (starting at 1)
     * \param value The string to put in the cell
     * \param style The style to apply to the cell
     * \return The XML encoded cell
     */
    std::string makeStringCell(unsigned int x, unsigned int y, const std::string &value, unsigned int styleId = 0);

    /*!
     * \brief Makes the XML for a cell containing an integer
     *
     * \param x The x coordinate of the cell (starting at 1)
     * \param y The y coordinate of the cell (starting at 1)
     * \param value The number to put in the cell
     * \param style The style to apply to the cell
     * \return The XML encoded cell
     */
    static std::string makeNumberCell(unsigned int x, unsigned int y, int value, unsigned int styleId = 0);

    /*!
     * \brief Makes the XML for a cell containing a floating point number
     *
     * \param x The x coordinate of the cell (starting at 1)
     * \param y The y coordinate of the cell (starting at 1)
     * \param value The number to put in the cell
     * \param style The style to apply to the cell
     * \return The XML encoded cell
     */
    static std::string makeNumberCell(unsigned int x, unsigned int y, double value, unsigned int styleId = 0);

    /*!
     * \brief Makes the XML for a cell containing a formula
     *
     * \param x The x coordinate of the cell (starting at 1)
     * \param y The y coordinate of the cell (starting at 1)
     * \param formula The formula to put in the cell
     * \param style The style to apply to the cell
     * \return The XML encoded cell
     */
    static std::string makeFormulaCell(unsigned int x, unsigned int y, const std::string &formula, unsigned int styleId = 0);

    /*!
     * \brief Makes the XML for a cell containing a date and/or time value
     *
     * The style should be set to a style with a date/time number format
     *
     * \param x The x coordinate of the cell (starting at 1)
     * \param y The y coordinate of the cell (starting at 1)
     * \param value The date/time value to put in the cell
     * \param style The style to apply to the cell
     * \return The XML encoded cell
     */
    static std::string makeDateTimeCell(unsigned int x, unsigned int y, time_t value, unsigned int styleId = 0);

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

    struct sheet_name {
        std::string display_name;
        std::string file_name;
        std::string relationship_id;
    };

    std::string tmp_dir; // the temporary directory used for storing parts of the XLSX file during construction
    std::string zip_dir; // the directory that is the base of the ZIP archive (will be inside the temporary directory)

    // List of content types for the [Content_Types].xml file
    std::vector<content_type_default> defaultContentTypes;
    std::vector<content_type_override> overrideContentTypes;

    // List of strings for the shared strings file
    std::vector<std::string> sharedStringsList;

    // List of worksheets that have been added to the file
    std::vector<sheet_name> worksheetList;

    /*!
     * \brief Gets the index number of a string in the shared strings list
     *
     * If not found, the string will be added to the list
     *
     * \param str The string to search for
     * \return The index
     */
    size_t getSharedStringId(const std::string &str);

    /*!
     * \brief Converts an (x,y) cell reference into the excel letter/number format (eg. "H35")
     *
     * \param x The x coordinate of the cell (starting at 1)
     * \param y The y coordinate of the cell (starting at 1)
     * \return The cell reference
     */
    static std::string getCellRef(unsigned int x, unsigned int y);

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
     * \brief Makes the XML for the workbook.xml file
     *
     * \return The XML encoded data
     */
    std::string makeWorkbookXML();

    /*!
     * \brief Makes the XML for the sharedStrings.xml file
     *
     * \return The XML encoded data
     */
    std::string makeSharedStringsXML();

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

#endif // WRITEXLSX_H
