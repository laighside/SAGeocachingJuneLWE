/**
  @file    gpx_map.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This displays a GPX file of caches on Google Maps or Leaflet
  This is only used by the GPX builder map page, not the public map

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

// First declaration for the infowindow to allow closing
var openedInfoWindow = null;

// This kind of works like a class (or object with functions)
var GPXfile = (function() {

    /**
     * Constructor
     *
     * @param {String} gpx_url The URL to fetch the GPX file from
     * @param {Object} map The Google/Leaflet map object
     * @param {String} map_type The type of map (either 'google' or 'leaflet')
     */
    function GPXfile(gpx_url, map, map_type) {
        this.map = map;

        // Download the GPX file
        downloadUrl(gpx_url, 'true', function(data, responseCode) {
            var caches = data.documentElement.getElementsByTagName("wpt");
            for (var i=0; i<caches.length; i++) {
                var point = {
                    lat:parseFloat(caches[i].getAttribute("lat")),
                    lon:parseFloat(caches[i].getAttribute("lon"))
                };
                var code, name, owner, type;
                for (var j=0; j<caches[i].childNodes.length; j++) {
                    if (caches[i].childNodes[j].nodeName == 'name'){
                        code = caches[i].childNodes[j].textContent;
                    };
                    if (caches[i].childNodes[j].nodeName == 'groundspeak:cache'){
                        for (var k=0; k<caches[i].childNodes[j].childNodes.length; k++) {
                            if (caches[i].childNodes[j].childNodes[k].nodeName == 'groundspeak:name'){
                                name = caches[i].childNodes[j].childNodes[k].textContent;
                            };
                            if (caches[i].childNodes[j].childNodes[k].nodeName == 'groundspeak:placed_by'){
                                owner = caches[i].childNodes[j].childNodes[k].textContent;
                            };
                            if (caches[i].childNodes[j].childNodes[k].nodeName == 'groundspeak:type'){
                                type = caches[i].childNodes[j].childNodes[k].textContent;
                            };
                        };
                    };
                };

                var text = '<p>Cache: ' + code + ' - ' + name + '<br/>By: ' + owner + '</p><p><div id="dist"></div></p>';
                var pin = 't';
                //if (owner == 'JLWE'){
                //    pin = 't_blue';
                //};
                if (type == 'Earthcache' || type == 'Virtual'){
                    pin = 'v';
                };
                if (type == 'Unknown Cache' || type == 'Multi'){
                    pin = 'u';
                };
                var tempMarker = new createMarkerIcon(point,text,pin,map, map_type);
            };
        });

    }

    function createMarkerIcon(point, text, pin, map, map_type) {  // From GCA
        // Define the icon image to display
        var iconimage = "/img/gmap_" + pin + ".png";

        if (map_type === 'google') {
            // Create the image
            var image  = new google.maps.MarkerImage(iconimage);
            // Create the marker at point on map with icon image
            var marker = new google.maps.Marker({
                position: new google.maps.LatLng(point.lat, point.lon),
                map: map,
                icon: image
            });
            // Info (popup) window with cache details
            var infowindow = new google.maps.InfoWindow({
                content: text
            });
            // On click if there's an open window close it, open the clicked one, assign it so it can be closed next click
            google.maps.event.addListener(marker, 'click', function() {
                if (openedInfoWindow != null) {
                    openedInfoWindow.close();
                };
                infowindow.open(map,marker);
                openedInfoWindow = infowindow;
                google.maps.event.addListener(infowindow, 'closeclick', function() {
                    openedInfoWindow = null;
                });
            });
            // Send the marker details back to add to the array
            return marker;
        }
        if (map_type === 'leaflet') {
            // Create the icon
            var customIcon = L.Icon.extend({
                options: {
                    iconUrl: iconimage,
                    iconAnchor:   [10, 34],
                    popupAnchor:  [0, -34]
                }
            });

            var icon_image = new customIcon();
            // Create the marker at point on map with icon image
            var map_marker_l = L.marker(new L.latLng(point.lat, point.lon), {icon: icon_image});
            map_marker_l.bindPopup(text);
            // Add marker to map
            map_marker_l.addTo(map);
            return map_marker_l;

        }
        return null;
    }

    /**
     * This returns the info window the user has opened
     *
     * @returns {Object} The info window object for the info window currently opened, or null if there is no info window open
     */
    GPXfile.prototype.openedInfoWindow = function() {
        return this.openedInfoWindow;
    }

    return GPXfile;

})();
