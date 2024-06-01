/**
  @file    PointCalculator.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Class that calculates the points for each cache and team

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "PointCalculator.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>

#include "../ext/nlohmann/json.hpp"

PointCalculator::PointCalculator(JlweCore *jlwe, int number_game_caches) {
    this->m_number_game_caches = number_game_caches;
    this->m_jlwe = jlwe;

    sql::Statement *stmt;
    sql::ResultSet *res;
    stmt = jlwe->getMysqlCon()->createStatement();
    res = stmt->executeQuery("SELECT id, name, hide_or_find, config FROM game_find_points_trads WHERE enabled != 0;");
    while (res->next()) {
        this->trad_points.push_back({res->getInt(1), res->getString(2), res->getString(3), res->getString(4), {}});
    }
    delete res;
    delete stmt;

    stmt = jlwe->getMysqlCon()->createStatement();
    res = stmt->executeQuery("SELECT id, short_name, long_name, point_value FROM game_find_points_extras WHERE enabled > 0;");
    while (res->next()) {
        this->extras_items.push_back({res->getInt(1), res->getString(2), res->getString(3), res->getInt(4)});
    }
    delete res;
    delete stmt;

    stmt = jlwe->getMysqlCon()->createStatement();
    res = stmt->executeQuery("SELECT cache_handout.cache_number, cache_handout.team_id, caches.cache_number, IF(cache_handout.owner_name = '', 0, 1), caches.zone_bonus, caches.osm_distance, caches.actual_distance, caches.camo, cache_handout.returned, caches.latitude, caches.longitude, caches.cache_name FROM caches RIGHT OUTER JOIN cache_handout ON caches.cache_number=cache_handout.cache_number ORDER BY cache_handout.cache_number;");
    while (res->next()) {
        if (res->isNull(1) || res->isNull(2)) // This means cache is in GPX list but not handout table. This shouldn't happen.
            continue;

        Cache c;
        c.cache_number = res->getInt(1);
        c.team_id = res->getInt(2);
        c.has_coordinates = !(res->isNull(3));
        c.handout = (res->getInt(4) > 0);
        c.zone_points = res->isNull(5) ? 0 : res->getInt(5);
        if (res->isNull(6) || res->isNull(7)) {
            c.walking_distance = 0;
        } else {
            c.walking_distance = res->getInt(7);
            if (c.walking_distance < 0)
                c.walking_distance = res->getInt(6);
        }
        c.creative = (!res->isNull(8)) && (res->getInt(8) > 0);
        c.returned = (!res->isNull(9)) && (res->getInt(9) > 0);
        c.latitude = res->getDouble(10);
        c.longitude = res->getDouble(11);
        c.cache_name = res->getString(12);
        c.total_hide_points = 0;
        c.total_find_points = 0;
        this->caches.push_back(c);
    }
    delete res;
    delete stmt;

    if (static_cast<int>(this->caches.size()) !=  number_game_caches)
        throw std::runtime_error("number_game_caches (" + std::to_string(number_game_caches) + ") does not match size of cache list (" + std::to_string(this->caches.size()) + ")");

    this->calculatePointsForEachPointSource();
    this->calculateTotalHideFindPoints();
}

PointCalculator::~PointCalculator() {
    // do nothing
}

std::vector<PointCalculator::Cache> * PointCalculator::getCacheList() {
    return &this->caches;
}

std::vector<PointCalculator::CachePoints> * PointCalculator::getPointSourceList() {
    return &this->trad_points;
}

std::vector<PointCalculator::ExtraItem> * PointCalculator::getExtrasItemsList() {
    return &this->extras_items;
}

std::vector<PointCalculator::Cache> PointCalculator::getCachesForTeam(int teamId) {
    std::vector<Cache> list;
    for (unsigned int j = 0; j < this->caches.size(); j++) {
        if (this->caches.at(j).team_id == teamId)
            list.push_back(this->caches.at(j));
    }
    return list;
}

int PointCalculator::getTeamHideScore(int teamId) {
    std::vector<Cache> team_caches = this->getCachesForTeam(teamId);
    std::sort(team_caches.begin(), team_caches.end(),
              [](const Cache & a, const Cache & b) -> bool {
        return a.total_hide_points > b.total_hide_points;
    });

    int hide_score = 0;
    if (team_caches.size() >= 1)
        hide_score += team_caches.at(0).total_hide_points;
    if (team_caches.size() >= 2)
        hide_score += team_caches.at(1).total_hide_points;

    return hide_score;
}

int PointCalculator::getCachesNotReturned(int teamId) {
    int result = 0;
    for (unsigned int j = 0; j < this->caches.size(); j++) {
        if (this->caches.at(j).team_id == teamId)
            if (this->caches.at(j).returned == false)
                result++;
    }
    return result;
}

std::vector<int> PointCalculator::getTeamTradFindList(int teamId) {
    std::vector<int> result(static_cast<size_t>(this->m_number_game_caches), 0);

    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;
    prep_stmt = this->m_jlwe->getMysqlCon()->prepareStatement("SELECT trad_cache_number,find_value FROM game_find_list WHERE trad_cache_number >= 0 AND team_id = ?;");
    prep_stmt->setInt(1, teamId);
    res = prep_stmt->executeQuery();
    while (res->next()) {
        int cache_number = res->getInt(1);
        if (cache_number > 0 && cache_number <= this->m_number_game_caches)
            result[cache_number - 1] = res->getInt(2);
    }
    delete res;
    delete prep_stmt;

    return result;
}

int PointCalculator::getTotalTradFindScore(const std::vector<int> &find_list) {
    int total = 0;
    for (unsigned int i = 0; i < find_list.size(); i++) {
        if (find_list.at(i))
            if (i < this->caches.size())
                total += this->caches.at(i).total_find_points;
    }
    return total;
}

std::vector<PointCalculator::ExtrasFind> PointCalculator::getTeamExtrasFindList(int teamId) {
    std::vector<ExtrasFind> result;

    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;
    prep_stmt = this->m_jlwe->getMysqlCon()->prepareStatement("SELECT team_id,extras_id_number,find_value FROM game_find_list WHERE extras_id_number IS NOT NULL AND team_id = ?;");
    prep_stmt->setInt(1, teamId);
    res = prep_stmt->executeQuery();
    while (res->next()) {
        result.push_back({res->getInt(1), res->getInt(2), res->getInt(3)});
    }
    delete res;
    delete prep_stmt;

    return result;
}

int PointCalculator::getTotalExtrasFindScore(const std::vector<PointCalculator::ExtrasFind> &find_list) {
    int total = 0;
    for (unsigned int i = 0; i < this->extras_items.size(); i++) {
        for (unsigned int j = 0; j < find_list.size(); j++) {
            if (this->extras_items.at(i).id == find_list.at(j).id) {
                total += (this->extras_items.at(i).points_value * find_list.at(j).value);
            }
        }
    }
    return total;
}

int PointCalculator::getMinutesLate(const std::vector<PointCalculator::ExtrasFind> &find_list) {
    for (unsigned int i = 0; i < find_list.size(); i++)
        if (find_list.at(i).id == -1)
            return find_list.at(i).value;
    return 0;
}

void PointCalculator::calculatePointsForEachPointSource() {

    for (unsigned int i = 0; i < this->trad_points.size(); i++) {
        this->trad_points.at(i).points_list.clear();
        // default 0 points
        for (unsigned int j = 0; j < this->caches.size(); j++)
            this->trad_points.at(i).points_list.push_back(0);

        nlohmann::json configJson;
        if (trad_points.at(i).configJson.size())
            configJson = nlohmann::json::parse(trad_points.at(i).configJson);

        if (trad_points.at(i).id == 1) { // 1 point per cache
            for (unsigned int j = 0; j < this->caches.size(); j++)
                if (this->caches.at(j).has_coordinates) // only give points to caches in GPX file
                    this->trad_points.at(i).points_list[j] = 1;

            if (configJson.is_array()) {
                for (nlohmann::json::iterator it = configJson.begin(); it != configJson.end(); ++it) {
                    nlohmann::json jsonObject = *it;
                    int cache_number = jsonObject["cache"];

                    if (cache_number > 0 && cache_number <= this->trad_points.at(i).points_list.size())
                        this->trad_points.at(i).points_list[cache_number - 1] = jsonObject["points"];
                }
            }

        } else if (trad_points.at(i).id == 2) { // walking points

            int distance_per_point = configJson["distance"];
            int max_points = configJson["max_points"];

            for (unsigned int j = 0; j < this->caches.size(); j++) {
                int cache_number = this->caches.at(j).cache_number;

                int walking_points = this->caches.at(j).walking_distance / distance_per_point;
                if (walking_points > max_points)
                    walking_points = max_points;

                if (cache_number > 0 && cache_number <= this->trad_points.at(i).points_list.size())
                    this->trad_points.at(i).points_list[cache_number - 1] = walking_points;
            }

        } else if (trad_points.at(i).id == 3) { // zone points
            for (unsigned int j = 0; j < this->caches.size(); j++) {
                int cache_number = this->caches.at(j).cache_number;

                if (cache_number > 0 && cache_number <= this->trad_points.at(i).points_list.size())
                    this->trad_points.at(i).points_list[cache_number - 1] = this->caches.at(j).zone_points;
            }

        } else if (trad_points.at(i).id == 4) { // creative points

            int points = configJson["points"];

            for (unsigned int j = 0; j < this->caches.size(); j++) {
                int cache_number = this->caches.at(j).cache_number;

                if (cache_number > 0 && cache_number <= this->trad_points.at(i).points_list.size())
                    this->trad_points.at(i).points_list[cache_number - 1] = this->caches.at(j).creative ? points : 0;
            }

        } else if (trad_points.at(i).id == 5) { // cache spacing points

            int distance_per_point = configJson["distance"];
            int max_points = configJson["max_points"];

            for (unsigned int j = 0; j < this->caches.size(); j++) {

                // skip caches without coordinates
                if (!this->caches.at(j).has_coordinates)
                    continue;

                int cache_number = this->caches.at(j).cache_number;
                double shortest_distance = 1e9;

                for (unsigned int k = 0; k < this->caches.size(); k++) {
                    // skip caches without coordinates
                    if (!this->caches.at(k).has_coordinates)
                        continue;
                    // skip comparing to the same cache
                    if (j == k)
                        continue;

                    // if the caches are owned by the same team
                    if (this->caches.at(j).team_id == this->caches.at(k).team_id) {

                        // check the distance between them
                        double distance = this->getDistanceBetweenCaches(this->caches.at(j), this->caches.at(k));
                        if (distance < shortest_distance)
                            shortest_distance = distance;
                    }
                }

                int spacing_points = static_cast<int>(shortest_distance) / distance_per_point;
                if (spacing_points > max_points)
                    spacing_points = max_points;

                if (cache_number > 0 && cache_number <= this->trad_points.at(i).points_list.size())
                    this->trad_points.at(i).points_list[cache_number - 1] = spacing_points;

            }

        }
    }
}

void PointCalculator::calculateTotalHideFindPoints() {
    for (unsigned int j = 0; j < this->caches.size(); j++) {
        int cache_number = this->caches.at(j).cache_number;
        int total_hide = 0;
        int total_find = 0;

        for (unsigned int i = 0; i < this->trad_points.size(); i++) {
            if (this->trad_points.at(i).hide_or_find == "H") {
                total_hide += this->trad_points.at(i).points_list.at(cache_number - 1);
            }
            if (this->trad_points.at(i).hide_or_find == "F") {
                total_find += this->trad_points.at(i).points_list.at(cache_number - 1);
            }
        }

        this->caches[j].total_hide_points = total_hide;
        this->caches[j].total_find_points = total_find;
    }
}

double PointCalculator::getDistanceBetweenCaches(const PointCalculator::Cache &c1, const PointCalculator::Cache &c2) {
    const double earth_radius = 6371e3; // metres
    double lat1_rad = c1.latitude * M_PI / 180;
    double lat2_rad = c2.latitude * M_PI / 180;
    double delta_lat = (c2.latitude - c1.latitude) * M_PI / 180;
    double delta_lon = (c2.longitude - c1.longitude) * M_PI / 180;

    double a = sin(delta_lat/2) * sin(delta_lat/2) +
              cos(lat1_rad) * cos(lat2_rad) *
              sin(delta_lon/2) * sin(delta_lon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));

    return earth_radius * c; // in metres
}
