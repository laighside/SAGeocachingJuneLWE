/**
  @file    PointCalculator.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Class that calculates the points for each cache and team

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef POINTCALCULATOR_H
#define POINTCALCULATOR_H

#include <string>
#include <vector>

#include "../core/JlweCore.h"

// Number of penalty points for each cache that isn't returned
#define CACHE_RETURN_PENALTY   -2

// Number of penalty points per minute late at the end of the game
#define MINUTES_LATE_PENALTY   -1

class PointCalculator
{
public:

    /*!
     * \brief This sets how the hide points are calculated
     * If true, the total for each cache is calculated, then the best two totals are used for the final hide score
     * If false, the points for best two caches from each category are calculated, then the totals from each category are added together to get the final hide score
     * This setting is only relevant if there is more then one "Hide only" points item
     */
    static inline bool use_totals_for_best_cache_calculation() {return false;}

    /*! \struct Cache
     *  \brief Stores a cache
     */
    struct Cache {
        int cache_number;
        int team_id;
        double latitude;
        double longitude;
        int zone_points;
        int walking_distance;
        bool creative;
        bool returned;
        bool has_coordinates;
        bool handout;

        int total_hide_points;
        int total_find_points;

        std::string cache_name;
    };

    /*! \struct CachePoints
     *  \brief Stores the points for the trad caches, for a single points source eg. walking points
     */
    struct CachePoints {
        int id;
        std::string item_name;
        std::string hide_or_find;
        std::string configJson;
        std::vector<int> points_list;
    };

    /*! \struct ExtraItem
     *  \brief Stores something a team gets points for that isn't a trad cache
     *         eg. Puzzles, black thunder, late return
     */
    struct ExtraItem {
        int id;
        std::string item_name_short;
        std::string item_name_long;
        bool single_find_only;
        char type;
        int points_value;
    };

    /*! \struct extraItem
     *  \brief Stores a single find on an ExtraItem for a given team
     *         eg. Team A found black thunder x2
     */
    struct ExtrasFind {
        int team_id;
        int id;
        int value;
    };

    /*! \struct BestScoreHides
     *  \brief Stores the list of caches that give the best score for a single point source (struct CachePoints)
     */
    struct BestScoreHides {
        int point_source_id;
        std::vector<int> cache_numbers;
    };

    /*!
     * \brief PointCalculator Constructor.
     *
     * \param jlwe JlweCore object (for mysql access)
     * \param number_game_caches The total number of caches in the game (from the website settings)
     */
    PointCalculator(JlweCore *jlwe, int number_game_caches);

    /*!
     * \brief PointCalculator Destructor.
     */
    ~PointCalculator();

    /*!
     * \brief Gets the list of all the caches
     *
     * \return The list of caches
     */
    std::vector<Cache> * getCacheList();

    /*!
     * \brief Gets the list of the sources of points for the traditional caches
     * eg. Walking points, zone points, etc.
     *
     * \return The list of items
     */
    std::vector<CachePoints> * getPointSourceList();

    /*!
     * \brief Gets the list of all the extra items that teams can get points on
     * eg. Puzzles, black thunder, etc.
     *
     * \return The list of items
     */
    std::vector<ExtraItem> * getExtrasItemsList();

    /*!
     * \brief Gets a list of caches hidden by the given team
     *
     * \param teamId The id number of the team
     * \return The list of caches hidden by the team
     */
    std::vector<Cache> getCachesForTeam(int teamId);

    /*!
     * \brief Gets the lists of caches that are the best two (highest score) for hide points in each category
     *
     * \param teamId The id number of the team
     * \return The lists of caches
     */
    std::vector<BestScoreHides> getBestScoreHidesForTeam(int teamId);

    /*!
     * \brief Gets the total number of hide points for a given team
     * This is the sum of hide points for the best two caches that the team hid
     *
     * \param best_caches_list A list of the caches that contribute to the total hide score
     * \return The total hide points
     */
    int getTeamHideScore(const std::vector<BestScoreHides> &best_caches_list);

    /*!
     * \brief Gets the total number of hide points for a given team
     * This is the sum of hide points for the best two caches that the team hid
     *
     * \param teamId The id number of the team
     * \return The total hide points
     */
    int getTeamHideScore(int teamId);

    /*!
     * \brief Gets the number of caches that a given team didn't return
     *
     * \param teamId The id number of the team
     * \return The number of caches not returned
     */
    int getCachesNotReturned(int teamId);

    /*!
     * \brief Gets a list of traditional caches found by the given team
     *
     * \param teamId The id number of the team
     * \return A list of 100 numbers (one for each cache), where 1 = found, 0 = not found by the team
     */
    std::vector<int> getTeamTradFindList(int teamId);

    /*!
     * \brief Gets the total number of traditional find points for a given team
     * This is the sum of find points for all the caches that the team found
     *
     * \param find_list The team's list of finds on traditional caches
     * \return The total find points
     */
    int getTotalTradFindScore(const std::vector<int> &find_list);

    /*!
     * \brief Gets a list of extras items found by the given team
     *
     * \param teamId The id number of the team
     * \return The list of extras finds (as id number and value) found by the team
     */
    std::vector<ExtrasFind> getTeamExtrasFindList(int teamId);

    /*!
     * \brief Gets the total number of find points on extras items for a given team
     * This is the sum of points for all the extras items that the team found
     *
     * \param find_list The team's list of finds on extras items
     * \return The total points
     */
    int getTotalExtrasFindScore(const std::vector<ExtrasFind> &find_list);

    /*!
     * \brief Gets the number of minutes late that a team is
     * This value is stored as an extras find with id = -1
     *
     * \param find_list The team's list of finds on extras items
     * \return The number of minutes
     */
    static int getMinutesLate(const std::vector<ExtrasFind> &find_list);

private:
    int m_number_game_caches;
    JlweCore *m_jlwe;

    std::vector<Cache> caches;
    std::vector<CachePoints> trad_points;
    std::vector<ExtraItem> extras_items;

    // Initialize points_list for each ExtraItem
    void calculatePointsForEachPointSource();
    // Initialize total_hide_points and total_find_points for each Cache
    void calculateTotalHideFindPoints();

    // Calculate the straight line distance (in metres) between two caches
    double getDistanceBetweenCaches(const Cache &c1, const Cache &c2);
};

#endif // POINTCALCULATOR_H
