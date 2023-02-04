/**
  @file    KmlFile.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A class to parse and store data from KML files
  Used for calculating if points are in polygons and the distances from lines

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef KMLFILE_H
#define KMLFILE_H

#include <string>
#include <vector>

#include "KmlPlacemark.h"

#include "../ext/pugixml/pugixml.hpp"

class KmlFile
{
public:

    KmlFile();
    ~KmlFile();

    /*!
     * \brief Reads and parses a KML file
     *
     * \param filename The KML file to read
     * \return true on success, false if there is an error
     */
    bool loadFile(const std::string &filename);

    /*!
     * \brief Gets a placemark from the file(s) at the given index
     *
     * \param index The index of the placemark to return
     * \return The placemark
     */
    KmlPlacemark Placemark(size_t index);

    /*!
     * \brief Get the number of placemarks in the file(s)
     *
     * \return The number of placemarks
     */
    size_t PlacemarkCount() const;

    /*!
     * \brief Gets the name of the KML file as defined in the <name> element
     *
     * \return The name
     */
    std::string Name() const;

    /*!
     * \brief Gets a description of the error that occurred during parsing the KML file
     *
     * \return A description of the error, or an empty string if no error occurred
     */
    std::string ParseError() const;

    /*!
     * \brief Calculates if a point is inside any of the polygons in the KML file
     *
     * \param latitude The latitude of the point to test
     * \param longitude The longitude of the point to test
     * \return true if the point is inside a polygon, false otherwise
     */
    bool pointInPolygon(double latitude, double longitude) const;

    /*!
     * \brief Calculates the distance from a given point to the nearest line in the KML file
     *
     * \param latitude The latitude of the point to test
     * \param longitude The longitude of the point to test
     * \return The distance, in meters, from the nearest line
     */
    double distanceFromPoint(double latitude, double longitude) const;

private:
    std::string m_KmlName;
    std::vector<KmlPlacemark> m_Placemarks;

    std::string m_ParseError;

    void loadFolder(const pugi::xml_node &xmlNode);

};

#endif // KMLFILE_H
