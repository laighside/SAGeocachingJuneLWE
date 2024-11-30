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

#include "../ooxml/WriteXLSX.h"

class WriteScoringXLSX : public WriteXLSX
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
     * \param template_dir Path to the template directory (this is copied to the tmp directory)
     * \param cacheCount Number of traditional caches in the game (the number_game_caches setting)
     * \param findExtras List of items that teams can find that aren't trad caches (eg. puzzles, black thunder)
     * \param defaultExtras List of non-find points items (hide points, cache return, late)
     */
    WriteScoringXLSX(std::string template_dir, unsigned int cacheCount, const std::vector<ExtraItem> &findExtras, const std::vector<ExtraItem> &defaultExtras);

    /*!
     * \brief WriteScoringXLSX Destructor.
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

    unsigned int m_cacheCount;
    std::vector<ExtraItem> m_findExtras;
    std::vector<ExtraItem> m_defaultExtras;

    std::string enter_data_sheet_display_name;
    std::string point_values_sheet_display_name;
    std::string calculator_sheet_display_name;

};

#endif // WRITESCORINGXLSX_H
