/**
  @file    PowerPoint.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Class for creating a PPTX (PowerPoint) file containing the presentation for Sunday night
  The PPT file is built by modifying a template PPT file, which is located at PPT_TEMPLATE_DIR
  This would probably be better if it was done using a 3rd party PPT library?

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef POWERPOINT_H
#define POWERPOINT_H

#include <string>
#include <vector>

#include "../core/JlweCore.h"

class PowerPoint
{
public:

    /*!
     * \brief PowerPoint Constructor.
     *
     * This creates a temporary directory with a copy of the template file
     */
    PowerPoint();

    /*!
     * \brief PowerPoint Destructor.
     *
     * This deletes the temporary directory
     */
    ~PowerPoint();

    /*! \struct teamScore
     *  \brief Stores the final score of one team
     */
    struct teamScore {
        std::string team_name;
        std::string team_members;
        int score;  // = actual score * 10 (this allows for .5 scores)
        int position;
    };

    /*! \struct bestCache
     *  \brief Stores the winner of a best cache prize
     */
    struct bestCache {
        std::string category;
        std::string winning_cache;
    };

    /*!
     * \brief Converts a score to a string to be shown on the presentation
     *
     * Essentially score / 10 converted to string with decimal place if required
     * And -100 becomes "DSQ"
     *
     * \param score The score
     * \return The score as a string
     */
    static std::string scoreToString(int score);

    /*!
     * \brief Get the list of scores for each team from the database, in order of placing
     *
     * \param jlwe JlweCore object
     * \param places A vector to place the team scores in (in order of placing)
     * \param disqualified A vector to place the disqualified teams in (if there are any)
     */
    static void getListOfTeamScores(JlweCore *jlwe, std::vector<teamScore> &places, std::vector<teamScore> &disqualified);

    /*!
     * \brief Creates a generic slide with title and text content
     *
     * Slide will have no animations
     *
     * \param title The slide title
     * \param text The slide content
     */
    void addGenericSlide(const std::string &title, const std::string &text);

    /*!
     * \brief Creates a title slide with "Welcome..."
     *
     * \param logo_file The full filename of the JLWE logo image
     * \param year The event year to show on the slide
     * \param town The town name to show on the slide
     */
    void addJlweTitleSlide(const std::string &logo_file, const std::string &year, const std::string &town);

    /*!
     * \brief Creates a slide with a list of teams, scores and rank
     *
     * Works best with 5 or less teams per slide. Animations will make each rank appear one by one.
     *
     * \param places The list of teams to show on the slide
     */
    void addJlwePlacesSlide(std::vector<teamScore> places);

    /*!
     * \brief Creates a slide with a list of disqualified teams
     *
     * Animations will make each team appear one by one.
     *
     * \param places The list of teams to show on the slide
     */
    void addJlweDisqualifiedSlide(std::vector<teamScore> places);

    /*!
     * \brief Creates a slide with the scores and rank for just one team
     *
     * Used for the winner and runner up slides
     *
     * \param team The team, score and rank to show on the slide
     * \param title The slide title
     * \param sub_title The slide sub title
     * \param timing Animation data
     */
    void addJlweSinglePlaceSlide(teamScore team, const std::string &title, const std::string &sub_title, std::vector<int> timing);

    /*!
     * \brief Creates a slide with a list of winning best caches
     *
     * Animations will make each winner appear one by one.
     *
     * \param caches The list of the categories and winners
     */
    void addJlweBestCachesSlide(std::vector<bestCache> caches);

    /*!
     * \brief Creates a slide for the rising star award winner
     *
     * Just a slide with the sponsor logo on it
     *
     * \param logo_file The full filename of the sponsor logo image
     */
    void addJlweRisingStarSlide(const std::string &logo_file);

    /*!
     * \brief Creates a slide for the "other prizes"
     *
     * Best costume, Best table, Best hats, etc.
     */
    void addJlweOtherPrizesSlide();

    /*!
     * \brief Creates a slide with the full final leader-board
     *
     * A slide with an image that has the list of all teams, scores, places
     * This is usually the final slide of the presentation
     *
     * \param places The list of all teams
     */
    void addJlweLeaderboardSlide(std::vector<teamScore> places);

    /*!
     * \brief Finalises the PPT file, then does the ZIP compression
     *
     * The PPT file is save in the temporary directory and the filename is returned
     * This file will be deleted when the PowerPoint object is destroyed
     *
     * \return The filename of the complete PPT file
     */
    std::string savePowerPointFile();

private:

    struct relationship {
        std::string id;
        std::string type;
        std::string target;
    };

    int slideCount;
    std::string tmp_dir; // the temporary directory used for storing parts of the PPT file during construction
    std::string zip_dir; // the directory that is the base of the ZIP archive (will be inside the temporary directory)

    std::string makeRelationshipXML(std::vector<relationship> *relationships);
    void writeStandardSlideRelationship(int slideNumber);
    std::string makeContentTypesXML();
    std::string makePresentationXML();
    std::string makeSlideXML(std::string content, std::vector<int> timing = {});
    std::string makeTimingXML(std::vector<int> timing);
    std::string makeSlideTitleXML(std::string title);
    std::string makeSlideContentTextboxXML(std::string paragraphs, bool autoFit = false);
    std::string makeSlideLineXML(std::string line, std::string sub_line);
    std::string makeTeamScoreLineXML(std::string team, std::string members, std::string position, std::string points);

    void addSlideFromContentTextBox(std::string title, std::string contentTextBoxXML, std::vector<int> timing = {}, bool autoFit = false);

    /*!
     * \brief Makes an SVG image with the full final leader-board
     *
     * \param places The list of all teams
     * \return The SVG data
     */
    std::string makeSvgLeaderBoard(std::vector<teamScore> places);

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

#endif // POWERPOINT_H
