/**
  @file    WriteDOCX.cpp
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
#include "WriteDOCX.h"
#include <filesystem>
#include <stdexcept>

#include "../core/Encoder.h"
#include "../core/JlweUtils.h"

WriteDOCX::WriteDOCX(const std::string &template_dir) {
    // make temp folder and filenames
    char dir_template[] = "/tmp/tmpdir.XXXXXX";
    char *dir_name = mkdtemp(dir_template);
    if (dir_name == nullptr)
        throw std::runtime_error("Unable to create temporary directory");
    this->tmp_dir = std::string(dir_name);
    this->zip_dir = this->tmp_dir + "/document/";

    // copy the template to the temp folder
    int cp_result = system(("cp -r " + template_dir + " " + this->zip_dir).c_str());
    if (cp_result)
        throw std::runtime_error("Copying template directory failed: " + template_dir);

    if (system(("mkdir " + this->zip_dir + "word").c_str()))
        throw std::runtime_error("Error while creating word directory");
    if (system(("mkdir " + this->zip_dir + "word/_rels").c_str()))
        throw std::runtime_error("Error while creating _rels directory");

    this->defaultContentTypes = {{"rels", "application/vnd.openxmlformats-package.relationships+xml"},
                                 {"xml", "application/xml"}};
    this->overrideContentTypes = {{"/docProps/core.xml", "application/vnd.openxmlformats-package.core-properties+xml"},
                                  {"/docProps/app.xml", "application/vnd.openxmlformats-officedocument.extended-properties+xml"},
                                  {"/word/document.xml", "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"}};
}

WriteDOCX::~WriteDOCX() {
    system(("rm -r " + this->tmp_dir + "/").c_str());
}

std::string WriteDOCX::saveDocxFile(const std::string &title, const std::string &creatorName) {

    std::vector<relationship> rels;
    for (unsigned int i = 0; i < this->media_files.size(); i++)
        rels.push_back({"rIdImg" + std::to_string(i + 1), "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image", "media/" + this->media_files.at(i)});
    writeStringToFile(makeRelationshipXML(&rels), this->zip_dir + "word/_rels/document.xml.rels");

    writeStringToFile(makeCoreXML(title, creatorName), this->zip_dir + "docProps/core.xml");
    writeStringToFile(makeContentTypesXML(&this->defaultContentTypes, &this->overrideContentTypes), this->zip_dir + "[Content_Types].xml");

    // zip the folder to make single file
    std::string filename = this->tmp_dir + "/document.zip";
    if (system(("cd " + this->zip_dir + " ; zip -q " + filename + " -r *").c_str()))
        throw std::runtime_error("Error while running zip compression");
    return filename;
}

std::string WriteDOCX::makeContentTypesXML(std::vector<content_type_default> *default_types, std::vector<content_type_override> *override_types) {
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

std::string WriteDOCX::makeRelationshipXML(std::vector<WriteDOCX::relationship> *relationships) {
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

std::string WriteDOCX::makeCoreXML(const std::string &title, const std::string &creatorName) {
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

std::string WriteDOCX::addMediaFile(const std::string &src_filename) {
    std::string media_dir = this->zip_dir + "word/media";
    if (!std::filesystem::is_directory(media_dir))
        std::filesystem::create_directory(media_dir);

    std::string extension = src_filename.substr(src_filename.find_last_of('.'));
    std::string filename = "image" + std::to_string(this->media_files.size() + 1) + extension;
    std::filesystem::copy_file(src_filename, media_dir + "/" + filename);

    this->media_files.push_back(filename);

    return "rIdImg" + std::to_string(this->media_files.size());
}

void WriteDOCX::addMimeType(const std::string &extension, const std::string &mimeType) {
    this->defaultContentTypes.push_back({extension, mimeType});
}

std::string WriteDOCX::makeSectionPropertiesXML(int pageWidth, int pageHeight, bool isLandscape, int columns, int margins, int header, int footer) {
    std::string result = "";
    result += "<w:sectPr>\n";
    result += "  <w:pgSz w:w=\"" + std::to_string(pageWidth) + "\" w:h=\"" + std::to_string(pageHeight) + "\" w:orient=\"" + (isLandscape ? "landscape" : "portrait") + "\"/>\n";
    result += "  <w:pgMar w:top=\"" + std::to_string(margins) + "\" w:right=\"" + std::to_string(margins) + "\" w:bottom=\"" + std::to_string(margins) + "\" w:left=\"" + std::to_string(margins) + "\" w:header=\"" + std::to_string(header) + "\" w:footer=\"" + std::to_string(footer) + "\" w:gutter=\"0\"/>\n";
    if (columns > 1)
        result += "<w:cols w:num=\"" + std::to_string(columns) + "\" w:space=\"" + std::to_string(margins) + "\"/>\n";
    result += "</w:sectPr>\n";
    return result;
}

std::string WriteDOCX::makeRunPropertiesXML(bool bold, bool italic, bool setFont, double fontSize) {
    std::string result = "";
    result += "<w:rPr>\n";
    if (bold)
        result += "<w:b/>\n";
    if (italic)
        result += "<w:i/>\n";
    if (setFont)
        result += "<w:sz w:val=\"" + std::to_string(static_cast<int>(fontSize * 2)) + "\"/>\n";
    result += "</w:rPr>\n";
    return result;
}

std::string WriteDOCX::makeTextXML(const std::string &text) {
    return "<w:t>" + Encoder::htmlEntityEncode(text) + "</w:t>\n";
}

std::string WriteDOCX::makeImageXML(const std::string &imageRId, const std::string &id, const std::string &name, int width, int height) {
    std::string result = "";
    result += "<w:drawing>\n";
    result += "<wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\">\n";
    result += "<wp:extent cx=\"" + std::to_string(width) + "\" cy=\"" + std::to_string(height) + "\"/>\n";
    result += "<wp:docPr id=\"" + Encoder::htmlAttributeEncode(id) + "\" name=\"" + Encoder::htmlAttributeEncode(name) + "\"/>\n";
    result += "<wp:cNvGraphicFramePr><a:graphicFrameLocks xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" noChangeAspect=\"1\"/></wp:cNvGraphicFramePr>\n";
    result += "<a:graphic xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">\n";
    result += "<a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">\n";
    result += "<pic:pic xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">\n";
    result += "<pic:nvPicPr><pic:cNvPr id=\"" + Encoder::htmlAttributeEncode(id) + "\" name=\"" + Encoder::htmlAttributeEncode(name) + "\"/><pic:cNvPicPr/></pic:nvPicPr>\n";
    result += "<pic:blipFill>\n";
    result += "<a:blip r:embed=\"" + imageRId + "\">\n";
    result += "</a:blip>\n";
    result += "<a:stretch><a:fillRect/></a:stretch>\n";
    result += "</pic:blipFill>\n";
    result += "<pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + std::to_string(width) + "\" cy=\"" + std::to_string(height) + "\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr>\n";
    result += "</pic:pic>\n";
    result += "</a:graphicData>\n";
    result += "</a:graphic>\n";
    result += "</wp:inline>\n";
    result += "</w:drawing>\n";
    return result;
}

void WriteDOCX::makeDocumentFromXML(const std::string &bodyXML) {
    std::string documentXML = "";
    documentXML += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    documentXML += "<w:document xmlns:wpc=\"http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas\" xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:wp14=\"http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing\" xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\" xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\" xmlns:wpg=\"http://schemas.microsoft.com/office/word/2010/wordprocessingGroup\" xmlns:wpi=\"http://schemas.microsoft.com/office/word/2010/wordprocessingInk\" xmlns:wne=\"http://schemas.microsoft.com/office/word/2006/wordml\" xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\" mc:Ignorable=\"w14 wp14\">\n";
    documentXML += bodyXML;
    documentXML += "</w:document>\n";

    writeStringToFile(documentXML, this->zip_dir + "word/document.xml");
}

void WriteDOCX::writeStringToFile(const std::string &data, const std::string &filename) {
    FILE * file = fopen(filename.c_str(), "w");
    if (!file)
        throw std::runtime_error("Unable to create file: " + filename);
    fwrite(data.c_str(), 1, data.size(), file);
    fclose(file);
}
