/**
  @file    geo_maths.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions for distance/bearing between coordinates, etc.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

// Points need to be of type google.maps.LatLng
// or this
var Point = (function() {
    function Point(lat, lon) {
        this.latitude = lat;
        this.longitude = lon;
    }

    Point.prototype.lat = function() {
        return this.latitude;
    };

    Point.prototype.lng = function() {
        return this.longitude;
    };

    return Point;
})();

/**
 * Calculates the distance between two sets of coordinates
 *
 * @param {Point} point1 One set of coordinates
 * @param {Point} point2 Another set of coordinates
 * @returns {Number} The distance in km
 */
function getDistance(point1, point2) {
    var p = 0.017453292519943295;    // Math.PI / 180
    var c = Math.cos;
    var a = 0.5 - c((point2.lat() - point1.lat()) * p)/2 +
        c(point1.lat() * p) * c(point2.lat() * p) *
        (1 - c((point2.lng() - point1.lng()) * p))/2;

    return 12742 * Math.asin(Math.sqrt(a)); // 2 * R; R = 6371 km
}

/**
 * Calculates the bearing between two sets of coordinates
 *
 * @param {Point} point1 One set of coordinates
 * @param {Point} point2 Another set of coordinates
 * @returns {Number} The bearing in degrees
 */
function getBearing(point1, point2) {
    var p = 0.017453292519943295;    // Math.PI / 180
    var c = Math.cos;

    var y = Math.sin((point2.lng() - point1.lng()) * p) * c(point2.lat() * p);
    var x = c(point1.lat() * p) * Math.sin(point2.lat() * p) -
        Math.sin(point1.lat() * p) * c(point2.lat() * p) * c((point2.lng() - point1.lng()) * p);
    var brng = Math.atan2(y, x) / p;
    if (brng < 0)
        brng = brng + 360;
    return brng;
}

/**
 * Calculates the projection of a given distance and bearing from a starting point
 *
 * @param {Point} point The start point
 * @param {Number} distance The distance in metres
 * @param {Number} bearing The bearing in degrees
 * @returns {Point} The projected point
 */
function waypointProjection(point, distance, bearing) {
    var p = 0.017453292519943295;    // Math.PI / 180
    var R = 6371000;                 // earth radius in metres

    var lat = Math.asin( Math.sin(point.lat() * p)*Math.cos(distance/R) +
                        Math.cos(point.lat() * p)*Math.sin(distance/R)*Math.cos(bearing * p) );
    var lon = (point.lng() * p) + Math.atan2(Math.sin(bearing * p)*Math.sin(distance/R)*Math.cos(point.lat() * p),
                             Math.cos(distance/R)-Math.sin(point.lat() * p)*Math.sin(lat));
    lon = lon / p;
    if (lon > 180)
        lon = lon - 360;
    if (lon < -180)
        lon = lon + 360;
    return {lat: lat / p, lon: lon};
}

/**
 * Rounds a number to the nearest integer towards zero
 *
 * @param {Number} val The number to round
 * @returns {Number} The rounded number
 */
function roundDown(val) {
    if (val < 0) {
        return Math.ceil(val);
    }
    return Math.floor(val);
}

/**
 * Converts a lat/lon pair to a string in decimal minutes format
 *
 * @param {Number} lat The latitude
 * @param {Number} lon The longitude
 * @returns {String} The coordinates in decimal minutes format
 */
function makeCoordString(lat, lon){
    var result = '';
    if (lat > 0) {
        result = result + 'N';
    } else {
        result = result + 'S';
    }
    var intPart = roundDown(lat);
    var minutes = Math.abs(lat - intPart) * 60;
    var minutesStr = minutes.toFixed(3);
    while (minutesStr.length < 6) {
        minutesStr = '0' + minutesStr;
    }
    result = result + Math.abs(intPart) + '° ' + minutesStr + ' ';

    if (lon < 0) {
        result = result + 'W';
    } else {
        result = result + 'E';
    }
    intPart = roundDown(lon);
    minutes = Math.abs(lon - intPart) * 60;
    minutesStr = minutes.toFixed(3);
    while (minutesStr.length < 6) {
        minutesStr = '0' + minutesStr;
    }
    result = result + Math.abs(intPart) + '° ' + minutesStr;

    return result;
}
