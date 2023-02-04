/**
  @file    KmlFile.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A class to parse and store data from KML files
  Used for calculating if points are in polygons and the distances from lines

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "KmlFile.h"

KmlFile::KmlFile() {
    // do nothing
}

KmlFile::~KmlFile() {
    // do nothing
}

bool KmlFile::loadFile(const std::string &filename) {
    pugi::xml_document doc;
    pugi::xml_parse_result parse_result = doc.load_file(filename.c_str());
    if (!parse_result) {
        this->m_ParseError = parse_result.description();
        return false;
    }

    pugi::xml_node kmlDoc = doc.child("kml").child("Document");
    this->m_KmlName = kmlDoc.child_value("name");

    this->loadFolder(kmlDoc);

    return true;
}

void KmlFile::loadFolder(const pugi::xml_node &xmlNode) {
    for (pugi::xml_named_node_iterator placemark_it = xmlNode.children("Placemark").begin(); placemark_it != xmlNode.children("Placemark").end(); ++placemark_it) {
        this->m_Placemarks.push_back(KmlPlacemark(*placemark_it));
    }
    for (pugi::xml_named_node_iterator folder_it = xmlNode.children("Folder").begin(); folder_it != xmlNode.children("Folder").end(); ++folder_it) {
        this->loadFolder(*folder_it);
    }
}

KmlPlacemark KmlFile::Placemark(size_t index) {
    return this->m_Placemarks.at(index);
}

size_t KmlFile::PlacemarkCount() const {
    return this->m_Placemarks.size();
}

std::string KmlFile::Name() const {
    return this->m_KmlName;
}

std::string KmlFile::ParseError() const {
    return this->m_ParseError;
}

bool KmlFile::pointInPolygon(double latitude, double longitude) const {
    for (unsigned int i = 0; i < this->m_Placemarks.size(); i++) {
        if (this->m_Placemarks.at(i).pointInPolygon(latitude, longitude))
            return true;
    }
    return false;
}

double KmlFile::distanceFromPoint(double latitude, double longitude) const {
    double min_distance = 40000000.0; // start at max
    for (unsigned int i = 0; i < this->m_Placemarks.size(); i++) {
        double distance = this->m_Placemarks.at(i).distanceFromPoint(latitude, longitude);
        if (distance < min_distance)
            min_distance = distance;
    }
    return min_distance;
}
