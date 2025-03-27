/**
  @file    WriteCacheListDOCX.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Class for creating a DOCX (MS Word) file containing the list of caches
  The DOCX file is built by modifying a template DOCX file, which is located in the "templates" directory
  The templates directory is set by the config file "ooxmlTemplatePath" setting

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "WriteCacheListDOCX.h"
#include <stdexcept>

#include "../core/Encoder.h"
#include "../core/JlweUtils.h"

WriteCacheListDOCX::WriteCacheListDOCX(const std::string &template_dir) :
    WriteDOCX(template_dir + "/cache_list")
{
    // do nothing
}

WriteCacheListDOCX::~WriteCacheListDOCX() {
    // do nothing
}

void WriteCacheListDOCX::makeDocumentCacheList(JlweCore *jlwe, KeyValueParser *options) {
    std::string result = "";
    result += " <w:body>\n";

    column_options_s column_options;
    std::string paragraph_settings = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\" w:beforeAutospacing=\"0\" w:afterAutospacing=\"0\"/></w:pPr>\n";

    int number_game_caches = 0;
    try {
        number_game_caches = std::stoi(jlwe->getGlobalVar("number_game_caches"));
    } catch (...) {}
    if (number_game_caches < 1)
        throw std::invalid_argument("Invalid setting for number_game_caches = " + std::to_string(number_game_caches));


    bool all_caches = (options->getValue("all_caches") == "true");
    bool rot13 = (options->getValue("rot13") == "true");

    column_options.cache_code = (options->getValue("cache_code") == "true");
    column_options.cache_name = (options->getValue("cache_name") == "true");
    column_options.team_name = (options->getValue("team_name") == "true");
    column_options.location = (options->getValue("location") == "true");
    column_options.public_hint = (options->getValue("public_hint") == "true");
    column_options.detailed_hint = (options->getValue("detailed_hint") == "true");
    column_options.camo = (options->getValue("camo") == "true");
    column_options.perm = (options->getValue("perm") == "true");
    column_options.zone_bonus = (options->getValue("zone_bonus") == "true");
    column_options.walking = (options->getValue("walking") == "true");

    std::string code_prefix = jlwe->getGlobalVar("gpx_code_prefix");

    std::string page_size = options->getValue("page_size");

    result += insertTableHeader(page_size);

    result += insertHeaderRow(column_options, paragraph_settings);

    int cache_number = 1;

    sql::Statement *stmt;
    sql::ResultSet *res;

    stmt = jlwe->getMysqlCon()->createStatement();
    res = stmt->executeQuery("SELECT cache_number,cache_name,team_name,latitude,longitude,public_hint,detailed_hint,camo,permanent,zone_bonus,osm_distance,actual_distance FROM caches;");
    while (res->next()){
        if (all_caches){
            while (cache_number != res->getInt(1)){
                if (cache_number == 51 || cache_number == 101 || cache_number == 151)
                    result += insertPageSplit(page_size, column_options, paragraph_settings);
                std::string cache_number_str = std::to_string(cache_number);
                if (cache_number_str.size() == 1)
                    cache_number_str = "0" + cache_number_str;
                std::string cache_code = Encoder::htmlEntityEncode(code_prefix + cache_number_str); //GC code
                result += insertBlankRow(cache_code, column_options, paragraph_settings);
                cache_number++;
            }
        }
        if (cache_number == 51 || cache_number == 101 || cache_number == 151)
            result += insertPageSplit(page_size, column_options, paragraph_settings);

        std::string cache_number_str = res->getString(1);
        if (cache_number_str.size() == 1)
            cache_number_str = "0" + cache_number_str;
        std::string cache_name = Encoder::htmlEntityEncode(JlweUtils::makeFullCacheName(res->getInt(1), res->getString(2), res->getInt(8), res->getInt(9)));
        std::string team_name = Encoder::htmlEntityEncode(res->getString(3));
        std::string cache_code = Encoder::htmlEntityEncode(code_prefix + cache_number_str); //GC code
        if (cache_name.size() == 0)
            cache_name = cache_code;

        std::string camo = "";
        if (res->getInt(8)){
            camo = "Yes";
        }
        std::string perm = "";
        if (res->getInt(9)){
            perm = "Yes";
        }

        std::string public_hint = res->getString(6);
        std::string detailed_hint = res->getString(7);
        if (rot13){
            public_hint = Encoder::ROT13Encode(public_hint);
            detailed_hint = Encoder::ROT13Encode(detailed_hint);
        }

        int walking = res->getInt(12);
        if (walking < 0)
            walking = res->getInt(11);

        result += "    <w:tr>\n";
        if (column_options.cache_code){
            result += "      <w:tc><w:p>\n";
            result += paragraph_settings;
            result += "      <w:r><w:t>" + cache_code + "</w:t></w:r></w:p></w:tc>\n";
        }
        if (column_options.cache_name){
            result += "      <w:tc><w:p>\n";
            result += paragraph_settings;
            result += "      <w:r><w:t>" + cache_name + "</w:t></w:r></w:p></w:tc>\n";
        }
        if (column_options.team_name){
            result += "      <w:tc><w:p>\n";
            result += paragraph_settings;
            result += "      <w:r><w:t>" + team_name + "</w:t></w:r></w:p></w:tc>\n";
        }
        if (column_options.location){
            result += "      <w:tc><w:p>\n";
            result += paragraph_settings;
            result += "      <w:r><w:t>" + JlweUtils::makeCoordString(res->getDouble(4), res->getDouble(5)) + "</w:t></w:r></w:p></w:tc>\n";
        }
        if (column_options.public_hint){
            result += "      <w:tc><w:p>\n";
            result += paragraph_settings;
            result += "      <w:r><w:t>" + Encoder::htmlEntityEncode(public_hint) + "</w:t></w:r></w:p></w:tc>\n";
        }
        if (column_options.detailed_hint){
            result += "      <w:tc><w:p>\n";
            result += paragraph_settings;
            result += "      <w:r><w:t>" + Encoder::htmlEntityEncode(detailed_hint) + "</w:t></w:r></w:p></w:tc>\n";
        }
        if (column_options.camo){
            result += "      <w:tc><w:p>\n";
            result += paragraph_settings;
            result += "      <w:r><w:t>" + camo + "</w:t></w:r></w:p></w:tc>\n";
        }
        if (column_options.perm){
            result += "      <w:tc><w:p>\n";
            result += paragraph_settings;
            result += "      <w:r><w:t>" + perm + "</w:t></w:r></w:p></w:tc>\n";
        }
        if (column_options.zone_bonus){
            result += "      <w:tc><w:p>\n";
            result += paragraph_settings;
            result += "      <w:r><w:t>" + res->getString(10) + "</w:t></w:r></w:p></w:tc>\n";
        }
        if (column_options.walking){
            result += "      <w:tc><w:p>\n";
            result += paragraph_settings;
            result += "      <w:r><w:t>" +std::to_string(walking) + "m</w:t></w:r></w:p></w:tc>\n";
        }
        result += "    </w:tr>\n";

        cache_number++;
    }
    delete res;
    delete stmt;


    if (all_caches){
        while (cache_number <= number_game_caches){
            if (cache_number == 51 || cache_number == 101 || cache_number == 151)
                result += insertPageSplit(page_size, column_options, paragraph_settings);
            std::string cache_number_str = std::to_string(cache_number);
            if (cache_number_str.size() == 1)
                cache_number_str = "0" + cache_number_str;
            std::string cache_code = Encoder::htmlEntityEncode(code_prefix + cache_number_str); //GC code
            result += insertBlankRow(cache_code, column_options, paragraph_settings);
            cache_number++;
        }
    }


    result += "  </w:tbl>\n";

    result += " <w:sectPr>\n";
    if (page_size == "a3_portrait") {
        result += " <w:pgSz w:w=\"16838\" w:h=\"23812\" w:orient=\"portrait\"/>\n";
    } else if (page_size == "a3_landscape") {
        result += " <w:pgSz w:w=\"23812\" w:h=\"16838\" w:orient=\"landscape\"/>\n";
    } else { //default to A4
        result += " <w:pgSz w:w=\"11906\" w:h=\"16838\" w:orient=\"portrait\"/>\n";
    }
    result += "  <w:pgMar w:top=\"567\" w:right=\"567\" w:bottom=\"567\" w:left=\"567\" w:header=\"567\" w:footer=\"567\" w:gutter=\"0\"/>\n";
    result += " </w:sectPr>\n";

    result += " </w:body>\n";
    this->makeDocumentFromXML(result);
}

void WriteCacheListDOCX::makeDocumentOwnerList(JlweCore *jlwe, KeyValueParser *options) {
    std::string result = "";
    result += " <w:body>\n";

    std::string paragraph_settings = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\" w:beforeAutospacing=\"0\" w:afterAutospacing=\"0\"/></w:pPr>\n";

    bool sort_by_name = (options->getValue("sort_by_name") == "true");

    std::string page_size = options->getValue("page_size", "a4");

    result += insertTableHeader(page_size);

    sql::Statement *stmt;
    sql::ResultSet *res;

    std::vector<owner_entry_s> cache_owners;
    stmt = jlwe->getMysqlCon()->createStatement();
    if (sort_by_name) {
        res = stmt->executeQuery("SELECT cache_number,owner_name,returned FROM cache_handout ORDER BY IF (owner_name <> '', 0, 1), owner_name, cache_number;");
    } else {
        res = stmt->executeQuery("SELECT cache_number,owner_name,returned FROM cache_handout ORDER BY cache_number;");
    }
    while (res->next()){
        cache_owners.push_back({res->getInt(1), res->getString(2), res->getInt(3)});
    }

    unsigned int half_size = (cache_owners.size() + 1) / 2;
    for (unsigned int i = 0; i < half_size; i++) {
        owner_entry_s owner_left = {0, "", 0};
        if (i < cache_owners.size())
            owner_left = cache_owners.at(i);
        owner_entry_s owner_right = {0, "", 0};
        if (half_size + i < cache_owners.size())
            owner_right = cache_owners.at(half_size + i);

        std::string cache_number_left_str = owner_left.cache_number ? std::to_string(owner_left.cache_number) : "";
        if (cache_number_left_str.size() == 1)
            cache_number_left_str = "0" + cache_number_left_str;
        std::string cache_number_right_str = owner_right.cache_number ?  std::to_string(owner_right.cache_number) : "";
        if (cache_number_right_str.size() == 1)
            cache_number_right_str = "0" + cache_number_right_str;

        result += "    <w:tr>\n";

        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t>" + cache_number_left_str + "</w:t></w:r></w:p></w:tc>\n";
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t>" + Encoder::htmlEntityEncode(owner_left.owner_name) + "</w:t></w:r></w:p></w:tc>\n";
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t>" + std::string(owner_left.returned ? "Y" : "") + "</w:t></w:r></w:p></w:tc>\n";

        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t>" + cache_number_right_str + "</w:t></w:r></w:p></w:tc>\n";
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t>" + Encoder::htmlEntityEncode(owner_right.owner_name) + "</w:t></w:r></w:p></w:tc>\n";
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t>" + std::string(owner_right.returned ? "Y" : "") + "</w:t></w:r></w:p></w:tc>\n";

        result += "    </w:tr>\n";
    }
    delete res;
    delete stmt;

    result += "  </w:tbl>\n";

    result += " <w:sectPr>\n";
    if (page_size == "a3_portrait") {
        result += " <w:pgSz w:w=\"16838\" w:h=\"23812\" w:orient=\"portrait\"/>\n";
    } else if (page_size == "a3_landscape") {
        result += " <w:pgSz w:w=\"23812\" w:h=\"16838\" w:orient=\"landscape\"/>\n";
    } else { //default to A4
        result += " <w:pgSz w:w=\"11906\" w:h=\"16838\" w:orient=\"portrait\"/>\n";
    }
    result += "  <w:pgMar w:top=\"567\" w:right=\"567\" w:bottom=\"567\" w:left=\"567\" w:header=\"567\" w:footer=\"567\" w:gutter=\"0\"/>\n";
    result += " </w:sectPr>\n";

    result += " </w:body>\n";
    this->makeDocumentFromXML(result);
}

std::string WriteCacheListDOCX::insertBlankRow(std::string cache_code, column_options_s column_options, std::string paragraph_settings){
    std::string result = "";
    result += "    <w:tr>\n";
    if (column_options.cache_code){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t>" + cache_code + "</w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.cache_name){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t></w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.team_name){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t></w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.location){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t></w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.public_hint){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t></w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.detailed_hint){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t></w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.camo){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t></w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.perm){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t></w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.zone_bonus){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t></w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.walking){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:t></w:t></w:r></w:p></w:tc>\n";
    }
    result += "    </w:tr>\n";
    return result;
}

std::string WriteCacheListDOCX::insertTableHeader(std::string page_size){
    std::string result = "";
    result += "  <w:tbl>\n";
    result += "    <w:tblPr>\n";
    result += "      <w:tblStyle w:val=\"TableGrid\" />\n";
    if (page_size == "a3_portrait"){
        result += "      <w:tblW w:w=\"15706\" w:type=\"dxa\" />\n";
    } else if (page_size == "a3_landscape"){
        result += "      <w:tblW w:w=\"22680\" w:type=\"dxa\" />\n";
    } else { //default to A4
        result += "      <w:tblW w:w=\"10774\" w:type=\"dxa\" />\n";
    }
    result += "      <w:tblBorders>\n";
    result += "        <w:top w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>\n";
    result += "        <w:left w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>\n";
    result += "        <w:bottom w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>\n";
    result += "        <w:right w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>\n";
    result += "        <w:insideH w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>\n";
    result += "        <w:insideV w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>\n";
    result += "      </w:tblBorders>\n";
    result += "    </w:tblPr>\n";
    return result;
}

std::string WriteCacheListDOCX::insertHeaderRow(column_options_s column_options, std::string paragraph_settings){
    std::string result = "";
    result += "    <w:tr>\n";
    if (column_options.cache_code){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:rPr><w:b/></w:rPr><w:t>Code</w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.cache_name){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:rPr><w:b/></w:rPr><w:t>Cache name</w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.team_name){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:rPr><w:b/></w:rPr><w:t>Team name</w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.location){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:rPr><w:b/></w:rPr><w:t>Coordinates</w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.public_hint){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:rPr><w:b/></w:rPr><w:t>Public Hint</w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.detailed_hint){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:rPr><w:b/></w:rPr><w:t>Detailed Hint</w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.camo){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:rPr><w:b/></w:rPr><w:t>Camo</w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.perm){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:rPr><w:b/></w:rPr><w:t>Perm</w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.zone_bonus){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:rPr><w:b/></w:rPr><w:t>Zone Bonus</w:t></w:r></w:p></w:tc>\n";
    }
    if (column_options.walking){
        result += "      <w:tc><w:p>\n";
        result += paragraph_settings;
        result += "      <w:r><w:rPr><w:b/></w:rPr><w:t>Walking</w:t></w:r></w:p></w:tc>\n";
    }
    result += "    </w:tr>\n";
    return result;
}

std::string WriteCacheListDOCX::insertPageSplit(std::string page_size, column_options_s column_options, std::string paragraph_settings){
    std::string result = "";
    //end table
    result += "  </w:tbl>\n";

    //page break
    result += "  <w:p>\n";
    result += "  <w:r><w:br w:type=\"page\"/></w:r>\n";
    result += "  </w:p>\n";

    //start new table
    result += insertTableHeader(page_size);

    result += insertHeaderRow(column_options, paragraph_settings);
    return result;
}
