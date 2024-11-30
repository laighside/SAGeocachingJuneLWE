/**
  @file    WriteXLSX.cpp
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
#include "WriteXLSX.h"
#include <stdexcept>

#include <iomanip>
#include <sstream>

#include "../core/Encoder.h"
#include "../core/JlweUtils.h"

WriteXLSX::WriteXLSX(const std::string &template_dir) {
    // make temp folder and filenames
    char dir_template[] = "/tmp/tmpdir.XXXXXX";
    char *dir_name = mkdtemp(dir_template);
    if (dir_name == nullptr)
        throw std::runtime_error("Unable to create temporary directory");
    this->tmp_dir = std::string(dir_name);
    this->zip_dir = this->tmp_dir + "/download_xlsx/";

    // copy the template to the temp folder
    int cp_result = system(("cp -r " + template_dir + " " + this->zip_dir).c_str());
    if (cp_result)
        throw std::runtime_error("Copying template directory failed: " + template_dir);

    if (system(("mkdir " + this->zip_dir + "xl/_rels").c_str()))
        throw std::runtime_error("Error while creating _rels directory");
    if (system(("mkdir " + this->zip_dir + "xl/worksheets").c_str()))
        throw std::runtime_error("Error while creating worksheets directory");

    this->defaultContentTypes = {{"rels", "application/vnd.openxmlformats-package.relationships+xml"},
                                 {"xml", "application/xml"}};
    this->overrideContentTypes = {{"/docProps/core.xml", "application/vnd.openxmlformats-package.core-properties+xml"},
                                  {"/docProps/app.xml", "application/vnd.openxmlformats-officedocument.extended-properties+xml"},
                                  {"/xl/workbook.xml", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"},
                                  {"/xl/styles.xml", "application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"},
                                  {"/xl/sharedStrings.xml", "application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"}};
}

WriteXLSX::~WriteXLSX() {
    system(("rm -r " + this->tmp_dir + "/").c_str());
}

std::string WriteXLSX::saveXlsxFile(const std::string &title, const std::string &creatorName) {
    std::vector<relationship> rels;

    for (unsigned int i = 0; i < this->worksheetList.size(); i++)
        rels.push_back({this->worksheetList.at(i).relationship_id, "http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet", "worksheets/" + this->worksheetList.at(i).file_name});
    rels.push_back({"rId" + std::to_string(this->worksheetList.size() + 2), "http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings", "sharedStrings.xml"});
    rels.push_back({"rId" + std::to_string(this->worksheetList.size() + 3), "http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles", "styles.xml"});
    writeStringToFile(makeRelationshipXML(&rels), this->zip_dir + "xl/_rels/workbook.xml.rels");

    writeStringToFile(makeSharedStringsXML(), this->zip_dir + "xl/sharedStrings.xml");
    writeStringToFile(makeCoreXML(title, creatorName), this->zip_dir + "docProps/core.xml");
    writeStringToFile(makeWorkbookXML(), this->zip_dir + "xl/workbook.xml");
    writeStringToFile(makeContentTypesXML(&this->defaultContentTypes, &this->overrideContentTypes), this->zip_dir + "[Content_Types].xml");

    // zip the folder to make single file
    std::string xlsx_filename = this->tmp_dir + "/download_xlsx.zip";
    if (system(("cd " + this->zip_dir + " ; zip -q " + xlsx_filename + " -r *").c_str()))
        throw std::runtime_error("Error while running zip compression");
    return xlsx_filename;
}

std::string WriteXLSX::makeContentTypesXML(std::vector<content_type_default> *default_types, std::vector<content_type_override> *override_types) {
    std::string result = "";

    result += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    result += "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">\n";

    for (unsigned int i = 0; i < default_types->size(); i++)
        result += " <Default Extension=\"" + Encoder::htmlAttributeEncode(default_types->at(i).extension) + "\" ContentType=\"" + Encoder::htmlAttributeEncode(default_types->at(i).type) + "\"/>\n";
    for (unsigned int i = 0; i < override_types->size(); i++)
        result += " <Override PartName=\"" + Encoder::htmlAttributeEncode(override_types->at(i).partname) + "\" ContentType=\"" + Encoder::htmlAttributeEncode(override_types->at(i).type) + "\"/>\n";

    result += "</Types>";

    return result;
}

std::string WriteXLSX::makeRelationshipXML(std::vector<WriteXLSX::relationship> *relationships) {
    std::string result = "";

    result += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    result += "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n";
    for (unsigned int i = 0; i < relationships->size(); i++) {
        relationship rel = relationships->at(i);
        result += " <Relationship Id=\"" + Encoder::htmlAttributeEncode(rel.id) + "\" Type=\"" + Encoder::htmlAttributeEncode(rel.type) + "\" Target=\"" + Encoder::htmlAttributeEncode(rel.target) + "\"/>\n";
    }
    result += "</Relationships>";

    return result;
}

std::string WriteXLSX::makeCoreXML(const std::string &title, const std::string &creatorName) {
    std::string result = "";
    result += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    result += "<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:dcterms=\"http://purl.org/dc/terms/\" xmlns:dcmitype=\"http://purl.org/dc/dcmitype/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n";
    result += " <dc:title>" + Encoder::htmlEntityEncode(title) + "</dc:title>\n";
    result += " <dc:creator>" + Encoder::htmlEntityEncode(creatorName) + "</dc:creator>\n";
    result += " <cp:lastModifiedBy>" + Encoder::htmlEntityEncode(creatorName) + "</cp:lastModifiedBy>\n";
    result += " <dcterms:created xsi:type=\"dcterms:W3CDTF\">" + JlweUtils::timeToW3CDTF(time(nullptr)) + "</dcterms:created>\n";
    result += " <dcterms:modified xsi:type=\"dcterms:W3CDTF\">" + JlweUtils::timeToW3CDTF(time(nullptr)) + "</dcterms:modified>\n";
    result += "</cp:coreProperties>\n";
    return result;
}

size_t WriteXLSX::getSharedStringId(const std::string &str) {
    for (unsigned int i = 0; i < this->sharedStringsList.size(); i++) {
        if (this->sharedStringsList.at(i) == str)
            return i;
    }

    // not found so add new string
    this->sharedStringsList.push_back(str);
    return this->sharedStringsList.size() - 1;
}

std::string WriteXLSX::makeSharedStringsXML() {
    std::string result = "";
    result += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    result += "<sst xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" count=\"" + std::to_string(this->sharedStringsList.size()) + "\" uniqueCount=\"" + std::to_string(this->sharedStringsList.size()) + "\">\n";
    for (unsigned int i = 0; i < this->sharedStringsList.size(); i++) {
        result += "  <si>\n";
        result += "    <t>" + Encoder::htmlEntityEncode(this->sharedStringsList.at(i)) + "</t>\n";
        result += "  </si>\n";
    }
    result += "</sst>\n";
    return result;
}

std::string WriteXLSX::getCellRef(unsigned int x, unsigned int y) {
    if (x < 1 || y < 1) return "";
    std::string letter = "";
    if (x <= 26) {
        char c = 'A' - 1 + static_cast<char>(x);
        letter = std::string(&c, 1);
    } else {
        char char1 = 'A' + static_cast<char>((x - 1) / 26) - 1;
        char char2 = 'A' + static_cast<char>((x - 1) % 26);
        letter = std::string(&char1, 1) + std::string(&char2, 1);
    }
    return letter + std::to_string(y);
}

std::string WriteXLSX::makeEmptyCell(unsigned int x, unsigned int y, unsigned int styleId) {
    std::string result = "";
    result += "  <c r=\"" + getCellRef(x, y) + "\" s=\"" + std::to_string(styleId) + "\" />\n";
    return result;
}

std::string WriteXLSX::makeStringCell(unsigned int x, unsigned int y, const std::string &value, unsigned int styleId) {
    if (value.size() == 0)
        return makeEmptyCell(x, y, styleId);

    std::string result = "";
    result += "  <c r=\"" + getCellRef(x, y) + "\" s=\"" + std::to_string(styleId) + "\" t=\"s\">\n";
    result += "    <v>" + std::to_string(this->getSharedStringId(value)) + "</v>\n";
    result += "  </c>\n";
    return result;
}

std::string WriteXLSX::makeNumberCell(unsigned int x, unsigned int y, int value, unsigned int styleId) {
    std::string result = "";
    result += "  <c r=\"" + getCellRef(x, y) + "\" s=\"" + std::to_string(styleId) + "\" t=\"n\">\n";
    result += "    <v>" + std::to_string(value) + "</v>\n";
    result += "  </c>\n";
    return result;
}

std::string WriteXLSX::makeNumberCell(unsigned int x, unsigned int y, double value, unsigned int styleId) {
    std::string result = "";
    result += "  <c r=\"" + getCellRef(x, y) + "\" s=\"" + std::to_string(styleId) + "\" t=\"n\">\n";
    std::stringstream ss;
    ss << std::fixed << std::setprecision(15) << value;
    std::string value_str = ss.str();

    // MS Excel can't seem to deal with trailing zeros, so we remove them
    if (value_str.find('.') != std::string::npos) {
        while (value_str.substr(value_str.size() - 1, 1) == "0") {
            if (value_str.size() < 2)
                break;
            if (value_str.substr(value_str.size() - 2, 1) == ".")
                break;
            value_str = value_str.substr(0, value_str.size() - 1);
        }
    }

    result += "    <v>" + value_str + "</v>\n";
    result += "  </c>\n";
    return result;
}

std::string WriteXLSX::makeFormulaCell(unsigned int x, unsigned int y, const std::string &formula, unsigned int style) {
    std::string result = "";
    result += "  <c r=\"" + getCellRef(x, y) + "\" s=\"" + std::to_string(style) + "\" t=\"str\">\n";
    result += "    <f>" + formula + "</f>\n";
    result += "  </c>\n";
    return result;
}

std::string WriteXLSX::makeDateTimeCell(unsigned int x, unsigned int y, time_t value, unsigned int styleId) {
    double excelTime = static_cast<double>(value) / 86400 + 25569; // convert to excel format
    return makeNumberCell(x, y, excelTime, styleId);
}

std::string WriteXLSX::makeWorkbookXML() {
    std::string result = "";
    result += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    result += "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">\n";
    result += " <sheets>\n";

    for (unsigned int i = 0; i < this->worksheetList.size(); i++)
        result += "  <sheet name=\"" + Encoder::htmlAttributeEncode(this->worksheetList.at(i).display_name) + "\" sheetId=\"" + std::to_string(i+1) + "\" r:id=\"" + Encoder::htmlAttributeEncode(this->worksheetList.at(i).relationship_id) + "\"/>\n";

    result += " </sheets>\n";
    result += "</workbook>\n";
    return result;
}

void WriteXLSX::writeStringToFile(const std::string &data, const std::string &filename) {
    FILE * file = fopen(filename.c_str(), "w");
    if (!file)
        throw std::runtime_error("Unable to create file: " + filename);
    fwrite(data.c_str(), 1, data.size(), file);
    fclose(file);
}

void WriteXLSX::addWorksheetFromXML(const std::string &sheetDataXML, const std::string &sheetDisplayName) {
    std::string sheet_filename = "sheet" + std::to_string(this->worksheetList.size() + 1) + ".xml";
    this->addWorksheetFromXML(sheetDataXML, sheetDisplayName, sheet_filename);
}

void WriteXLSX::addWorksheetFromXML(const std::string &sheetDataXML, const std::string &sheetDisplayName, const std::string &filename) {
    std::string r_id = "rId" + std::to_string(this->worksheetList.size() + 1);
    this->worksheetList.push_back({sheetDisplayName, filename, r_id});
    this->overrideContentTypes.push_back({"/xl/worksheets/" + filename, "application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"});

    std::string worksheetXML = "";
    worksheetXML += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    worksheetXML += "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" mc:Ignorable=\"x14ac\" xmlns:x14ac=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/ac\">\n";
    worksheetXML += sheetDataXML;
    worksheetXML += "</worksheet>\n";

    writeStringToFile(worksheetXML, this->zip_dir + "xl/worksheets/" + filename);
}
