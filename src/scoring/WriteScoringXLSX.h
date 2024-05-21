/**
  @file    WriteScoringXLSX.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Class for creating a XLSX (Excel) file containing the scoring spreadsheet
  The XLSX file is built by modifying a template XLSX file, which is located in the "templates" directory
  The templates directory is set by the config file "ooxmlTemplatePath" setting

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef WRITESCORINGXLSX_H
#define WRITESCORINGXLSX_H

#include <string>
#include <vector>

class WriteScoringXLSX
{
public:

    /*! \struct TeamFinds
     *  \brief Stores a team name and the list of their finds
     */
    struct TeamFinds {
        std::string team_name;
        std::vector<int> finds;
        std::vector<int> finds_extras;
        std::vector<int> finds_default_extras;
    };

    /*! \struct CachePoints
     *  \brief Stores the points for the trad caches, for a single points source eg. walking points
     */
    struct CachePoints {
        int id;
        std::string item_name;
        std::vector<int> points_value;
    };

    /*! \struct ExtraItem
     *  \brief Stores something a team gets points for that isn't a trad cache
     *         eg. Puzzles, black thunder, late return
     */
    struct ExtraItem {
        std::string item_name;
        int points_value;
    };

    /*!
     * \brief WriteScoringXLSX Constructor.
     *
     * This creates a temporary directory with a copy of the template file
     *
     * \param template_dir Path to the template directory (this is copied to the tmp directory)
     * \param cacheCount Number of traditional caches in the game (the number_game_caches setting)
     * \param findExtras List of items that teams can find that aren't trad caches (eg. puzzles, black thunder)
     * \param defaultExtras List of non-find points items (hide points, cache return, late)
     */
    WriteScoringXLSX(std::string template_dir, unsigned int cacheCount, const std::vector<ExtraItem> &findExtras, const std::vector<ExtraItem> &defaultExtras);

    /*!
     * \brief WriteScoringXLSX Destructor.
     *
     * This deletes the temporary directory
     */
    ~WriteScoringXLSX();

    /*!
     * \brief Creates the "Enter Data" sheet
     *
     * \param teams The list of competing teams
     * \param find_hide_items_count The number of items that traditional caches get points from (used to calculate the cell index of the point values in sheet 2)
     */
    void addEnterDataSheet(const std::vector<TeamFinds> &teams, unsigned int find_hide_items_count);

    /*!
     * \brief Creates the "Point Values" sheet
     *
     * \param find_points The list of items that traditional caches get find points for
     * \param hide_points The list of items that traditional caches get hide points for
     */
    void addPointValuesSheet(const std::vector<CachePoints> &find_points, const std::vector<CachePoints> &hide_points);

    /*!
     * \brief Creates the "Score Calculator" sheet
     */
    void addScoreCalculatorSheet();

    /*!
     * \brief Finalises the XLSX file, then does the ZIP compression
     *
     * The XLSX file is save in the temporary directory and the filename is returned
     * This file will be deleted when the WriteScoringXLSX object is destroyed
     *
     * \param creatorName Value for the "creator" field in the document properties
     * \return The filename of the complete XLSX file
     */
    std::string saveScoringXlsxFile(const std::string &creatorName);

private:

    // List of the styles in the styles.xml file
    enum xlsx_style {
        NO_STYLE = 0,
        GREEN_BOLD_BORDER = 1,
        TITLE_PINK = 2,
        TITLE_BLUE = 3,
        TITLE_DARK_GREY = 4,
        TITLE_LIGHT_GREY = 5,
        TITLE_BROWN = 6,
        TITLE_NO_COLOR = 7,
        TEAM_NAME = 8,
        GRID_PINK = 9,
        GRID_BLUE = 10,
        GRID_DARK_GREY = 11,
        GRID_LIGHT_GREY = 12,
        GRID_BROWN = 13,
        TOTALS_ROW = 14,
        HYPERLINK_BOLD = 15,
        BOLD_NO_COLOR = 16,
        BOLD_NO_COLOR_CENTER = 17,
        TEXT_CENTER = 18,
        RED_BACKGROUND = 19,
        HYPERLINK = 20,
        RED_TEXT = 21,
        TEAM_NAME_BROWN = 22,
    };

    struct relationship {
        std::string id;
        std::string type;
        std::string target;
    };

    struct sheet_name {
        std::string display_name;
        std::string file_name;
    };

    unsigned int m_cacheCount;
    std::vector<ExtraItem> m_findExtras;
    std::vector<ExtraItem> m_defaultExtras;

    sheet_name enter_data_sheet;
    sheet_name point_values_sheet;
    sheet_name calculator_sheet;

    std::string tmp_dir; // the temporary directory used for storing parts of the XLSX file during construction
    std::string zip_dir; // the directory that is the base of the ZIP archive (will be inside the temporary directory)

    // List of strings for the shared strings file
    std::vector<std::string> sharedStringsList;

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
     * \brief Makes the XML for an empty cell (used for applying a style to a cell without content)
     *
     * \param x The x coordinate of the cell (starting at 1)
     * \param y The y coordinate of the cell (starting at 1)
     * \param style The style to apply to the cell
     * \return The XML encoded cell
     */
    static std::string makeEmptyCell(unsigned int x, unsigned int y, xlsx_style style = NO_STYLE);

    /*!
     * \brief Makes the XML for a cell containing a string
     *
     * \param x The x coordinate of the cell (starting at 1)
     * \param y The y coordinate of the cell (starting at 1)
     * \param value The string to put in the cell
     * \param style The style to apply to the cell
     * \return The XML encoded cell
     */
    std::string makeStringCell(unsigned int x, unsigned int y, const std::string &value, xlsx_style style = NO_STYLE);

    /*!
     * \brief Makes the XML for a cell containing a number
     *
     * \param x The x coordinate of the cell (starting at 1)
     * \param y The y coordinate of the cell (starting at 1)
     * \param value The number to put in the cell
     * \param style The style to apply to the cell
     * \return The XML encoded cell
     */
    static std::string makeNumberCell(unsigned int x, unsigned int y, int value, xlsx_style style = NO_STYLE);

    /*!
     * \brief Makes the XML for a cell containing a formula
     *
     * \param x The x coordinate of the cell (starting at 1)
     * \param y The y coordinate of the cell (starting at 1)
     * \param formula The formula to put in the cell
     * \param style The style to apply to the cell
     * \return The XML encoded cell
     */
    static std::string makeFormulaCell(unsigned int x, unsigned int y, const std::string &formula, xlsx_style style = NO_STYLE);

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
     * \param creatorName Value for the "creator" field in the document properties
     * \return The XML encoded data
     */
    static std::string makeCoreXML(const std::string &creatorName);

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
     * \brief Takes the sheetData XML, puts it into a worksheet file and saves it
     *
     * \param filename The file name to write the XML data to
     * \param sheetDataXML The sheetData XML for the sheet
     */
    void addWorksheetFromXML(const std::string &filename, const std::string &sheetDataXML);

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

#endif // WRITESCORINGXLSX_H
