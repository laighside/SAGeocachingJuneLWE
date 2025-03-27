/**
  @file    WriteCacheListDOCX.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Class for creating a DOCX (MS Word) file containing the list of caches
  The DOCX file is built by modifying a template DOCX file, which is located in the "templates" directory
  The templates directory is set by the config file "ooxmlTemplatePath" setting

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef WRITECACHELISTDOCX_H
#define WRITECACHELISTDOCX_H

#include <string>
#include <vector>

#include "../ooxml/WriteDOCX.h"
#include "../core/JlweCore.h"
#include "../core/KeyValueParser.h"

class WriteCacheListDOCX : public WriteDOCX
{
public:

    /*!
     * \brief WriteCacheListDOCX Constructor.
     *
     * \param template_dir Path to the template directory (this is copied to the tmp directory)
     */
    WriteCacheListDOCX(const std::string &template_dir);

    /*!
     * \brief WriteCacheListDOCX Destructor.
     */
    ~WriteCacheListDOCX();

    /*!
     * \brief Creates the document xml file with a list of caches (with names, coordinates, hints, etc.)
     *
     * \param jlwe pointer to JlweCore object
     * \param options URL query string parameters
     */
    void makeDocumentCacheList(JlweCore *jlwe, KeyValueParser *options);

    /*!
     * \brief Creates the document xml file with a list of who owns each cache
     *
     * \param jlwe pointer to JlweCore object
     * \param options URL query string parameters
     */
    void makeDocumentOwnerList(JlweCore *jlwe, KeyValueParser *options);

private:

    // which columns are enabled
    struct column_options_s {
        bool cache_code;
        bool cache_name;
        bool team_name;
        bool location;
        bool public_hint;
        bool detailed_hint;
        bool camo;
        bool perm;
        bool zone_bonus;
        bool walking;
    };

    struct owner_entry_s {
        int cache_number;
        std::string owner_name;
        int returned;
    };

    std::string insertBlankRow(std::string cache_code, column_options_s column_options, std::string paragraph_settings);
    std::string insertTableHeader(std::string page_size);
    std::string insertHeaderRow(column_options_s column_options, std::string paragraph_settings);
    std::string insertPageSplit(std::string page_size, column_options_s column_options, std::string paragraph_settings);

};

#endif // WRITECACHELISTDOCX_H
