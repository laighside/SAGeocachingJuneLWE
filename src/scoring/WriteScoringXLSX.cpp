/**
  @file    WriteScoringXLSX.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Class for creating a XLSX (Excel) file containing the scoring spreadsheet
  The XLSX file is built by modifying a template XLSX file, which is located in the "templates" directory
  The templates directory is set by the config file "ooxmlTemplatePath" setting

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "WriteScoringXLSX.h"
#include <stdexcept>

#include "../core/Encoder.h"
#include "../core/JlweUtils.h"

// The number of rows in the spreadsheet to reserve for the list of teams
#define MAX_TEAM_COUNT 50

WriteScoringXLSX::WriteScoringXLSX(std::string template_dir, unsigned int cacheCount, const std::vector<ExtraItem> &findExtras, const std::vector<ExtraItem> &defaultExtras) :
    WriteXLSX(template_dir + "/scoring")
{
    this->m_cacheCount = cacheCount;
    this->m_findExtras = findExtras;
    this->m_defaultExtras = defaultExtras;

    this->enter_data_sheet_display_name = "Enter Data";
    this->point_values_sheet_display_name = "Point Values";
    this->calculator_sheet_display_name = "Score Calculator";
}

WriteScoringXLSX::~WriteScoringXLSX() {
    // do nothing
}

void WriteScoringXLSX::addEnterDataSheet(const std::vector<TeamFinds> &teams, unsigned int find_hide_items_count) {
    std::string sheetData = "";

    unsigned int extrasStartColId = this->m_cacheCount + 3;
    unsigned int extrasEndColId = extrasStartColId + this->m_findExtras.size() + this->m_defaultExtras.size();

    sheetData += "<sheetViews>\n";
    sheetData += "  <sheetView workbookViewId=\"0\">\n";
    sheetData += "    <pane xSplit=\"1\" ySplit=\"1\" topLeftCell=\"B2\" activePane=\"bottomRight\" state=\"frozen\"/>\n";
    sheetData += "  </sheetView>\n";
    sheetData += "</sheetViews>\n";
    sheetData += "<cols>\n";
    sheetData += "  <col min=\"1\" max=\"1\" width=\"35\" customWidth=\"1\"/>\n";
    sheetData += "  <col min=\"2\" max=\"" + std::to_string(this->m_cacheCount + 1) + "\" width=\"4\" customWidth=\"1\"/>\n";
    sheetData += "  <col min=\"" + std::to_string(this->m_cacheCount + 2) + "\" max=\"" + std::to_string(this->m_cacheCount + 2) + "\" width=\"4\" customWidth=\"1\"/>\n";
    sheetData += "  <col min=\"" + std::to_string(extrasStartColId) + "\" max=\"" + std::to_string(extrasEndColId) + "\" width=\"8\" customWidth=\"1\"/>\n";
    sheetData += "  <col min=\"" + std::to_string(extrasEndColId + 2) + "\" max=\"" + std::to_string(extrasEndColId + 3) + "\" width=\"10\" customWidth=\"1\"/>\n";
    sheetData += "</cols>\n";

    sheetData += "<sheetData>\n";
    unsigned int rowId = 0;

    // Title row
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    sheetData += makeStringCell(1, 1, "Teams", GREEN_BOLD_BORDER);

    //unsigned int colId = 0;
    for (unsigned int i = 0; i < this->m_cacheCount; i++) {
        bool isOddPage = ((i / 10) % 2 == 0);
        sheetData += makeStringCell(i + 2, 1, "C" + std::to_string(i + 1), isOddPage ? TITLE_PINK : TITLE_BLUE);
    }

    sheetData += makeEmptyCell(extrasStartColId - 1, 1, TITLE_DARK_GREY);
    for (unsigned int i = 0; i < this->m_findExtras.size(); i++)
        sheetData += makeStringCell(extrasStartColId + i, 1, this->m_findExtras.at(i).item_name, TITLE_LIGHT_GREY);

    sheetData += makeEmptyCell(extrasStartColId + this->m_findExtras.size(), 1, TITLE_LIGHT_GREY);
    for (unsigned int i = 0; i < this->m_defaultExtras.size(); i++)
        sheetData += makeStringCell(extrasStartColId + this->m_findExtras.size() + 1 + i, 1, this->m_defaultExtras.at(i).item_name, TITLE_BROWN);

    sheetData += makeStringCell(extrasEndColId + 2, 1, "Trad Found", TITLE_NO_COLOR);
    sheetData += makeStringCell(extrasEndColId + 3, 1, "Extras Found", TITLE_NO_COLOR);

    sheetData += "</row>\n";

    // Find grid rows
    for (unsigned int j = 0; j < MAX_TEAM_COUNT; j++) {

        sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
        bool hasTeam = (teams.size() > j);
        if (hasTeam) {
            sheetData += makeStringCell(1, j + 2, teams.at(j).team_name, TEAM_NAME);
        } else {
            sheetData += makeEmptyCell(1, j + 2, TEAM_NAME);
        }

        for (unsigned int i = 0; i < this->m_cacheCount; i++) {
            bool isOddPage = ((i / 10) % 2 == 0);
            if (hasTeam && teams.at(j).finds.size() > i) {
                int value = teams.at(j).finds.at(i);
                if (value) {
                    sheetData += makeNumberCell(i + 2, j + 2, value, isOddPage ? GRID_PINK : GRID_BLUE);
                } else {
                    sheetData += makeEmptyCell(i + 2, j + 2, isOddPage ? GRID_PINK : GRID_BLUE);
                }
            } else {
                sheetData += makeEmptyCell(i + 2, j + 2, isOddPage ? GRID_PINK : GRID_BLUE);
            }
        }

        sheetData += makeEmptyCell(extrasStartColId - 1, j + 2, GRID_DARK_GREY);
        for (unsigned int i = 0; i < this->m_findExtras.size(); i++) {
            if (hasTeam && teams.at(j).finds_extras.size() > i) {
                int value = teams.at(j).finds_extras.at(i);
                if (value) {
                    sheetData += makeNumberCell(extrasStartColId + i, j + 2, value, GRID_LIGHT_GREY);
                } else {
                    sheetData += makeEmptyCell(extrasStartColId + i, j + 2, GRID_LIGHT_GREY);
                }
            } else {
                sheetData += makeEmptyCell(extrasStartColId + i, j + 2, GRID_LIGHT_GREY);
            }
        }

        sheetData += makeEmptyCell(extrasStartColId + this->m_findExtras.size(), j + 2, GRID_LIGHT_GREY);
        for (unsigned int i = 0; i < this->m_defaultExtras.size(); i++) {
            if (hasTeam && teams.at(j).finds_default_extras.size() > i) {
                int value = teams.at(j).finds_default_extras.at(i);
                if (value) {
                    sheetData += makeNumberCell(extrasStartColId + this->m_findExtras.size() + 1 + i, j + 2, value, GRID_BROWN);
                } else {
                    sheetData += makeEmptyCell(extrasStartColId + this->m_findExtras.size() + 1 + i, j + 2, GRID_BROWN);
                }
            } else {
                sheetData += makeEmptyCell(extrasStartColId + this->m_findExtras.size() + 1 + i, j + 2, GRID_BROWN);
            }
        }

        std::string trad_formula = "SUM(" + getCellRef(2, j + 2) + ":" +  getCellRef(this->m_cacheCount + 1, j + 2) + ")";
        sheetData += makeFormulaCell(extrasEndColId + 2, j + 2, trad_formula, NO_STYLE);
        std::string extras_formula = "SUM(" + getCellRef(extrasStartColId, j + 2) + ":" +  getCellRef(extrasStartColId + this->m_findExtras.size(), j + 2) + ")";
        sheetData += makeFormulaCell(extrasEndColId + 3, j + 2, extras_formula, NO_STYLE);

        sheetData += "</row>\n";
    }

    // Totals row
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    sheetData += makeStringCell(1, rowId, "Totals", TOTALS_ROW);
    for (unsigned int i = 0; i < (this->m_cacheCount + this->m_findExtras.size() + this->m_defaultExtras.size() + 2); i++) {
        std::string formula = "SUM(" + getCellRef(i + 2, 2) + ":" +  getCellRef(i + 2, rowId - 1) + ")";
        sheetData += makeFormulaCell(i + 2, rowId, formula, TOTALS_ROW);
    }
    sheetData += "</row>\n";

    // skip row
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\"></row>\n";

    // repeat titles
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    for (unsigned int i = 2; i <= extrasEndColId; i++)
        sheetData += makeFormulaCell(i, rowId, getCellRef(i, 1), HYPERLINK_BOLD);
    sheetData += "</row>\n";

    // Find point values
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    sheetData += makeStringCell(1, rowId, "Find points", BOLD_NO_COLOR);
    for (unsigned int i = 0; i < this->m_cacheCount; i++) {
        std::string formula = "SUM('" + this->point_values_sheet_display_name + "'!" + getCellRef(i + 2, 6) + ")";
        sheetData += makeFormulaCell(i + 2, rowId, formula, BOLD_NO_COLOR_CENTER);
    }
    for (unsigned int i = 0; i < extrasEndColId - extrasStartColId + 1; i++) {
        std::string formula = "SUM('" + this->point_values_sheet_display_name + "'!" + getCellRef(1, i * 2 + 14 + find_hide_items_count) + ")";
        sheetData += makeFormulaCell(extrasStartColId + i, rowId, formula, BOLD_NO_COLOR_CENTER);
    }
    sheetData += "</row>\n";

    sheetData += "</sheetData>\n";

    this->addWorksheetFromXML(sheetData, this->enter_data_sheet_display_name);
}

void WriteScoringXLSX::addPointValuesSheet(const std::vector<CachePoints> &find_points, const std::vector<CachePoints> &hide_points) {
    std::string sheetData = "";

    unsigned int extrasStartColId = this->m_cacheCount + 3;
    unsigned int extrasDefaultStartColId = extrasStartColId + this->m_findExtras.size() + 1;

    sheetData += "<cols>\n";
    sheetData += "  <col min=\"1\" max=\"1\" width=\"15\" customWidth=\"1\"/>\n";
    sheetData += "  <col min=\"2\" max=\"" + std::to_string(this->m_cacheCount + 1) + "\" width=\"4\" customWidth=\"1\"/>\n";
    sheetData += "</cols>\n";

    sheetData += "<sheetData>\n";

    unsigned int rowId = 0;

    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    sheetData += makeStringCell(1, rowId, "Enter the details of the hides below the red boxes, the red boxes will calculate automatically", BOLD_NO_COLOR);
    sheetData += "</row>\n";
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    sheetData += makeStringCell(1, rowId, "(everything on this page is now set by the website, any changes to these numbers will not appear on the website)", NO_STYLE);
    sheetData += "</row>\n";

    // skip row
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\"></row>\n";

    // Find points
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    sheetData += makeStringCell(1, rowId, "Finder Bonus", BOLD_NO_COLOR);
    sheetData += "</row>\n";

    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    for (unsigned int i = 0; i < this->m_cacheCount; i++)
        sheetData += makeStringCell(i + 2, rowId, "C" + std::to_string(i + 1), TEXT_CENTER);
    sheetData += "</row>\n";

    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    sheetData += makeStringCell(1, rowId, "Total", NO_STYLE);
    for (unsigned int i = 0; i < this->m_cacheCount; i++) {
        std::string formula = "SUM(" + getCellRef(i + 2, rowId + 1) + ":" +  getCellRef(i + 2, rowId + ((find_points.size() < 1) ? 1 : find_points.size())) + ")";
        sheetData += makeFormulaCell(i + 2, rowId, formula, RED_BACKGROUND);
    }
    sheetData += "</row>\n";

    for (unsigned int j = 0; j < find_points.size(); j++) {
        sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
        sheetData += makeStringCell(1, rowId, find_points.at(j).item_name, NO_STYLE);
        for (unsigned int i = 0; i < this->m_cacheCount; i++) {
            if (find_points.at(j).points_value.size() > i && find_points.at(j).points_value.at(i)) {
                sheetData += makeNumberCell(i + 2, rowId, find_points.at(j).points_value.at(i), TEXT_CENTER);
            } else {
                sheetData += makeEmptyCell(i + 2, rowId, TEXT_CENTER);
            }
        }
        sheetData += "</row>\n";
    }

    // skip row
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\"></row>\n";

    // Hide points
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    sheetData += makeStringCell(1, rowId, "Hider Bonus", BOLD_NO_COLOR);
    sheetData += makeStringCell(2, rowId, "(this isn't used?)", NO_STYLE);
    sheetData += "</row>\n";

    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    for (unsigned int i = 0; i < this->m_cacheCount; i++)
        sheetData += makeStringCell(i + 2, rowId, "C" + std::to_string(i + 1), TEXT_CENTER);
    sheetData += "</row>\n";

    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    sheetData += makeStringCell(1, rowId, "Total", NO_STYLE);
    for (unsigned int i = 0; i < this->m_cacheCount; i++) {
        std::string formula = "SUM(" + getCellRef(i + 2, rowId + 1) + ":" +  getCellRef(i + 2, rowId + ((hide_points.size() < 1) ? 1 : hide_points.size())) + ")";
        sheetData += makeFormulaCell(i + 2, rowId, formula, RED_BACKGROUND);
    }
    sheetData += "</row>\n";

    for (unsigned int j = 0; j < hide_points.size(); j++) {
        sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
        sheetData += makeStringCell(1, rowId, hide_points.at(j).item_name, NO_STYLE);
        for (unsigned int i = 0; i < this->m_cacheCount; i++) {
            if (hide_points.at(j).points_value.size() > i && hide_points.at(j).points_value.at(i)) {
                sheetData += makeNumberCell(i + 2, rowId, hide_points.at(j).points_value.at(i), TEXT_CENTER);
            } else {
                sheetData += makeEmptyCell(i + 2, rowId, TEXT_CENTER);
            }
        }
        sheetData += "</row>\n";
    }

    // skip row
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\"></row>\n";

    // Extras points
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    sheetData += makeStringCell(1, rowId, "Enter Game points where the red numbers are to reflect the current years game, do not change the headings in blue", BOLD_NO_COLOR);
    sheetData += "</row>\n";

    for (unsigned int i = 0; i < this->m_findExtras.size(); i++) {
        sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
        std::string formula = "'" + this->enter_data_sheet_display_name + "'!" + getCellRef(extrasStartColId + i, 1);
        sheetData += makeFormulaCell(1, rowId, formula, HYPERLINK);
        sheetData += "</row>\n";
        sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
        sheetData += makeNumberCell(1, rowId, this->m_findExtras.at(i).points_value, RED_TEXT);
        sheetData += "</row>\n";
    }

    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    std::string formula = "'" + this->enter_data_sheet_display_name + "'!" + getCellRef(extrasDefaultStartColId - 1, 1);
    sheetData += makeFormulaCell(1, rowId, formula, HYPERLINK);
    sheetData += "</row>\n";
    // skip row (but make it red text)
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    sheetData += makeEmptyCell(1, rowId, RED_TEXT);
    sheetData += "</row>\n";

    for (unsigned int i = 0; i < this->m_defaultExtras.size(); i++) {
        sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
        std::string formula = "'" + this->enter_data_sheet_display_name + "'!" + getCellRef(extrasDefaultStartColId + i, 1);
        sheetData += makeFormulaCell(1, rowId, formula, HYPERLINK);
        sheetData += "</row>\n";
        sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
        sheetData += makeNumberCell(1, rowId, this->m_defaultExtras.at(i).points_value, RED_TEXT);
        sheetData += "</row>\n";
    }

    sheetData += "</sheetData>\n";

    this->addWorksheetFromXML(sheetData, this->point_values_sheet_display_name);
}

void WriteScoringXLSX::addScoreCalculatorSheet() {
    std::string sheetData = "";

    unsigned int extrasStartColId = this->m_cacheCount + 3;
    unsigned int extrasEndColId = extrasStartColId + this->m_findExtras.size() + this->m_defaultExtras.size();

    sheetData += "<sheetViews>\n";
    sheetData += "  <sheetView workbookViewId=\"0\">\n";
    sheetData += "    <pane xSplit=\"1\" ySplit=\"1\" topLeftCell=\"B2\" activePane=\"bottomRight\" state=\"frozen\"/>\n";
    sheetData += "  </sheetView>\n";
    sheetData += "</sheetViews>\n";
    sheetData += "<cols>\n";
    sheetData += "  <col min=\"1\" max=\"1\" width=\"35\" customWidth=\"1\"/>\n";
    sheetData += "  <col min=\"2\" max=\"2\" width=\"8\" customWidth=\"1\"/>\n";
    sheetData += "  <col min=\"3\" max=\"" + std::to_string(this->m_cacheCount + 2) + "\" width=\"4\" customWidth=\"1\"/>\n";
    sheetData += "  <col min=\"" + std::to_string(this->m_cacheCount + 3) + "\" max=\"" + std::to_string(this->m_cacheCount + 3) + "\" width=\"2\" customWidth=\"1\"/>\n";
    sheetData += "  <col min=\"" + std::to_string(extrasStartColId + 1) + "\" max=\"" + std::to_string(extrasEndColId + 1) + "\" width=\"8\" customWidth=\"1\"/>\n";
    sheetData += "  <col min=\"" + std::to_string(extrasEndColId + 2) + "\" max=\"" + std::to_string(extrasEndColId + 2) + "\" width=\"8\" customWidth=\"1\"/>\n";
    sheetData += "</cols>\n";

    sheetData += "<sheetData>\n";

    unsigned int rowId = 0;

    // Title row
    sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";
    sheetData += makeFormulaCell(1, 1, "'" + this->enter_data_sheet_display_name + "'!" + getCellRef(1, 1), GREEN_BOLD_BORDER);
    sheetData += makeStringCell(2, 1, "Total", BOLD_NO_COLOR);

    for (unsigned int i = 0; i < this->m_cacheCount; i++)
        sheetData += makeFormulaCell(3 + i, 1, "'" + this->enter_data_sheet_display_name + "'!" + getCellRef(2 + i, 1), TITLE_NO_COLOR);
    sheetData += makeEmptyCell(extrasStartColId, 1, TITLE_NO_COLOR);
    for (unsigned int i = extrasStartColId; i <= extrasEndColId; i++)
        sheetData += makeFormulaCell(i + 1, 1, "'" + this->enter_data_sheet_display_name + "'!" + getCellRef(i, 1), TITLE_NO_COLOR);
    sheetData += makeStringCell(extrasEndColId + 2, 1, "Total", BOLD_NO_COLOR);

    sheetData += "</row>\n";

    // Find grid rows
    unsigned int point_value_row_idx = MAX_TEAM_COUNT + 5;
    for (unsigned int j = 0; j < MAX_TEAM_COUNT; j++) {

        sheetData += "<row r=\"" + std::to_string(++rowId) + "\">\n";

        sheetData += makeFormulaCell(1, j + 2, "'" + this->enter_data_sheet_display_name + "'!" + getCellRef(1, j + 2), TEAM_NAME_BROWN);
        sheetData += makeFormulaCell(2, j + 2, getCellRef(extrasEndColId + 2, j + 2), BOLD_NO_COLOR);

        for (unsigned int i = 0; i < this->m_cacheCount; i++) {
            std::string formula = "'" + this->enter_data_sheet_display_name + "'!" + getCellRef(i + 2, j + 2) + "*" + "'" + this->enter_data_sheet_display_name + "'!" + getCellRef(i + 2, point_value_row_idx);
            sheetData += makeFormulaCell(i + 3, j + 2, formula, NO_STYLE);
        }

        for (unsigned int i = extrasStartColId; i <= extrasEndColId; i++) {
            std::string formula = "'" + this->enter_data_sheet_display_name + "'!" + getCellRef(i, j + 2) + "*" + "'" + this->enter_data_sheet_display_name + "'!" + getCellRef(i, point_value_row_idx);
            sheetData += makeFormulaCell(i + 1, j + 2, formula, NO_STYLE);
        }

        sheetData += makeFormulaCell(extrasEndColId + 2, j + 2, "SUM(" + getCellRef(3, j + 2) + ":" + getCellRef(extrasEndColId + 1, j + 2) +")", BOLD_NO_COLOR);

        sheetData += "</row>\n";
    }

    sheetData += "</sheetData>\n";

    this->addWorksheetFromXML(sheetData, this->calculator_sheet_display_name);
}
