/**
  @file    PowerPoint.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Class for creating a PPTX (PowerPoint) file containing the presentation for Sunday night
  The PPT file is built by modifying a template PPT file, which is located at PPT_TEMPLATE_DIR
  This would probably be better if it was done using a 3rd party PPT library?

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "PowerPoint.h"
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>

#include "../core/Encoder.h"
#include "../core/JlweUtils.h"

#define PPT_TEMPLATE_DIR "/var/www/ooxml/powerpoint/template/"

PowerPoint::PowerPoint() {

    // make temp folder and filenames
    char dir_template[] = "/tmp/tmpdir.XXXXXX";
    char *dir_name = mkdtemp(dir_template);
    if (dir_name == nullptr)
        throw std::runtime_error("Unable to create temporary directory");
    this->tmp_dir = std::string(dir_name);
    this->zip_dir = this->tmp_dir + "/download_ppt/";

    // copy the template to the temp folder
    system(("cp -r " + std::string(PPT_TEMPLATE_DIR) + " " + this->zip_dir).c_str());

    this->slideCount = 0;
}

PowerPoint::~PowerPoint() {
    system(("rm -r " + this->tmp_dir + "/").c_str());
}

std::string PowerPoint::savePowerPointFile() {

    writeStringToFile(makePresentationXML(), this->zip_dir + "ppt/presentation.xml");

    std::vector<relationship> rels;
    rels.push_back({"rId1", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster", "slideMasters/slideMaster1.xml"});
    rels.push_back({"rId2", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/presProps", "presProps.xml"});
    rels.push_back({"rId3", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/viewProps", "viewProps.xml"});
    rels.push_back({"rId4", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme", "theme/theme1.xml"});
    rels.push_back({"rId5", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/tableStyles", "tableStyles.xml"});
    for (int i = 1; i <= this->slideCount; i++)
        rels.push_back({"slideId" + std::to_string(i), "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide", "slides/slide" + std::to_string(i) + ".xml"});
    writeStringToFile(makeRelationshipXML(&rels), this->zip_dir + "ppt/_rels/presentation.xml.rels");

    writeStringToFile(makeContentTypesXML(), this->zip_dir + "[Content_Types].xml");

    // zip the folder to make single file
    std::string ppt_filename = this->tmp_dir + "/download_ppt.zip";
    system(("cd " + this->zip_dir + " ; zip -q " + ppt_filename + " -r *").c_str());
    return ppt_filename;
}

std::string PowerPoint::makeRelationshipXML(std::vector<PowerPoint::relationship> *relationships) {
    std::string result = "";

    result += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    result += "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n";
    for (unsigned int i = 0; i < relationships->size(); i++) {
        relationship rel = relationships->at(i);
        result += " <Relationship Id=\"" + rel.id + "\" Type=\"" + rel.type + "\" Target=\"" + rel.target + "\"/>\n";
    }
    result += "</Relationships>";

    return result;
}

void PowerPoint::writeStandardSlideRelationship(int slideNumber) {
    std::vector<PowerPoint::relationship> rels;
    rels.push_back({"rId1", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout", "../slideLayouts/slideLayout2.xml"});
    writeStringToFile(makeRelationshipXML(&rels), this->zip_dir + "ppt/slides/_rels/slide" + std::to_string(slideNumber) + ".xml.rels");
}

std::string PowerPoint::makeContentTypesXML() {
    std::string result = "";
    result += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    result += "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">\n";
    result += " <Default Extension=\"png\" ContentType=\"image/png\"/>\n";
    result += " <Default Extension=\"jpeg\" ContentType=\"image/jpeg\"/>\n";
    result += " <Default Extension=\"svg\" ContentType=\"image/svg+xml\"/>\n";
    result += " <Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>\n";
    result += " <Default Extension=\"xml\" ContentType=\"application/xml\"/>\n";
    result += " <Override PartName=\"/ppt/presentation.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml\"/>\n";
    result += " <Override PartName=\"/ppt/slideMasters/slideMaster1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml\"/>\n";

    for (int i = 0; i < this->slideCount; i++) {
        result += " <Override PartName=\"/ppt/slides/slide" + std::to_string(i + 1) + ".xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slide+xml\"/>\n";
    }

    result += " <Override PartName=\"/ppt/presProps.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.presProps+xml\"/>\n";
    result += " <Override PartName=\"/ppt/viewProps.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.viewProps+xml\"/>\n";
    result += " <Override PartName=\"/ppt/theme/theme1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.theme+xml\"/>\n";
    result += " <Override PartName=\"/ppt/tableStyles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.tableStyles+xml\"/>\n";
    result += " <Override PartName=\"/ppt/slideLayouts/slideLayout1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml\"/>\n";
    result += " <Override PartName=\"/ppt/slideLayouts/slideLayout2.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml\"/>\n";
    result += " <Override PartName=\"/ppt/slideLayouts/slideLayout3.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml\"/>\n";
    result += " <Override PartName=\"/ppt/slideLayouts/slideLayout4.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml\"/>\n";
    result += " <Override PartName=\"/ppt/slideLayouts/slideLayout5.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml\"/>\n";
    result += " <Override PartName=\"/ppt/slideLayouts/slideLayout6.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml\"/>\n";
    result += " <Override PartName=\"/ppt/slideLayouts/slideLayout7.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml\"/>\n";
    result += " <Override PartName=\"/ppt/slideLayouts/slideLayout8.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml\"/>\n";
    result += " <Override PartName=\"/ppt/slideLayouts/slideLayout9.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml\"/>\n";
    result += " <Override PartName=\"/ppt/slideLayouts/slideLayout10.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml\"/>\n";
    result += " <Override PartName=\"/ppt/slideLayouts/slideLayout11.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml\"/>\n";
    result += " <Override PartName=\"/docProps/core.xml\" ContentType=\"application/vnd.openxmlformats-package.core-properties+xml\"/>\n";
    result += " <Override PartName=\"/docProps/app.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.extended-properties+xml\"/>\n";
    result += "</Types>\n";
    return result;
}

std::string PowerPoint::makePresentationXML() {
    std::string result = "";

    result += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    result += "<p:presentation xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" saveSubsetFonts=\"1\">\n";
    result += " <p:sldMasterIdLst>\n";
    result += "  <p:sldMasterId id=\"2147483648\" r:id=\"rId1\"/>\n";
    result += " </p:sldMasterIdLst>\n";
    result += " <p:sldIdLst>\n";
    for (int i = 0; i < this->slideCount; i++) {
        result += "  <p:sldId id=\"" + std::to_string(256 + i) + "\" r:id=\"slideId" + std::to_string(i + 1) + "\"/>\n";
    }
    result += " </p:sldIdLst>\n";
    result += " <p:sldSz cx=\"9144000\" cy=\"6858000\" type=\"screen4x3\"/>\n";
    result += " <p:notesSz cx=\"6858000\" cy=\"9144000\"/>\n";

    result += " <p:defaultTextStyle>\n";
    result += "  <a:defPPr><a:defRPr lang=\"en-US\"/></a:defPPr>\n";
    result += "  <a:lvl1pPr marL=\"0\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\">\n";
    result += "   <a:defRPr sz=\"1800\" kern=\"1200\">\n";
    result += "    <a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/><a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/>\n";
    result += "   </a:defRPr>\n";
    result += "  </a:lvl1pPr>\n";
    result += "  <a:lvl2pPr marL=\"457200\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\">\n";
    result += "   <a:defRPr sz=\"1800\" kern=\"1200\">\n";
    result += "    <a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/><a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/>\n";
    result += "   </a:defRPr>\n";
    result += "  </a:lvl2pPr>\n";
    result += "  <a:lvl3pPr marL=\"914400\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\">\n";
    result += "   <a:defRPr sz=\"1800\" kern=\"1200\">\n";
    result += "    <a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/><a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/>\n";
    result += "   </a:defRPr>\n";
    result += "  </a:lvl3pPr>\n";
    result += "  <a:lvl4pPr marL=\"1371600\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\">\n";
    result += "   <a:defRPr sz=\"1800\" kern=\"1200\">\n";
    result += "    <a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/><a:ea typeface=\"+mn-ea\"/> <a:cs typeface=\"+mn-cs\"/>\n";
    result += "   </a:defRPr>\n";
    result += "  </a:lvl4pPr>\n";
    result += "  <a:lvl5pPr marL=\"1828800\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\">\n";
    result += "   <a:defRPr sz=\"1800\" kern=\"1200\">\n";
    result += "    <a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/><a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/>\n";
    result += "   </a:defRPr>\n";
    result += "  </a:lvl5pPr>\n";
    result += "  <a:lvl6pPr marL=\"2286000\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\">\n";
    result += "   <a:defRPr sz=\"1800\" kern=\"1200\">\n";
    result += "    <a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/><a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/>\n";
    result += "   </a:defRPr>\n";
    result += "  </a:lvl6pPr>\n";
    result += "  <a:lvl7pPr marL=\"2743200\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\">\n";
    result += "   <a:defRPr sz=\"1800\" kern=\"1200\">\n";
    result += "    <a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/><a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/>\n";
    result += "   </a:defRPr>\n";
    result += "  </a:lvl7pPr>\n";
    result += "  <a:lvl8pPr marL=\"3200400\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\">\n";
    result += "   <a:defRPr sz=\"1800\" kern=\"1200\">\n";
    result += "    <a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/><a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/>\n";
    result += "   </a:defRPr>\n";
    result += "  </a:lvl8pPr>\n";
    result += "  <a:lvl9pPr marL=\"3657600\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\">\n";
    result += "   <a:defRPr sz=\"1800\" kern=\"1200\">\n";
    result += "    <a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/><a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/>\n";
    result += "   </a:defRPr>\n";
    result += "  </a:lvl9pPr>\n";
    result += " </p:defaultTextStyle>\n";

    result += "</p:presentation>\n";
    return result;
}


std::string PowerPoint::makeSlideXML(std::string content, std::vector<int> timing) {
    std::string result = "";

    result += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    result += "<p:sld xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\">\n";
    result += " <p:cSld>\n";
    result += "  <p:spTree>\n";

    result += "   <p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>\n";
    result += "   <p:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"0\" cy=\"0\"/></a:xfrm></p:grpSpPr>\n";

    result += content;

    result += "  </p:spTree>\n";
    result += " </p:cSld>\n";
    result += " <p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr>\n";
    result += " <p:timing>\n";
    result += "  <p:tnLst>\n";
    result += "   <p:par>\n";
    result += "    <p:cTn id=\"1\" dur=\"indefinite\" restart=\"never\" nodeType=\"tmRoot\">\n";
    if (timing.size() >= 2)
        result += makeTimingXML(timing);

    result += "    </p:cTn>\n";
    result += "   </p:par>\n";
    result += "  </p:tnLst>\n";
    result += " </p:timing>\n";
    result += "</p:sld>\n";

    return result;
}

std::string PowerPoint::makeTimingXML(std::vector<int> timing) {
    std::string result = "";

    result += "     <p:childTnLst>\n";
    result += "      <p:seq concurrent=\"1\" nextAc=\"seek\">\n";
    result += "       <p:cTn id=\"2\" dur=\"indefinite\" nodeType=\"mainSeq\">\n";
    result += "        <p:childTnLst>\n";

    int parCount = timing.size() / 2;

    for (int i = 0; i < parCount; i++) {

        int idStart = i * 6 + 4;
        result += "         <p:par>\n";
        result += "          <p:cTn id=\"" + std::to_string(++idStart) + "\" fill=\"hold\">\n";
        result += "           <p:stCondLst><p:cond delay=\"indefinite\"/></p:stCondLst>\n";
        result += "           <p:childTnLst>\n";
        result += "            <p:par>\n";
        result += "             <p:cTn id=\"" + std::to_string(++idStart) + "\" fill=\"hold\">\n";
        result += "              <p:stCondLst><p:cond delay=\"0\"/></p:stCondLst>\n";
        result += "              <p:childTnLst>\n";
        result += "               <p:par>\n";
        result += "                <p:cTn id=\"" + std::to_string(++idStart) + "\" presetID=\"1\" presetClass=\"entr\" presetSubtype=\"0\" fill=\"hold\" nodeType=\"clickEffect\">\n";
        result += "                 <p:stCondLst><p:cond delay=\"0\"/></p:stCondLst>\n";
        result += "                 <p:childTnLst>\n";
        result += "                  <p:set>\n";
        result += "                   <p:cBhvr>\n";
        result += "                    <p:cTn id=\"" + std::to_string(++idStart) + "\" dur=\"1\" fill=\"hold\">\n";
        result += "                     <p:stCondLst><p:cond delay=\"0\"/></p:stCondLst>\n";
        result += "                    </p:cTn>\n";
        result += "                    <p:tgtEl>\n";
        result += "                     <p:spTgt spid=\"" + std::to_string(3) + "\">\n";
        result += "                      <p:txEl>\n";
        result += "                       <p:pRg st=\"" + std::to_string(timing.at(i * 2)) + "\" end=\"" + std::to_string(timing.at(i * 2 + 1)) + "\"/>\n";
        result += "                      </p:txEl>\n";
        result += "                     </p:spTgt>\n";
        result += "                    </p:tgtEl>\n";
        result += "                    <p:attrNameLst><p:attrName>style.visibility</p:attrName></p:attrNameLst>\n";
        result += "                   </p:cBhvr>\n";
        result += "                   <p:to><p:strVal val=\"visible\"/></p:to>\n";
        result += "   	            </p:set>\n";
        result += "                 </p:childTnLst>\n";
        result += "                </p:cTn>\n";
        result += "               </p:par>\n";
        result += "              </p:childTnLst>\n";
        result += "             </p:cTn>\n";
        result += "            </p:par>\n";
        result += "           </p:childTnLst>\n";
        result += "          </p:cTn>\n";
        result += "         </p:par>\n";

    }

    result += "        </p:childTnLst>\n";
    result += "       </p:cTn>\n";
    result += "       <p:prevCondLst>\n";
    result += "	  <p:cond evt=\"onPrev\" delay=\"0\">\n";
    result += "	   <p:tgtEl>\n";
    result += "	    <p:sldTgt/>\n";
    result += "	   </p:tgtEl>\n";
    result += "	  </p:cond>\n";
    result += "       </p:prevCondLst>\n";
    result += "       <p:nextCondLst>\n";
    result += "	  <p:cond evt=\"onNext\" delay=\"0\">\n";
    result += "	   <p:tgtEl>\n";
    result += "	    <p:sldTgt/>\n";
    result += "	   </p:tgtEl>\n";
    result += "	  </p:cond>\n";
    result += "       </p:nextCondLst>\n";
    result += "      </p:seq>\n";
    result += "     </p:childTnLst>\n";

    return result;
}

std::string PowerPoint::makeSlideTitleXML(std::string title) {
    std::string result = "";

    result += "   <p:sp>\n";
    result += "    <p:nvSpPr>\n";
    result += "     <p:cNvPr id=\"2\" name=\"Title 1\"/>\n";
    result += "     <p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr>\n";
    result += "     <p:nvPr><p:ph type=\"ctrTitle\"/></p:nvPr>\n";
    result += "    </p:nvSpPr>\n";
    result += "    <p:spPr/>\n";
    result += "    <p:txBody>\n";
    result += "     <a:bodyPr/>\n";
    result += "     <a:lstStyle/>\n";
    result += "     <a:p>\n";
    result += "      <a:r>\n";
    result += "       <a:rPr lang=\"en-AU\" dirty=\"0\" smtClean=\"0\"/>\n";
    result += "       <a:t>" + Encoder::htmlEntityEncode(title) + "</a:t>\n";
    result += "      </a:r>\n";
    result += "      <a:endParaRPr lang=\"en-AU\" dirty=\"0\"/>\n";
    result += "     </a:p>\n";
    result += "    </p:txBody>\n";
    result += "   </p:sp>\n";

    return result;
}

std::string PowerPoint::makeSlideContentTextboxXML(std::string paragraphs, bool autoFit) {
    std::string result = "";

    result += "   <p:sp>\n";
    result += "    <p:nvSpPr>\n";
    result += "     <p:cNvPr id=\"3\" name=\"Content Placeholder 2\"/>\n";
    result += "     <p:cNvSpPr>\n";
    result += "      <a:spLocks noGrp=\"1\"/>\n";
    result += "     </p:cNvSpPr>\n";
    result += "     <p:nvPr>\n";
    result += "      <p:ph idx=\"1\"/>\n";
    result += "     </p:nvPr>\n";
    result += "    </p:nvSpPr>\n";
    result += "    <p:spPr/>\n";
    result += "    <p:txBody>\n";
    result += "     <a:bodyPr>\n";
    if (autoFit)
        result += "      <a:normAutofit lnSpcReduction=\"10000\"/>\n";
    result += "     </a:bodyPr>\n";
    result += "     <a:lstStyle/>\n";
    result += paragraphs;
    result += "    </p:txBody>\n";
    result += "   </p:sp>\n";

    return result;
}

std::string PowerPoint::makeSlideLineXML(std::string line, std::string sub_line) {
    std::string result = "";
    result += "     <a:p>\n";
    result += "      <a:r>\n";
    result += "       <a:rPr lang=\"en-AU\" dirty=\"0\" smtClean=\"0\"/>\n";
    result += "       <a:t>" + Encoder::htmlEntityEncode(line) + "</a:t>\n";
    result += "      </a:r>\n";
    result += "     </a:p>\n";
    if (sub_line.size()) {
        result += "     <a:p>\n";
        result += "      <a:pPr lvl=\"1\"/>\n";
        result += "      <a:r>\n";
        result += "       <a:rPr lang=\"en-AU\" dirty=\"0\" smtClean=\"0\"/>\n";
        result += "       <a:t>" + Encoder::htmlEntityEncode(sub_line) + "</a:t>\n";
        result += "      </a:r>\n";
        result += "     </a:p>\n";
    }
    return result;
}

std::string PowerPoint::makeTeamScoreLineXML(std::string team, std::string members, std::string position, std::string points) {
    unsigned int charCount = (30 - (position.size() + team.size() + points.size())) * 2;
    std::string spaces = "";
    for (unsigned int i = 0; i < charCount; i++)
        spaces += " ";

    return makeSlideLineXML(position + ": " + team + spaces + points + " points" , members);
}

void PowerPoint::writeStringToFile(const std::string &data, const std::string &filename) {
    FILE * file = fopen(filename.c_str(), "w");
    if (!file)
        throw std::runtime_error("Unable to create file: " + filename);
    fwrite(data.c_str(), 1, data.size(), file);
    fclose(file);
}

std::string PowerPoint::scoreToString(int score) {
    if (score <= -1000)
        return "DSQ";

    std::string result = std::to_string(score / 10);
    int score_dec = score % 10;
    if (score_dec != 0)
        result += "." + std::to_string(std::abs(score_dec));

    return result;
}

void PowerPoint::addSlideFromContentTextBox(std::string title, std::string contentTextBoxXML, std::vector<int> timing, bool autoFit) {

    std::string contentXml = makeSlideTitleXML(title) + makeSlideContentTextboxXML(contentTextBoxXML, autoFit);
    std::string slideXML = makeSlideXML(contentXml, timing);

    this->slideCount++;
    writeStringToFile(slideXML, this->zip_dir + "ppt/slides/slide" + std::to_string(this->slideCount) + ".xml");
    writeStandardSlideRelationship(this->slideCount);
}

void PowerPoint::addGenericSlide(const std::string &title, const std::string &text) {
    std::string pageText = "";
    std::vector<int> timing = {0};
    std::vector<std::string> lines = JlweUtils::splitString(text, '\n');
    for (unsigned int i = 0; i < lines.size(); i++) {
        std::string line = lines.at(i);
        JlweUtils::trimString(line);
        if (line.size()) {
            if (line.at(0) == '-') { // is sub-line
                line = line.substr(1); // remove the '-'
                JlweUtils::trimString(line);
                if (line.size()) {
                    pageText += "     <a:p>\n";
                    pageText += "      <a:pPr lvl=\"1\"/>\n";
                    pageText += "      <a:r>\n";
                    pageText += "       <a:rPr lang=\"en-AU\" dirty=\"0\" smtClean=\"0\"/>\n";
                    pageText += "       <a:t>" + Encoder::htmlEntityEncode(line) + "</a:t>\n";
                    pageText += "      </a:r>\n";
                    pageText += "     </a:p>\n";
                }
            } else {
                pageText += "     <a:p>\n";
                pageText += "      <a:r>\n";
                pageText += "       <a:rPr lang=\"en-AU\" dirty=\"0\" smtClean=\"0\"/>\n";
                pageText += "       <a:t>" + Encoder::htmlEntityEncode(line) + "</a:t>\n";
                pageText += "      </a:r>\n";
                pageText += "     </a:p>\n";
                if (i > 0) {
                    timing.push_back(i - 1);
                    timing.push_back(i);
                }
            }
        }
    }
    timing.push_back(static_cast<int>(lines.size()) - 1);

    if (pageText.size()) {
        addSlideFromContentTextBox(title, pageText, timing);
    } else {
        addSlideFromContentTextBox(title, pageText);
    }
}

void PowerPoint::addJlwePlacesSlide(std::vector<PowerPoint::teamScore> places) {

    int maxPos = 0;
    int minPos = 100;
    std::string score_list = "";
    std::vector<int> timing;
    int previousPosition = 0;
    int parCount = 0;
    timing.push_back(parCount);
    for (unsigned int i = 0; i < places.size(); i++) {

        PowerPoint::teamScore place = places.at(places.size() - i - 1);

        score_list += makeTeamScoreLineXML(place.team_name, place.team_members, std::to_string(place.position), scoreToString(place.score));

        int pos = place.position;
        if (pos > maxPos)
            maxPos = pos;
        if (pos < minPos)
            minPos = pos;

        if (previousPosition != pos && i > 0) {
            timing.push_back(parCount - 1);
            timing.push_back(parCount);
        }
        previousPosition = pos;

        parCount++;
        if (place.team_members.size()) parCount++;
    }
    timing.push_back(parCount - 1);

    std::string title = "Places " + std::to_string(minPos) + " to " + std::to_string(maxPos);
    if (minPos == maxPos)
        title = JlweUtils::numberToOrdinal(minPos) + " Place";
    addSlideFromContentTextBox(title, score_list, timing);
}

void PowerPoint::addJlweDisqualifiedSlide(std::vector<PowerPoint::teamScore> places) {

    std::string score_list = "";
    std::vector<int> timing;
    int parCount = 0;
    for (unsigned int i = 0; i < places.size(); i++) {

        PowerPoint::teamScore place = places.at(i);

        score_list += makeSlideLineXML(place.team_name, place.team_members);

        timing.push_back(parCount);
        parCount++;
        if (place.team_members.size()) parCount++;
        timing.push_back(parCount - 1);
    }

    addSlideFromContentTextBox("Disqualified", score_list, timing);
}

void PowerPoint::addJlweOtherPrizesSlide() {
    std::string pageText = "";
    pageText += makeSlideLineXML("Best costume", "");
    pageText += makeSlideLineXML("Best table", "");
    pageText += makeSlideLineXML("Best hats", "");
    addSlideFromContentTextBox("Other prizes", pageText);
}


void PowerPoint::addJlweBestCachesSlide(std::vector<PowerPoint::bestCache> caches) {

    std::string pageText = "";
    std::vector<int> timing;
    for (unsigned int i = 0; i < caches.size(); i++) {
        if (caches.at(i).category.size() && caches.at(i).winning_cache.size()) {
            pageText += makeSlideLineXML(caches.at(i).category, caches.at(i).winning_cache);
            timing.push_back(i * 2 + 1);
            timing.push_back(i * 2 + 1);
        }
    }
    addSlideFromContentTextBox("Best cache voting", pageText, timing, true);
}

void PowerPoint::addJlweSinglePlaceSlide(PowerPoint::teamScore team, const std::string &title, const std::string &sub_title, std::vector<int> timing) {

    std::string pageText = "";
    pageText += "     <a:p>\n";
    pageText += "      <a:r>\n";
    pageText += "       <a:rPr lang=\"en-AU\" dirty=\"0\" smtClean=\"0\"/>\n";
    pageText += "       <a:t>" + Encoder::htmlEntityEncode(sub_title) + "</a:t>\n";
    pageText += "      </a:r>\n";
    pageText += "     </a:p>\n";

    pageText += "     <a:p>\n";
    pageText += "      <a:endParaRPr lang=\"en-AU\" dirty=\"0\"/>\n";
    pageText += "     </a:p>\n";
    pageText += "     <a:p>\n";
    pageText += "      <a:endParaRPr lang=\"en-AU\" dirty=\"0\"/>\n";
    pageText += "     </a:p>\n";
    pageText += "     <a:p>\n";
    pageText += "      <a:pPr marL=\"0\" indent=\"0\" algn=\"ctr\">\n";
    pageText += "       <a:buNone/>\n";
    pageText += "      </a:pPr>\n";
    pageText += "      <a:r>\n";
    pageText += "     	<a:rPr lang=\"en-AU\" sz=\"6000\" dirty=\"0\" smtClean=\"0\"/>\n";
    pageText += "       <a:t>" + Encoder::htmlEntityEncode(team.team_name) + "</a:t>\n";
    pageText += "      </a:r>\n";
    pageText += "     </a:p>\n";
    if (team.team_members.size()) {
        pageText += "     <a:p>\n";
        pageText += "      <a:pPr marL=\"0\" indent=\"0\" algn=\"ctr\">\n";
        pageText += "       <a:buNone/>\n";
        pageText += "      </a:pPr>\n";
        pageText += "      <a:r>\n";
        pageText += "       <a:rPr lang=\"en-AU\" dirty=\"0\" smtClean=\"0\"/>\n";
        pageText += "       <a:t>" + Encoder::htmlEntityEncode(team.team_members) + "</a:t>\n";
        pageText += "      </a:r>\n";
        pageText += "      <a:endParaRPr lang=\"en-AU\" dirty=\"0\"/>\n";
        pageText += "     </a:p>\n";
    }

    addSlideFromContentTextBox(title, pageText, timing);
}


void PowerPoint::addJlweTitleSlide(const std::string &logo_file, const std::string &year, const std::string &town) {

    std::string slideXML = "";
    std::string imageRID = "rId2";

    slideXML += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    slideXML += "<p:sld xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\">\n";
    slideXML += " <p:cSld>\n";
    slideXML += "  <p:spTree>\n";

    slideXML += "   <p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>\n";
    slideXML += "   <p:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"0\" cy=\"0\"/></a:xfrm></p:grpSpPr>\n";

    slideXML += "   <p:sp>\n";
    slideXML += "    <p:nvSpPr>\n";
    slideXML += "     <p:cNvPr id=\"2\" name=\"Title 1\"/>\n";
    slideXML += "     <p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr>\n";
    slideXML += "     <p:nvPr><p:ph type=\"ctrTitle\"/></p:nvPr>\n";
    slideXML += "    </p:nvSpPr>\n";
    slideXML += "    <p:spPr>\n";
    slideXML += "      <a:xfrm>\n";
    slideXML += "       <a:off x=\"685800\" y=\"1412776\"/>\n";
    slideXML += "       <a:ext cx=\"7772400\" cy=\"1470025\"/>\n";
    slideXML += "      </a:xfrm>\n";
    slideXML += "    </p:spPr>\n";
    slideXML += "    <p:txBody>\n";
    slideXML += "     <a:bodyPr/>\n";
    slideXML += "     <a:lstStyle/>\n";
    slideXML += "     <a:p>\n";
    slideXML += "      <a:r>\n";
    slideXML += "       <a:rPr lang=\"en-AU\" dirty=\"0\" smtClean=\"0\"/>\n";
    slideXML += "       <a:t>Welcome</a:t>\n";
    slideXML += "      </a:r>\n";
    slideXML += "      <a:endParaRPr lang=\"en-AU\" dirty=\"0\"/>\n";
    slideXML += "     </a:p>\n";
    slideXML += "    </p:txBody>\n";
    slideXML += "   </p:sp>\n";

    slideXML += "   <p:sp>\n";
    slideXML += "    <p:nvSpPr>\n";
    slideXML += "     <p:cNvPr id=\"3\" name=\"Subtitle 2\"/>\n";
    slideXML += "     <p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr>\n";
    slideXML += "     <p:nvPr><p:ph type=\"subTitle\" idx=\"1\"/></p:nvPr>\n";
    slideXML += "    </p:nvSpPr>\n";
    slideXML += "    <p:spPr>\n";
    slideXML += "     <a:xfrm>\n";
    slideXML += "      <a:off x=\"1371600\" y=\"3717032\"/>\n";
    slideXML += "      <a:ext cx=\"6400800\" cy=\"1752600\"/>\n";
    slideXML += "     </a:xfrm>\n";
    slideXML += "    </p:spPr>\n";
    slideXML += "    <p:txBody>\n";
    slideXML += "     <a:bodyPr><a:noAutofit/></a:bodyPr>\n";
    slideXML += "     <a:lstStyle/>\n";
    slideXML += "     <a:p>\n";
    slideXML += "      <a:r>\n";
    slideXML += "       <a:rPr lang=\"en-AU\" sz=\"6000\" dirty=\"0\" smtClean=\"0\"/>\n";
    slideXML += "       <a:t>June Long Weekend " + Encoder::htmlEntityEncode(year) + "</a:t>\n";
    slideXML += "      </a:r>\n";
    slideXML += "      <a:endParaRPr lang=\"en-AU\" sz=\"6000\" dirty=\"0\"/>\n";
    slideXML += "     </a:p>\n";
    slideXML += "    </p:txBody>\n";
    slideXML += "   </p:sp>\n";

    slideXML += "   <p:sp>\n";
    slideXML += "    <p:nvSpPr>\n";
    slideXML += "	  <p:cNvPr id=\"4\" name=\"Town TextBox\"/>\n";
    slideXML += "     <p:cNvSpPr txBox=\"1\"/>\n";
    slideXML += "     <p:nvPr/>\n";
    slideXML += "    </p:nvSpPr>\n";
    slideXML += "    <p:spPr>\n";
    slideXML += "     <a:xfrm>\n";
    slideXML += "      <a:off x=\"1071235\" y=\"5733256\"/>\n";
    slideXML += "      <a:ext cx=\"7001531\" cy=\"646331\"/>\n";
    slideXML += "     </a:xfrm>\n";
    slideXML += "     <a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>\n";
    slideXML += "     <a:noFill/>\n";
    slideXML += "    </p:spPr>\n";
    slideXML += "    <p:txBody>\n";
    slideXML += "     <a:bodyPr wrap=\"square\" rtlCol=\"0\"><a:spAutoFit/></a:bodyPr>\n";
    slideXML += "	  <a:lstStyle/>\n";
    slideXML += "     <a:p>\n";
    slideXML += "      <a:pPr algn=\"ctr\"/>\n";
    slideXML += "       <a:r>\n";
    slideXML += "        <a:rPr lang=\"en-AU\" sz=\"3600\" dirty=\"0\" smtClean=\"0\"/>\n";
    slideXML += "        <a:t>" + Encoder::htmlEntityEncode(town) + "</a:t>\n";
    slideXML += "       </a:r>\n";
    slideXML += "      <a:endParaRPr lang=\"en-AU\" sz=\"3600\" dirty=\"0\"/>\n";
    slideXML += "     </a:p>\n";
    slideXML += "    </p:txBody>\n";
    slideXML += "   </p:sp>\n";

    slideXML += "   <p:pic>\n";
    slideXML += "    <p:nvPicPr>\n";
    slideXML += "     <p:cNvPr id=\"1026\" name=\"Picture 2\"/>\n";
    slideXML += "     <p:cNvPicPr><a:picLocks noChangeAspect=\"1\" noChangeArrowheads=\"1\"/></p:cNvPicPr>\n";
    slideXML += "     <p:nvPr/>\n";
    slideXML += "    </p:nvPicPr>\n";
    slideXML += "    <p:blipFill>\n";
    slideXML += "     <a:blip r:embed=\"" + imageRID + "\"/>\n";
    slideXML += "     <a:srcRect/>\n";
    slideXML += "     <a:stretch><a:fillRect/></a:stretch>\n";
    slideXML += "    </p:blipFill>\n";
    slideXML += "    <p:spPr bwMode=\"auto\">\n";
    slideXML += "     <a:xfrm>\n";
    slideXML += "      <a:off x=\"251520\" y=\"260648\"/>\n";
    slideXML += "      <a:ext cx=\"2381250\" cy=\"3209925\"/>\n";
    slideXML += "     </a:xfrm>\n";
    slideXML += "     <a:prstGeom prst=\"rect\">\n";
    slideXML += "      <a:avLst/>\n";
    slideXML += "     </a:prstGeom>\n";
    slideXML += "     <a:noFill/>\n";
    slideXML += "     <a:ln><a:noFill/></a:ln>\n";
    slideXML += "    </p:spPr>\n";
    slideXML += "   </p:pic>\n";

    slideXML += "   <p:pic>\n";
    slideXML += "    <p:nvPicPr>\n";
    slideXML += "     <p:cNvPr id=\"1027\" name=\"Picture 3\"/>\n";
    slideXML += "     <p:cNvPicPr><a:picLocks noChangeAspect=\"1\" noChangeArrowheads=\"1\"/></p:cNvPicPr>\n";
    slideXML += "     <p:nvPr/>\n";
    slideXML += "    </p:nvPicPr>\n";
    slideXML += "    <p:blipFill>\n";
    slideXML += "     <a:blip r:embed=\"" + imageRID + "\"/>\n";
    slideXML += "     <a:srcRect/>\n";
    slideXML += "     <a:stretch><a:fillRect/></a:stretch>\n";
    slideXML += "    </p:blipFill>\n";
    slideXML += "    <p:spPr bwMode=\"auto\">\n";
    slideXML += "     <a:xfrm>\n";
    slideXML += "      <a:off x=\"6444208\" y=\"260648\"/>\n";
    slideXML += "      <a:ext cx=\"2381250\" cy=\"3209925\"/>\n";
    slideXML += "     </a:xfrm>\n";
    slideXML += "     <a:prstGeom prst=\"rect\">\n";
    slideXML += "      <a:avLst/>\n";
    slideXML += "     </a:prstGeom>\n";
    slideXML += "     <a:noFill/>\n";
    slideXML += "     <a:ln><a:noFill/></a:ln>\n";
    slideXML += "    </p:spPr>\n";
    slideXML += "   </p:pic>\n";

    slideXML += "  </p:spTree>\n";
    slideXML += " </p:cSld>\n";
    slideXML += " <p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr>\n";
    slideXML += "</p:sld>\n";

    this->slideCount++;
    writeStringToFile(slideXML, this->zip_dir + "ppt/slides/slide" + std::to_string(this->slideCount) + ".xml");

    std::vector<PowerPoint::relationship> rels;
    rels.push_back({"rId1", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout", "../slideLayouts/slideLayout1.xml"});
    rels.push_back({imageRID, "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image", "../media/jlwe_logo.png"});
    writeStringToFile(makeRelationshipXML(&rels), this->zip_dir + "ppt/slides/_rels/slide" + std::to_string(this->slideCount) + ".xml.rels");

    system(("cp " + logo_file + " " + this->zip_dir + "ppt/media/jlwe_logo.png").c_str());

}

void PowerPoint::addJlweRisingStarSlide(const std::string &logo_file) {

    std::string slideXML = "";
    std::string imageRID = "rId2";

    slideXML += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    slideXML += "<p:sld xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\">\n";
    slideXML += " <p:cSld>\n";
    slideXML += "  <p:spTree>\n";

    slideXML += "   <p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>\n";
    slideXML += "   <p:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"0\" cy=\"0\"/></a:xfrm></p:grpSpPr>\n";

    slideXML += "   <p:sp>\n";
    slideXML += "    <p:nvSpPr>\n";
    slideXML += "     <p:cNvPr id=\"2\" name=\"Title 1\"/>\n";
    slideXML += "     <p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr>\n";
    slideXML += "     <p:nvPr><p:ph type=\"ctrTitle\"/></p:nvPr>\n";
    slideXML += "    </p:nvSpPr>\n";
    slideXML += "    <p:spPr>\n";
    slideXML += "      <a:xfrm>\n";
    slideXML += "       <a:off x=\"685800\" y=\"1670943\"/>\n";
    slideXML += "       <a:ext cx=\"7772400\" cy=\"1470025\"/>\n";
    slideXML += "      </a:xfrm>\n";
    slideXML += "    </p:spPr>\n";
    slideXML += "    <p:txBody>\n";
    slideXML += "     <a:bodyPr/>\n";
    slideXML += "     <a:lstStyle/>\n";
    slideXML += "     <a:p>\n";
    slideXML += "      <a:r>\n";
    slideXML += "       <a:rPr lang=\"en-AU\" dirty=\"0\" smtClean=\"0\"/>\n";
    slideXML += "       <a:t>Rising Star Award</a:t>\n";
    slideXML += "      </a:r>\n";
    slideXML += "      <a:endParaRPr lang=\"en-AU\" dirty=\"0\"/>\n";
    slideXML += "     </a:p>\n";
    slideXML += "    </p:txBody>\n";
    slideXML += "   </p:sp>\n";

    slideXML += "   <p:pic>\n";
    slideXML += "    <p:nvPicPr>\n";
    slideXML += "     <p:cNvPr id=\"1026\" name=\"Logo\"/>\n";
    slideXML += "     <p:cNvPicPr><a:picLocks noChangeAspect=\"1\" noChangeArrowheads=\"1\"/></p:cNvPicPr>\n";
    slideXML += "     <p:nvPr/>\n";
    slideXML += "    </p:nvPicPr>\n";
    slideXML += "    <p:blipFill>\n";
    slideXML += "     <a:blip r:embed=\"" + imageRID + "\"/>\n";
    slideXML += "     <a:srcRect/>\n";
    slideXML += "     <a:stretch><a:fillRect/></a:stretch>\n";
    slideXML += "    </p:blipFill>\n";
    slideXML += "    <p:spPr bwMode=\"auto\">\n";
    slideXML += "     <a:xfrm>\n";
    slideXML += "      <a:off x=\"2795654\" y=\"417806\"/>\n";
    slideXML += "      <a:ext cx=\"3552693\" cy=\"1499026\"/>\n";
    slideXML += "     </a:xfrm>\n";
    slideXML += "     <a:prstGeom prst=\"rect\">\n";
    slideXML += "      <a:avLst/>\n";
    slideXML += "     </a:prstGeom>\n";
    slideXML += "     <a:noFill/>\n";
    slideXML += "     <a:ln><a:noFill/></a:ln>\n";
    slideXML += "    </p:spPr>\n";
    slideXML += "   </p:pic>\n";

    slideXML += "  </p:spTree>\n";
    slideXML += " </p:cSld>\n";
    slideXML += " <p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr>\n";
    slideXML += "</p:sld>\n";

    this->slideCount++;
    writeStringToFile(slideXML, this->zip_dir + "ppt/slides/slide" + std::to_string(this->slideCount) + ".xml");

    std::vector<PowerPoint::relationship> rels;
    rels.push_back({"rId1", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout", "../slideLayouts/slideLayout2.xml"});
    rels.push_back({imageRID, "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image", "../media/cartographics.png"});
    writeStringToFile(makeRelationshipXML(&rels), this->zip_dir + "ppt/slides/_rels/slide" + std::to_string(this->slideCount) + ".xml.rels");

    system(("cp " + logo_file + " " + this->zip_dir + "ppt/media/cartographics.png").c_str());
}

void PowerPoint::addJlweLeaderboardSlide(std::vector<PowerPoint::teamScore> places) {

    std::string slideXML = "";
    std::string imageRID = "rId2";

    slideXML += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    slideXML += "<p:sld xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\">\n";
    slideXML += " <p:cSld>\n";
    slideXML += "  <p:spTree>\n";

    slideXML += "   <p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>\n";
    slideXML += "   <p:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"0\" cy=\"0\"/></a:xfrm></p:grpSpPr>\n";

    slideXML += "   <p:sp>\n";
    slideXML += "    <p:nvSpPr>\n";
    slideXML += "     <p:cNvPr id=\"2\" name=\"Title 1\"/>\n";
    slideXML += "     <p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr>\n";
    slideXML += "     <p:nvPr><p:ph type=\"title\"/></p:nvPr>\n";
    slideXML += "    </p:nvSpPr>\n";
    slideXML += "    <p:spPr>\n";
    slideXML += "      <a:xfrm>\n";
    slideXML += "       <a:off x=\"457200\" y=\"274638\"/>\n";
    slideXML += "       <a:ext cx=\"3034680\" cy=\"5962674\"/>\n";
    slideXML += "      </a:xfrm>\n";
    slideXML += "    </p:spPr>\n";
    slideXML += "    <p:txBody>\n";
    slideXML += "     <a:bodyPr/>\n";
    slideXML += "     <a:lstStyle/>\n";
    slideXML += "     <a:p>\n";
    slideXML += "      <a:r>\n";
    slideXML += "       <a:rPr lang=\"en-AU\" dirty=\"0\" smtClean=\"0\"/>\n";
    slideXML += "       <a:t>Final leaderboard</a:t>\n";
    slideXML += "      </a:r>\n";
    slideXML += "      <a:endParaRPr lang=\"en-AU\" dirty=\"0\"/>\n";
    slideXML += "     </a:p>\n";
    slideXML += "    </p:txBody>\n";
    slideXML += "   </p:sp>\n";

    slideXML += "   <p:pic>\n";
    slideXML += "    <p:nvPicPr>\n";
    slideXML += "     <p:cNvPr id=\"1026\" name=\"leaderboard_pic\"/>\n";
    slideXML += "     <p:cNvPicPr><a:picLocks noChangeAspect=\"1\" noChangeArrowheads=\"1\"/></p:cNvPicPr>\n";
    slideXML += "     <p:nvPr/>\n";
    slideXML += "    </p:nvPicPr>\n";
    slideXML += "    <p:blipFill>\n";
    slideXML += "     <a:blip r:embed=\"" + imageRID + "\"/>\n";
    slideXML += "     <a:srcRect/>\n";
    slideXML += "     <a:stretch><a:fillRect/></a:stretch>\n";
    slideXML += "    </p:blipFill>\n";
    slideXML += "    <p:spPr bwMode=\"auto\">\n";
    slideXML += "     <a:xfrm>\n";
    slideXML += "      <a:off x=\"4211960\" y=\"0\"/>\n";
    slideXML += "      <a:ext cx=\"3888432\" cy=\"6882811\"/>\n";
    slideXML += "     </a:xfrm>\n";
    slideXML += "     <a:prstGeom prst=\"rect\">\n";
    slideXML += "      <a:avLst/>\n";
    slideXML += "     </a:prstGeom>\n";
    slideXML += "     <a:noFill/>\n";
    slideXML += "     <a:ln><a:noFill/></a:ln>\n";
    slideXML += "    </p:spPr>\n";
    slideXML += "   </p:pic>\n";

    slideXML += "  </p:spTree>\n";
    slideXML += " </p:cSld>\n";
    slideXML += " <p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr>\n";
    slideXML += "</p:sld>\n";

    this->slideCount++;
    writeStringToFile(slideXML, this->zip_dir + "ppt/slides/slide" + std::to_string(this->slideCount) + ".xml");

    std::vector<PowerPoint::relationship> rels;
    rels.push_back({"rId1", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout", "../slideLayouts/slideLayout2.xml"});
    rels.push_back({imageRID, "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image", "../media/leaderboard.png"});
    writeStringToFile(makeRelationshipXML(&rels), this->zip_dir + "ppt/slides/_rels/slide" + std::to_string(this->slideCount) + ".xml.rels");

    // make leaderboard image
    writeStringToFile(makeSvgLeaderBoard(places), this->tmp_dir + "/leaderboard.svg");

    // SVG images aren't supported by some versions of powerpoint so we need to convert to a PNG image
    system(("convert " + this->tmp_dir + "/leaderboard.svg " + this->zip_dir + "ppt/media/leaderboard.png").c_str());
}

std::string PowerPoint::makeSvgLeaderBoard(std::vector<PowerPoint::teamScore> places) {
    std::string result = "";
    int rowHeight = 28;
    int totalHeight = rowHeight * (places.size() + 1);
    result += "<svg xmlns=\"http://www.w3.org/2000/svg\" height=\"" + std::to_string(totalHeight + 2) + "px\" width=\"402px\" >";
    result += "<style>\n";
    result += "text { font-size: 20px; font-family: sans-serif; fill: black; }\n";
    result += "line { stroke: black; stroke-width: 2px; }\n";
    result += "</style>\n";
    result += "<rect width=\"100%\" height=\"100%\" fill=\"white\" />";

    result += "<rect width=\"100%\" height=\"" + std::to_string(rowHeight + 1) + "\" fill=\"#4FAE27\" />";
    result += "<line x1=\"1\" y1=\"0\" x2=\"1\" y2=\"" + std::to_string(totalHeight + 2) + "\" />";
    result += "<line x1=\"61\" y1=\"0\" x2=\"61\" y2=\"" + std::to_string(totalHeight + 2) + "\" />";
    result += "<line x1=\"321\" y1=\"0\" x2=\"321\" y2=\"" + std::to_string(totalHeight + 2) + "\" />";
    result += "<line x1=\"401\" y1=\"0\" x2=\"401\" y2=\"" + std::to_string(totalHeight + 2) + "\" />";

    result += "<line x1=\"0\" y1=\"1\" x2=\"402\" y2=\"1\" />";
    result += "<line x1=\"0\" y1=\"" + std::to_string(rowHeight + 1) + "\" x2=\"402\" y2=\"" + std::to_string(rowHeight + 1) + "\" />";

    result += "<text x=\"31\" y=\"" + std::to_string(rowHeight - 4) + "\" dominant-baseline=\"middle\" text-anchor=\"middle\" >#</text>";
    result += "<text x=\"191\" y=\"" + std::to_string(rowHeight - 4) + "\" dominant-baseline=\"middle\" text-anchor=\"middle\" >Team Name</text>";
    result += "<text x=\"361\" y=\"" + std::to_string(rowHeight - 4) + "\" dominant-baseline=\"middle\" text-anchor=\"middle\" >Points</text>";

    for (int i = 0; i < places.size(); i++) {
        result += "<line x1=\"0\" y1=\"" + std::to_string((i + 2) * rowHeight + 1) + "\" x2=\"402\" y2=\"" + std::to_string((i + 2) * rowHeight + 1) + "\" />";
        if (places.at(i).score > -1000)
            result += "<text x=\"31\" y=\"" + std::to_string((i + 2) * rowHeight - 4) + "\" dominant-baseline=\"middle\" text-anchor=\"middle\" >" + std::to_string(places.at(i).position) + "</text>";
        result += "<text x=\"67\" y=\"" + std::to_string((i + 2) * rowHeight - 4) + "\" >" + Encoder::htmlEntityEncode(places.at(i).team_name.substr(0, 25)) + "</text>";
        result += "<text x=\"393\" y=\"" + std::to_string((i + 2) * rowHeight - 4) + "\" dominant-baseline=\"middle\" text-anchor=\"end\" >" + PowerPoint::scoreToString(places.at(i).score) + "</text>";
    }

    result += "</svg>";
    return result;
}

void PowerPoint::getListOfTeamScores(JlweCore *jlwe, std::vector<PowerPoint::teamScore> &places, std::vector<PowerPoint::teamScore> &disqualified) {
    sql::Statement *stmt = jlwe->getMysqlCon()->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT team_name, team_members, final_score FROM game_teams WHERE competing = 1 AND final_score IS NOT NULL ORDER BY final_score DESC, team_name;");
    int i = 1;
    int previous_score = 0;
    int previous_position = i;
    while (res->next()) {
        int score = res->getInt(3);
        int position = i;
        if (score == previous_score) {
            position = previous_position;
        } else {
            previous_score = score;
            previous_position = i;
        }

        PowerPoint::teamScore place = {res->getString(1).substr(0, 30), res->getString(2).substr(0, 200), score, position};
        if (score > -1000) {
            places.push_back(place);
        } else {
            disqualified.push_back(place);
        }

        i++;
    }
    delete res;
    delete stmt;
}
