/**
  @file    KmlCoordinateList.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A class represent a list of coordinates from a KML file (a single line or polygon)
  Used for calculating if points are in polygons and the distances from lines

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "KmlCoordinateList.h"

#include <cmath>

KmlCoordinateList::KmlCoordinateList() {
    // do nothing
}

KmlCoordinateList::~KmlCoordinateList() {
    // do nothing
}

KmlCoordinateList::KmlCoordinateList(const pugi::xml_node &xmlNode, bool isLinearRing) {
    this->m_isLinearRing = isLinearRing;
    std::string coordinateListString = xmlNode.child_value("coordinates");

    size_t spaceIndex = coordinateListString.find(' ');
    while (spaceIndex != std::string::npos) {
        std::string coordinateString = coordinateListString.substr(0, spaceIndex);
        this->parseCoordinate(coordinateString);
        coordinateListString = coordinateListString.substr(spaceIndex + 1);
        spaceIndex = coordinateListString.find(' ');
    }
    this->parseCoordinate(coordinateListString);
}

void KmlCoordinateList::parseCoordinate(const std::string &coordinateString) {
    size_t commaIndex = coordinateString.find(',');
    if (commaIndex == std::string::npos)
        return;

    size_t comma2Index = coordinateString.find(',', commaIndex + 1);

    std::string lonStr = coordinateString.substr(0, commaIndex);
    std::string latStr = "";
    std::string eleStr = "";
    if (comma2Index == std::string::npos) {
        latStr = coordinateString.substr(commaIndex + 1);
    } else {
        latStr = coordinateString.substr(commaIndex + 1, comma2Index - commaIndex - 1);
        eleStr = coordinateString.substr(comma2Index + 1);
    }

    double lat = 0.0;
    try {
        lat = std::stod(latStr);
    } catch (...) {}
    double lon = 0.0;
    try {
        lon = std::stod(lonStr);
    } catch (...) {}
    double ele = 0.0;
    try {
        ele = std::stod(eleStr);
    } catch (...) {}

    this->m_Coordinates.push_back({lat, lon, ele});

}

KmlCoordinateList::BoundingBox KmlCoordinateList::getBoundingBox() const {
    BoundingBox result;
    result.maxLat = -1000.0;
    result.minLat = 1000.0;
    result.maxLon = -1000.0;
    result.minLon = 1000.0;

    for (unsigned int i = 0; i < this->m_Coordinates.size(); i++) {
        Coordinate p = this->m_Coordinates.at(i);
        if (result.maxLat < p.latitude)
            result.maxLat = p.latitude;
        if (result.minLat > p.latitude)
            result.minLat = p.latitude;
        if (result.maxLon < p.longitude)
            result.maxLon = p.longitude;
        if (result.minLon > p.longitude)
            result.minLon = p.longitude;
    }
    return result;
}

// point in polygon functions
KmlCoordinateList::IntersectType KmlCoordinateList::linesCross(KmlCoordinateList::Coordinate line1Start, KmlCoordinateList::Coordinate line1End, KmlCoordinateList::Coordinate line2Start, KmlCoordinateList::Coordinate line2End, double tolerance, KmlCoordinateList::d_values *d_values){
    // Convert vector 1 to a line (line 1) of infinite length.
    // We want the line in linear equation standard form: A*x + B*y + C = 0
    // See: http://en.wikipedia.org/wiki/Linear_equation
    double a1 = line1Start.latitude - line1End.latitude; //v1y2 - v1y1;
    double b1 = line1End.longitude - line1Start.longitude; //v1x1 - v1x2;
    double c1 = (line1Start.longitude * line1End.latitude) - (line1End.longitude * line1Start.latitude);

    // Catch case if start and end points are the same
    if (fabs(a1) <= tolerance && fabs(b1) <= tolerance) return NOT_LINE;

    // Every point (x,y), that solves the equation above, is on the line,
    // every point that does not solve it, is not. The equation will have a
    // positive result if it is on one side of the line and a negative one
    // if is on the other side of it. We insert (x1,y1) and (x2,y2) of vector
    // 2 into the equation above.
    double d1_line2 = (a1 * line2Start.longitude) + (b1 * line2Start.latitude) + c1; //(a1 * v2x1) + (b1 * v2y1) + c1;
    double d2_line2 = (a1 * line2End.longitude) + (b1 * line2End.latitude) + c1; //(a1 * v2x2) + (b1 * v2y2) + c1;

    // If d1 and d2 both have the same sign, they are both on the same side
    // of our line 1 and in that case no intersection is possible. Careful,
    // 0 is a special case, that's why we don't test ">=" and "<=",
    // but "<" and ">".
    if (d1_line2 > tolerance && d2_line2 > tolerance) return NO;
    if (d1_line2 < (-1 * tolerance) && d2_line2 < (-1 * tolerance)) return NO;

    // The fact that vector 2 intersected the infinite line 1 above doesn't
    // mean it also intersects the vector 1. Vector 1 is only a subset of that
    // infinite line 1, so it may have intersected that line before the vector
    // started or after it ended. To know for sure, we have to repeat the
    // the same test the other way round. We start by calculating the
    // infinite line 2 in linear equation standard form.
    double a2 = line2Start.latitude - line2End.latitude; //v2y2 - v2y1;
    double b2 = line2End.longitude - line2Start.longitude; //v2x1 - v2x2;
    double c2 = (line2Start.longitude * line2End.latitude) - (line2End.longitude * line2Start.latitude); //(v2x2 * v2y1) - (v2x1 * v2y2);

    // Catch case if start and end points are the same
    if (fabs(a2) <= tolerance && fabs(b2) <= tolerance) return NOT_LINE;

    // Calculate d1 and d2 again, this time using points of vector 1.
    double d1_line1 = (a2 * line1Start.longitude) + (b2 * line1Start.latitude) + c2; //(a2 * v1x1) + (b2 * v1y1) + c2;
    double d2_line1 = (a2 * line1End.longitude) + (b2 * line1End.latitude) + c2; //(a2 * v1x2) + (b2 * v1y2) + c2;

    // Again, if both have the same sign (and neither one is 0),
    // no intersection is possible.
    if (d1_line1 > tolerance && d2_line1 > tolerance) return NO;
    if (d1_line1 < (-1 * tolerance) && d2_line1 < (-1 * tolerance)) return NO;

    if (d_values) {
        d_values->d_line1Start = d1_line1;
        d_values->d_line1End = d2_line1;
        d_values->d_line2Start = d1_line2;
        d_values->d_line2End = d2_line2;
    }

    // If we get here, only two possibilities are left. Either the two
    // vectors intersect in exactly one point or they are collinear, which
    // means they intersect in any number of points from zero to infinite.
    // (this is the same as all d values == zero)
    // there is also the special case here when the lines are on the same 'infinite line' but are otherwise completely separate
    //if (fabs((a1 * b2) - (a2 * b1)) <= tolerance) return COLLINEAR;
    if (fabs(d1_line1) <= tolerance && fabs(d2_line1) <= tolerance && fabs(d1_line2) <= tolerance && fabs(d2_line2) <= tolerance) return COLLINEAR;

    // If they are not collinear, they must intersect in exactly one point.
    // If all the d values are non-zero, the lines intersect
    if (fabs(d1_line2) > tolerance && fabs(d2_line2) > tolerance && fabs(d1_line1) > tolerance && fabs(d2_line1) > tolerance) return YES;

    // If some of the d values are zero, but others not, one of the lines starts/finishes on the other
    // 3 zero values shouldn't be possible (there will always be 0,1,2 or 4)
    // If there are 2 zeros, the lines share a start/end point
    if (fabs(d1_line2) <= tolerance || fabs(d2_line2) <= tolerance){ //line 2 starts/ends on line 1
        if (fabs(d1_line1) <= tolerance || fabs(d2_line1) <= tolerance) return COMMON_POINT;
        return LINE2_STARTS_ON_1;
    }
    if (fabs(d1_line1) <= tolerance || fabs(d2_line1) <= tolerance){ //line 1 starts/ends on line 2
        if (fabs(d1_line2) <= tolerance || fabs(d2_line2) <= tolerance) return COMMON_POINT;
        return LINE1_STARTS_ON_2;
    }

    // shouldn't get here
    throw std::invalid_argument("Something went wrong while calculating if lines cross");
}

int KmlCoordinateList::countIntersections(KmlCoordinateList::Coordinate lineStart, KmlCoordinateList::Coordinate lineEnd, double tolerance) const {
    int intersections = 0;
    for (unsigned int i = 0; i < this->m_Coordinates.size() - 1; i++){
        d_values d_values;
        IntersectType intersect = linesCross(lineStart, lineEnd, this->m_Coordinates.at(i), this->m_Coordinates.at(i + 1), tolerance, &d_values);
        if (intersect == COMMON_POINT || intersect == LINE1_STARTS_ON_2)
            // this means the line starts/finishes on the edge of the polygon
            // do nothing?
            return -1;
        if (intersect == COLLINEAR) {
            // this means part of the polygon edge runs along on top of the line
            // ignore since the 'crosses' in this case are dealt with by LINE2_STARTS_ON_1
        }
        if (intersect == LINE2_STARTS_ON_1){
            // there are two possibilities here,
            // either the line intersects at a vertex of the polygon
            // or the polygon 'touches' the line but doesn't cross

            // so we need to know the d-values for each of the points that are at
            // the other ends of the polygon segments that start/finish on the line
            // if both have the same sign, then the polygon 'touches' the line but dosn't cross
            // if that have different signs, then the line intersects at a vertex of the polygon

            // so only count positive signed ones, hence:
            // both negative touch = +0
            // both positive touch = +2
            // intersects at a vertice = +1
            if (fabs(d_values.d_line2Start) <= tolerance) {
                if (d_values.d_line2End > tolerance)
                    intersections++;
            } else if (fabs(d_values.d_line2End) <= tolerance) {
                if (d_values.d_line2Start > tolerance)
                    intersections++;
            }
        }
        if (intersect == YES)
            intersections++;
    }
    return intersections;
}

// point distance from line functions
double KmlCoordinateList::distanceFromPointToLine(KmlCoordinateList::Coordinate point, KmlCoordinateList::Coordinate lineStart, KmlCoordinateList::Coordinate lineEnd) {
    // inital bearing from line start to line end
    double toRadians = M_PI / 180;
    double y = sin(lineEnd.longitude * toRadians - lineStart.longitude * toRadians) * cos(lineEnd.latitude * toRadians);
    double x = cos(lineStart.latitude * toRadians) * sin(lineEnd.latitude * toRadians) - sin(lineStart.latitude * toRadians) * cos(lineEnd.latitude * toRadians) * cos(lineEnd.longitude * toRadians - lineStart.longitude * toRadians);
    double brngStartToEnd = atan2(y, x);

    // inital bearing from line start to point
    y = sin(point.longitude * toRadians - lineStart.longitude * toRadians) * cos(point.latitude * toRadians);
    x = cos(lineStart.latitude * toRadians) * sin(point.latitude * toRadians) - sin(lineStart.latitude * toRadians) * cos(point.latitude * toRadians) * cos(point.longitude * toRadians - lineStart.longitude * toRadians);
    double brngStartToPoint = atan2(y, x);

    // is point in the same direction as the line?
    double deltaAngle = M_PI - fabs(fabs(brngStartToEnd - brngStartToPoint) - M_PI);
    bool sameSide = deltaAngle < (M_PI / 2);

    // angular distance between line start and point
    double deltaLat = (point.latitude - lineStart.latitude) * toRadians;
    double deltaLon = (point.longitude - lineStart.longitude) * toRadians;
    double a = sin(deltaLat/2) * sin(deltaLat/2) +
            cos(lineStart.latitude * toRadians) * cos(point.latitude * toRadians) *
            sin(deltaLon/2) * sin(deltaLon/2);
    double distStartToPoint = 2 * atan2(sqrt(a), sqrt(1-a));

    // if not in same direction, the shortest distance is to the start point
    if (sameSide == false)
        return fabs(distStartToPoint) * 6371000; //earth radius

    // distance between point an infinte line
    double angDistance = asin(sin(distStartToPoint) * sin(brngStartToPoint - brngStartToEnd));

    // angular length of line
    deltaLat = (lineEnd.latitude - lineStart.latitude) * toRadians;
    deltaLon = (lineEnd.longitude - lineStart.longitude) * toRadians;
    a = sin(deltaLat/2) * sin(deltaLat/2) +
            cos(lineStart.latitude * toRadians) * cos(lineEnd.latitude * toRadians) *
            sin(deltaLon/2) * sin(deltaLon/2);
    double lengthOfLine = 2 * atan2(sqrt(a), sqrt(1-a));

    // location of shortest distance intersect point (from the line start point)
    double trackDistance = acos(cos(distStartToPoint) / cos(angDistance));

    // if interesect is before the end of the line then return the shortest distance
    if (trackDistance < lengthOfLine)
        return fabs(angDistance) * 6371000; //earth radius

    // if not, the shortest distance to the line segment is to the end point
    // angular distance between line end and point
    deltaLat = (point.latitude - lineEnd.latitude) * toRadians;
    deltaLon = (point.longitude - lineEnd.longitude) * toRadians;
    a = sin(deltaLat/2) * sin(deltaLat/2) +
            cos(lineEnd.latitude * toRadians) * cos(point.latitude * toRadians) *
            sin(deltaLon/2) * sin(deltaLon/2);
    double distEndToPoint = 2 * atan2(sqrt(a), sqrt(1-a));

    return fabs(distEndToPoint) * 6371000; //earth radius
}

double KmlCoordinateList::distanceFromPoint(KmlCoordinateList::Coordinate point) const {
    double min_distance = 40000000.0; //start at max
    for (unsigned int i = 0; i < this->m_Coordinates.size() - 1; i++){
        double distance = distanceFromPointToLine(point, this->m_Coordinates.at(i), this->m_Coordinates.at(i + 1));
        if (distance < min_distance)
            min_distance = distance;
    }
    return min_distance;
}


