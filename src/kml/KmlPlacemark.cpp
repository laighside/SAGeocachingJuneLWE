/**
  @file    KmlPlacemark.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A class represent a placemark from a KML file - either a line or polygon
  Used for calculating if points are in polygons and the distances from lines

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "KmlPlacemark.h"

// Distance (in lat/lon degrees) between points/lines for them to be considered on top of each other
#define TOLERANCE    0.0000000001

KmlPlacemark::KmlPlacemark() {
    // do nothing
}

KmlPlacemark::~KmlPlacemark() {
    // do nothing
}

KmlPlacemark::KmlPlacemark(const pugi::xml_node &xmlNode) {
    this->m_PlacemarkName = xmlNode.child_value("name");

    for (pugi::xml_named_node_iterator line_string_it = xmlNode.children("LineString").begin(); line_string_it != xmlNode.children("LineString").end(); ++line_string_it) {
        this->m_lineStrings.push_back(KmlCoordinateList(*line_string_it, false));
    }

    pugi::xml_node polygonNode = xmlNode.child("Polygon");
    for (pugi::xml_named_node_iterator outer_boundary_it = polygonNode.children("outerBoundaryIs").begin(); outer_boundary_it != polygonNode.children("outerBoundaryIs").end(); ++outer_boundary_it) {
        this->m_outerRings.push_back(KmlCoordinateList(outer_boundary_it->child("LinearRing"), true));
    }
    for (pugi::xml_named_node_iterator inner_boundary_it = polygonNode.children("innerBoundaryIs").begin(); inner_boundary_it != polygonNode.children("innerBoundaryIs").end(); ++inner_boundary_it) {
        this->m_innerRings.push_back(KmlCoordinateList(inner_boundary_it->child("LinearRing"), true));
    }
}

std::string KmlPlacemark::Name() const {
    return this->m_PlacemarkName;
}

KmlCoordinateList::BoundingBox KmlPlacemark::getBoundingBox() const {
    KmlCoordinateList::BoundingBox result;
    result.maxLat = -1000.0;
    result.minLat = 1000.0;
    result.maxLon = -1000.0;
    result.minLon = 1000.0;

    for (unsigned int i = 0; i < this->m_lineStrings.size(); i++) {
        KmlCoordinateList::BoundingBox innerBB = this->m_lineStrings.at(i).getBoundingBox();
        if (result.maxLat < innerBB.maxLat)
            result.maxLat = innerBB.maxLat;
        if (result.minLat > innerBB.minLat)
            result.minLat = innerBB.minLat;
        if (result.maxLon < innerBB.maxLon)
            result.maxLon = innerBB.maxLon;
        if (result.minLon > innerBB.minLon)
            result.minLon = innerBB.minLon;
    }
    for (unsigned int i = 0; i < this->m_outerRings.size(); i++) {
        KmlCoordinateList::BoundingBox innerBB = this->m_outerRings.at(i).getBoundingBox();
        if (result.maxLat < innerBB.maxLat)
            result.maxLat = innerBB.maxLat;
        if (result.minLat > innerBB.minLat)
            result.minLat = innerBB.minLat;
        if (result.maxLon < innerBB.maxLon)
            result.maxLon = innerBB.maxLon;
        if (result.minLon > innerBB.minLon)
            result.minLon = innerBB.minLon;
    }
    for (unsigned int i = 0; i < this->m_innerRings.size(); i++) {
        KmlCoordinateList::BoundingBox innerBB = this->m_innerRings.at(i).getBoundingBox();
        if (result.maxLat < innerBB.maxLat)
            result.maxLat = innerBB.maxLat;
        if (result.minLat > innerBB.minLat)
            result.minLat = innerBB.minLat;
        if (result.maxLon < innerBB.maxLon)
            result.maxLon = innerBB.maxLon;
        if (result.minLon > innerBB.minLon)
            result.minLon = innerBB.minLon;
    }
    return result;
}

bool KmlPlacemark::pointInPolygon(double latitude, double longitude) const {
    KmlCoordinateList::BoundingBox bbox = this->getBoundingBox();
    KmlCoordinateList::Coordinate testPoint = {latitude, longitude, 0.0};
    KmlCoordinateList::Coordinate rayStart = testPoint;
    rayStart.longitude = bbox.minLon - 0.1;

    int intersections = this->countIntersections(rayStart, testPoint, this->m_lineStrings, TOLERANCE);
    if (intersections == -1) // special case where the point is on the edge of the polygon
        return true;
    int sectionCount = this->countIntersections(rayStart, testPoint, this->m_outerRings, TOLERANCE);
    if (sectionCount == -1) // special case where the point is on the edge of the polygon
        return true;
    intersections += sectionCount;
    sectionCount = this->countIntersections(rayStart, testPoint, this->m_innerRings, TOLERANCE);
    if (sectionCount == -1) // special case where the point is on the edge of the polygon
        return true;
    intersections += sectionCount;

    if (intersections & 0x1) {
        return true;
    } else {
        return false;
    }
}

int KmlPlacemark::countIntersections(KmlCoordinateList::Coordinate lineStart, KmlCoordinateList::Coordinate lineEnd, const std::vector<KmlCoordinateList> &polygon, double tolerance) {
    int intersections = 0;
    for (unsigned int i = 0; i < polygon.size(); i++) {
        int sectionCount = polygon.at(i).countIntersections(lineStart, lineEnd, tolerance);
        if (sectionCount == -1) // special case where the point is on the edge of the polygon
            return sectionCount;
        intersections += sectionCount;
    }
    return intersections;
}


double KmlPlacemark::distanceFromPoint(double latitude, double longitude) const {
    double min_distance = 40000000.0; // start at max
    for (unsigned int i = 0; i < this->m_lineStrings.size(); i++){
        double distance = this->m_lineStrings.at(i).distanceFromPoint({latitude, longitude, 0.0});
        if (distance < min_distance)
            min_distance = distance;
    }
    for (unsigned int i = 0; i < this->m_outerRings.size(); i++){
        double distance = this->m_outerRings.at(i).distanceFromPoint({latitude, longitude, 0.0});
        if (distance < min_distance)
            min_distance = distance;
    }
    for (unsigned int i = 0; i < this->m_innerRings.size(); i++){
        double distance = this->m_innerRings.at(i).distanceFromPoint({latitude, longitude, 0.0});
        if (distance < min_distance)
            min_distance = distance;
    }

    return min_distance;
}



