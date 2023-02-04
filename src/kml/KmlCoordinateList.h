/**
  @file    KmlCoordinateList.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A class represent a list of coordinates from a KML file (a single line or polygon)
  Used for calculating if points are in polygons and the distances from lines

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef KMLCOORDINATELIST_H
#define KMLCOORDINATELIST_H
#include <string>
#include <vector>

#include "../ext/pugixml/pugixml.hpp"

class KmlCoordinateList
{
public:
    KmlCoordinateList();
    ~KmlCoordinateList();

    /*! \struct Coordinate
     *  \brief Stores a geographic coordinate
     *
     *  Latitude, longitude, elevation
     */
    struct Coordinate {
        double latitude;
        double longitude;
        double elevation;
    };

    /*! \struct BoundingBox
     *  \brief Stores a rectangle of geographic coordinates
     *
     *  Max latitude, max longitude, min latitude, min longitude
     */
    struct BoundingBox {
        double maxLat;
        double minLat;
        double maxLon;
        double minLon;
    };

    /*!
     * \brief Constructs the coordinate list from the XML data
     *
     * \param xmlNode The XML data
     * \param isLinearRing Set to true if the coordinate list is a Linear Ring - ie. a closed loop (start point == end point)
     */
    KmlCoordinateList(const pugi::xml_node &xmlNode, bool isLinearRing);

    /*!
     * \brief Calculate the bounding box for all the coordiantes in the list
     *
     * \return The bounding box
     */
    BoundingBox getBoundingBox() const;

    /*!
     * \brief This function calculates how many times a given line crosses the line formed by the coordinate list
     *
     * This is used by the point in polygon algorithm
     *
     * \param lineStart The start point of the line
     * \param lineEnd The end point of the line
     * \param tolerance Tolerance used when checking if double values are equal
     * \return The number of intersections, or -1 if the line starts/ends on the polygon
     */
    int countIntersections(Coordinate lineStart, Coordinate lineEnd, double tolerance) const;

    /*!
     * \brief This function calculates the shortest distance from a given point to the line formed by the coordinate list
     *
     * \param point The point to calculate the distance from
     * \return The distance, in meters, from the nearest line
     */
    double distanceFromPoint(Coordinate point) const;

private:

    struct d_values {
        double d_line1Start;
        double d_line1End;
        double d_line2Start;
        double d_line2End;
    };

    enum IntersectType {
        NO, YES, LINE2_STARTS_ON_1, LINE1_STARTS_ON_2, COMMON_POINT, COLLINEAR, NOT_LINE
    };

    static IntersectType linesCross(Coordinate line1Start, Coordinate line1End, Coordinate line2Start, Coordinate line2End, double tolerance, d_values *d_values);

    static double distanceFromPointToLine(Coordinate point, Coordinate lineStart, Coordinate lineEnd);

    /*!
     * \brief This parses a coordinate and adds it to the list
     *
     * \param coordinateString The coordinate in format "longitude,latitude,elevation"
     */
    void parseCoordinate(const std::string &coordinateString);

    std::vector<Coordinate> m_Coordinates;
    bool m_isLinearRing;
};

#endif // KMLCOORDINATELIST_H
