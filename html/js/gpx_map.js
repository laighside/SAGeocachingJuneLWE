/**
  @file    gpx_map.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This displays a GPX file of caches on Google Maps (or Leaflet?)
  This is only used by the GPX builder map page, not the public map

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

// This kind of works like a class (or object with functions)
var GPXfile = (function() {

    /**
     * Constructor
     *
     * @param {String} gpx_url The URL to fetch the GPX file from
     * @param {Object} map The Google Maps object
     * @param {Function} callback
     */
    function GPXfile(gpx_url, map, callback) {
        this.map = map;
        this.openedInfoWindow = null;   // First declaration for the infowindow to allow closing

        // Download the GPX file
        downloadUrl(gpx_url, 'true', function(data, responseCode) {
            var caches = data.documentElement.getElementsByTagName("wpt");
            for (var i=0; i<caches.length; i++) {
                var point = new google.maps.LatLng(parseFloat(caches[i].getAttribute("lat")),parseFloat(caches[i].getAttribute("lon")));
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
                var tempMarker = new createMarkerIcon(point,text,pin,map);
            };
        });

        function createMarkerIcon(point, text, pin, map) {  // From GCA
            // Define the icon image to display
            var iconimage = "/img/gmap_" + pin + ".png";
            // Create the image
            var image  = new google.maps.MarkerImage(iconimage);
            // Create the marker at point on map with icon image
            var marker = new google.maps.Marker({
                position: point,
                map: map,
                icon: image
            });
            // Info (popup) window with cache details
            var infowindow = new google.maps.InfoWindow({
                content: text
            });
            // On click if there's an open window close it, open the clicked one, assign it so it can be closed next click
            google.maps.event.addListener(marker, 'click', function() {
                if (this.openedInfoWindow != null) {
                    this.openedInfoWindow.close();
                };
                infowindow.open(map,marker);
                this.openedInfoWindow = infowindow;
                google.maps.event.addListener(infowindow, 'closeclick', function() {
                    this.openedInfoWindow = null;
                });
            });
            // Send the marker details back to add to the array
            return marker;
        }
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
