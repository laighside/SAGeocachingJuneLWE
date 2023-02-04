/**
  @file    KmlPlacemark.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A class represent a placemark from a KML file - either a line or polygon
  Used for calculating if points are in polygons and the distances from lines

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef KMLPLACEMARK_H
#define KMLPLACEMARK_H
#include <string>
#include <vector>

#include "KmlCoordinateList.h"

#include "../ext/pugixml/pugixml.hpp"

class KmlPlacemark
{
public:
    KmlPlacemark();
    ~KmlPlacemark();

    /*!
     * \brief Constructs the placemark from the XML data
     *
     * \param xmlNode The XML data
     */
    KmlPlacemark(const pugi::xml_node &xmlNode);

    /*!
     * \brief Gets the name of the placemark as defined in the <name> element
     *
     * \return The name
     */
    std::string Name() const;

    /*!
     * \brief Calculates the bounding box of the placemark
     *
     * \return The bounding box
     */
    KmlCoordinateList::BoundingBox getBoundingBox() const;

    /*!
     * \brief Calculates if a point is inside any of the polygons in the placemark
     *
     * \param latitude The latitude of the point to test
     * \param longitude The longitude of the point to test
     * \return true if the point is inside a polygon, false otherwise
     */
    bool pointInPolygon(double latitude, double longitude) const;

    /*!
     * \brief Calculates the distance from a given point to the nearest line in the placemark
     *
     * \param latitude The latitude of the point to test
     * \param longitude The longitude of the point to test
     * \return The distance, in meters, from the nearest line
     */
    double distanceFromPoint(double latitude, double longitude) const;

private:
    std::string m_PlacemarkName;

    std::vector<KmlCoordinateList> m_outerRings;
    std::vector<KmlCoordinateList> m_innerRings;
    std::vector<KmlCoordinateList> m_lineStrings;

    /*!
     * \brief Calculates the number of times a given line crosses the lines of the polygon
     *
     * \param lineStart The start point of the line to test
     * \param lineEnd The end point of the line to test
     * \param polygon The list of coordinates that make a polygon
     * \param tolerance Tolerance when comparing floating point values
     * \return The number of intersections, or -1 if the line starts/end on the boundary of the polygon
     */
    static int countIntersections(KmlCoordinateList::Coordinate lineStart, KmlCoordinateList::Coordinate lineEnd, const std::vector<KmlCoordinateList> &polygon, double tolerance);

};

#endif // KMLPLACEMARK_H
