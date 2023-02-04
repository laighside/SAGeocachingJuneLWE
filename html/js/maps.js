/**
  @file    maps.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Functions for using JS maps (Google maps or Leaflet)

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

/**
 * Loads the map
 *
 * @param {String} map_element The ID of the HTML element to put the map in
 * @param {String} kml_file URL to a KML file to display on the map (can be left blank)
 * @param {String} type The default type of map (eg. satellite)
 * @returns {Object} The map object
 */
function loadMap(map_element, kml_file, type) {
    if (map_type == 'google') {
        return loadGoogleMap(map_element, kml_file, type);
    }
    if (map_type == 'leaflet') {
        return loadLeafletMap(map_element, kml_file, type);
    }
    return null;
}

/**
 * Adds a marker to the map
 *
 * @param {Object} map The map object
 * @returns {Object} The marker object
 */
function addMarker(map) {
    if (map_type == 'google') {
        var map_marker_g = new google.maps.Marker({
            position: new google.maps.LatLng(0,0),
            map: map,
            icon: '/img/gmap_t.png'
        });
        return map_marker_g;
    }
    if (map_type == 'leaflet') {
        // Create the icon
        var customIcon = L.Icon.extend({
            options: {
                iconUrl: '/img/gmap_t.png',
                iconAnchor:   [10, 34],
                popupAnchor:  [0, -34]
            }
        });

        var image = new customIcon();
        // Create the marker at point on map with icon image
        var map_marker_l = L.marker(new L.latLng(0, 0), {icon: image});
        // Add marker to map
        map_marker_l.addTo(map);
        return map_marker_l;
    }
    return null;
}

/**
 * Moves a map marker to a new latitude/longitude
 *
 * @param {Object} map The map object
 * @param {Object} marker The marker object to move
 * @param {Number} latitude The new latitude to move to
 * @param {Number} longitude The new longitude to move to
 */
function moveMarker(map, marker, latitude, longitude) {
    if (map_type == 'google') {
        map.setCenter({lat: latitude, lng: longitude});
        marker.setPosition(new google.maps.LatLng(latitude,longitude));
    }
    if (map_type == 'leaflet') {
        map.setView([latitude, longitude], map.getZoom());
        marker.setLatLng(new L.LatLng(latitude,longitude));
    }
}

/**
 * Sets the zoom level of the map
 *
 * @param {Object} map The map object
 * @param {Integer} zoom The new zoom level
 */
function setMapZoom(map, zoom) {
    if (map_type == 'google') {
        map.setZoom(zoom);
    }
    if (map_type == 'leaflet') {
        map.setZoom(zoom);
    }
}

function invalidateMapSize(map) {
    if (map_type == 'google') {
        google.maps.event.trigger(map, "resize");
    }
    if (map_type == 'leaflet') {
        map.invalidateSize(false);
    }
}

/**
 * Loads a Google map
 *
 * @param {String} map_element The ID of the HTML element to put the map in
 * @param {String} kml_file URL to a KML file to display on the map (can be left blank)
 * @param {String} type The default type of map (eg. satellite)
 * @returns {Object} The map object
 */
function loadGoogleMap(map_element, kml_file, type) {
    var map = new google.maps.Map(document.getElementById(map_element), {
        center: {lat: 0, lng: 0},
        zoom: 12,
        mapTypeId: type
    });

    // add kml file
    if (kml_file && kml_file.length > 0){
        var kmlLayer = new google.maps.KmlLayer(kml_file, {
            map: map
        });
    };

    map.setCenter({lat: 0, lng: 0});
    map.setZoom(12);
    return map;
}

/**
 * Loads a Leaflet map
 *
 * @param {String} map_element The ID of the HTML element to put the map in
 * @param {String} kml_file URL to a KML file to display on the map (can be left blank)
 * @param {String} type The default type of map (eg. satellite) (unused)
 * @returns {Object} The map object
 */
function loadLeafletMap(map_element, kml_file, type) {
    //make layers
    var osm_layer = L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    });
    var ocm_layer = L.tileLayer('https://tile.thunderforest.com/cycle/{z}/{x}/{y}.png', {
        attribution: '&copy; OpenCycleMap, ' + 'Map data ' + '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    });

    // make map
    var map = L.map('map_area').setView([0, 0], 12);

    // make layer controls
    var baseMaps = {
        "OSM": osm_layer,
        "OCM": ocm_layer
    };
    L.control.layers(baseMaps).addTo(map);
    osm_layer.addTo(map);

    // add kml file
    if (kml_file && kml_file.length > 0){
        var customLayer = L.geoJson(null, {
            style: function(feature) {
                return { fillColor: "#FF000000" };
            }
        });
        var game_kml = omnivore.kml(kml_file, null, customLayer).addTo(map);
        game_kml.on('ready', function() {
            map.fitBounds(game_kml.getBounds())
        });
    };

    return map;
}
