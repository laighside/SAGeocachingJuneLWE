/**
  @file    edit_caches.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This is all the JS for the page /cgi-bin/gpx_builder/edit_caches.cgi

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

// details of the current cache
var latitude = 0;
var longitude = 0;
var zone_bonus_points = 0;
var osm_distance = 0;
var user_submitted_distance = -1;

// map objects
var map_marker;
var map;

// This is true if the cache has come from the public form (this is called a user submitted cache, and needs to be reviewed by an admin)
// If false, the cache has been entered into this form by an admin
var is_user_cache = false;

// This is called once the page has loaded
function onPageLoad() {
    map = loadMap('map_area', kml_file_current, 'satellite');
    map_marker = addMarker(map);

    if (map_type == 'google') {
        setTimeout(function(){setMapZoom(map, 14);}, 200);
        cacheLoad()
    }
    if (map_type == 'leaflet') {
        setTimeout(function(){setMapZoom(map, 14);}, 100);
        setTimeout(function(){
            cacheLoad()
        }, 500);
    }
}

// This loads the details of a cache to start with
// Is called after the map has loaded
function cacheLoad() {
    // If a user submitted cache ID is provided then load it
    if (typeof id_number !== 'undefined') {
        if (id_number > 0) {
            is_user_cache = true;
            getUserCache(id_number);
            return;
        }
    }

    // If a cache number is provided then load that specific cache
    if (typeof cache_number !== 'undefined') {
        if (cache_number > 0) {
            document.getElementById("cache_number").value = cache_number;
            is_user_cache = false;
            getCacheDetails();
            return;
        }
    }

    // No cache number provided so just load the default one
    is_user_cache = false;
    getCacheDetails();
}

// This is called when the selected cache number changes
// If we are currently reviewing a user submitted cache, this changes the number of that cache
// Otherwise load the new cache number that is selected
function getCacheDetails(){
    var cache_number = document.getElementById("cache_number").value;
    if (is_user_cache) {
        downloadUrl('/cgi-bin/gpx_builder/get_cache.cgi?cache_number=' + cache_number.toString(), null, checkCacheExists);
    } else {
        downloadUrl('/cgi-bin/gpx_builder/get_cache.cgi?cache_number=' + cache_number.toString(), null, loadCacheJson);
    }
};

// This gets the details of a user submitted cache
function getUserCache(id) {
    downloadUrl('/cgi-bin/gpx_builder/get_cache.cgi?user_cache_id_number=' + id.toString(), null, loadCacheJson);
};

// This takes the cache data from a HTTP response and places it in the form (used to load existing data)
function loadCacheJson(data, responseCode) {

    if (responseCode === 200){
        var jsonObj = JSON.parse(data);

        if (jsonObj.error == null) {
            if (is_user_cache) {
                document.getElementById("cache_number").value = jsonObj.cache_number;
                document.getElementById("hidder_name").innerHTML = jsonObj.team_name;
                document.getElementById("hidder_phone").innerHTML = jsonObj.phone_number;
                getCacheDetails();
            }
            document.getElementById("cache_name").value = jsonObj.cache_name;
            document.getElementById("team_name").value = jsonObj.team_name;
            document.getElementById("coordinates").value = makeCoordString(jsonObj.latitude, jsonObj.longitude);
            document.getElementById("public_hint").value = jsonObj.public_hint;
            document.getElementById("detailed_hint").value = jsonObj.detailed_hint;
            document.getElementById("camo").checked = jsonObj.camo;
            document.getElementById("permanent").checked = jsonObj.permanent;
            document.getElementById("private").checked = jsonObj.private_property;
            document.getElementById("page_note").innerHTML = '';
            if (is_user_cache) {
                user_submitted_distance = jsonObj.actual_distance;
                document.getElementById("user_distance").innerText = (user_submitted_distance < 0 ? "-" : user_submitted_distance.toString() + 'm');
            } else {
                if (jsonObj.actual_distance < 0){
                    document.getElementById("distance_correct_yes").checked = true;
                    document.getElementById("distance_correct_no").checked = false;
                    document.getElementById("actual_distance").value = 0;
                } else {
                    document.getElementById("distance_correct_yes").checked = false;
                    document.getElementById("distance_correct_no").checked = true;
                    document.getElementById("actual_distance").value = jsonObj.actual_distance;
                }
            }
            updateCoords(false);

        } else {
            document.getElementById("cache_name").value = '';
            document.getElementById("team_name").value = '';
            document.getElementById("coordinates").value = makeCoordString(0, 0);
            document.getElementById("public_hint").value = '';
            document.getElementById("detailed_hint").value = '';
            document.getElementById("camo").checked = false;
            document.getElementById("permanent").checked = false;
            document.getElementById("private").checked = false;
            if (jsonObj.error == 'cache not found') {
                document.getElementById("page_note").innerHTML = 'Error: ' + jsonObj.error + ' (this is normal when entering the cache for the first time)';
            } else {
                document.getElementById("page_note").innerHTML = 'Error: ' + jsonObj.error;
            }
            updateCoords(true);
        }
    }
}

// This takes the cache data from a HTTP response and uses it to check if the cache number already exists
// Only used for user submitted caches
function checkCacheExists(data, responseCode) {

    if (responseCode === 200) {
        var jsonObj = JSON.parse(data);
        document.getElementById("save_button").disabled = true;

        if (jsonObj.error == null) {
            // cache already exists
            document.getElementById("cache_number_note").innerHTML = '<span style=\"color: red;\">Error: Cache number already exists</span>';

        } else {
            document.getElementById("cache_number_note").innerHTML = '';
            if (jsonObj.cache_exists == false) {
                document.getElementById("page_note").innerHTML = '';
                document.getElementById("save_button").disabled = false;
            } else {
                document.getElementById("page_note").innerHTML = 'Error: ' + jsonObj.error;
            }
            updateCoords(true);
        }
    }
}

// This parses the coordinate string and updates the map marker
// And a request is sent to the server to get playing field and zone details about the new location
// clicked is set to true when the function is called by a click on the 'Verify' button
function updateCoords(clicked) {
    try {
        // Parse the coordinate string
        var position = new Coordinates(document.getElementById("coordinates").value);
        latitude = position.getLatitude();
        longitude = position.getLongitude();

        // Update the map marker location
        moveMarker(map, map_marker, latitude, longitude);

        // Make request to get zone info about the new location
        downloadUrl('/cgi-bin/get_coord_info.cgi?lat=' + latitude.toString() + '&lon=' + longitude.toString(), null,
            function(data, responseCode) {

            if (clicked) {
                document.getElementById("osm_distance").innerText = "-";
                if (is_user_cache) {

                } else {
                    document.getElementById("distance_block").style.display = "none";
                    document.getElementById("distance_correct_yes").checked = false;
                    document.getElementById("distance_correct_no").checked = false;
                }
            }
            zone_bonus_points = 0;
            osm_distance = 0;

            if (responseCode === 200) {
                var jsonObj = JSON.parse(data);

                if (jsonObj.in_playing_field != null) {
                    if (jsonObj.in_playing_field == true) {
                        var note = 'Cache is inside playing field.';
                        if (jsonObj.zone_name != null){
                            zone_bonus_points = jsonObj.zone_points;
                            note = note + '<br/>Cache is in bouns zone: ' + jsonObj.zone_name + ' (' + jsonObj.zone_points.toString() + ' points)';
                        }
                        if (jsonObj.from_osm_road != null){
                            osm_distance = jsonObj.from_osm_road;
                            //note = note + '<br/>Distance from nearest road (OSM data): ' + jsonObj.from_osm_road.toString() + 'm';
                            document.getElementById("osm_distance").innerText = jsonObj.from_osm_road.toString() + 'm';
                            if (!is_user_cache) {
                                document.getElementById("distance_block").style.display = "block";
                                distanceCorrectChanged();
                            }
                        }
                        document.getElementById("playing_field").innerHTML = note;
                    } else {
                        document.getElementById("playing_field").innerHTML = '<span style="color: red;">Warning: Cache is not in the playing field!</span>';
                        if (!is_user_cache)
                            document.getElementById("distance_block").style.display = "none";
                    }
                } else {
                    document.getElementById("playing_field").innerHTML = '<span style="color: red;">Error: ' + jsonObj.error + '</span>';
                }
            }
        });

    } catch (error) {
        alert('Invalid Coordinates: ' + error);
    }
}

// This is called when the save button is clicked
// It sends the cache data to the server
function setCacheDetails() {
    if (is_user_cache) {
        if (document.getElementById("walking_distance_osm").checked == false && document.getElementById("walking_distance_user").checked == false && document.getElementById("walking_distance_other").checked == false) {
            alert('Please select a walking distance');
            return;
        }
    }

    var jsonObj = {
        "cache_name":document.getElementById("cache_name").value,
        "team_name":document.getElementById("team_name").value,
        "latitude":latitude,
        "longitude":longitude,
        "public_hint":document.getElementById("public_hint").value,
        "detailed_hint":document.getElementById("detailed_hint").value,
        "camo":document.getElementById("camo").checked,
        "permanent":document.getElementById("permanent").checked,
        "private_property":document.getElementById("private").checked,
        "zone_bonus":zone_bonus_points,
        "osm_distance":osm_distance,
        "actual_distance":-1
    };

    if (is_user_cache) {
        jsonObj.id_number = id_number
        if (document.getElementById("walking_distance_user").checked == true)
            jsonObj.actual_distance = user_submitted_distance;
        if (document.getElementById("walking_distance_other").checked == true)
            jsonObj.actual_distance = Number(document.getElementById("other_distance").value);
    } else {
        if (document.getElementById("distance_correct_yes").checked == false){
            jsonObj.actual_distance = Number(document.getElementById("actual_distance").value);
        }
    }

    var cache_number = document.getElementById("cache_number").value;
    postUrl('/cgi-bin/gpx_builder/set_cache.cgi?cache_number=' + cache_number.toString(), JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, false, null, null);
            }, httpErrorResponseHandler);
        /*function(data, responseCode) {

            if (responseCode === 200){
                var jsonObj = JSON.parse(data);

                if (jsonObj.error == null){
                    document.getElementById("page_note").innerHTML = 'Success: ' + jsonObj.success;
                }else{
                    document.getElementById("page_note").innerHTML = 'Error: ' + jsonObj.error;
                }
            }
     });*/
}

// This is called when the "is this distance correct?" radiobuttons are changed
function distanceCorrectChanged(){
    if (document.getElementById("distance_correct_no").checked){
        document.getElementById("actual_distance_block").style.display = "block";
    } else {
        document.getElementById("actual_distance_block").style.display = "none";
    }
}

// This deletes a user submitted cache
function deleteUserCache() {
    if (typeof id_number !== 'undefined') {
        if (confirm("Are you sure you wish to delete this cache?") == true) {
            var jsonObj = {
                "reviewCacheId":id_number
            };
            postUrl('/cgi-bin/gpx_builder/delete_cache.cgi', JSON.stringify(jsonObj), null,
                function(data, responseCode) {
                    httpResponseHandler(data, responseCode, false, function() {window.location.href = '/cgi-bin/gpx_builder/gpx_builder.cgi';}, null);
                }, httpErrorResponseHandler);
                /*function(data, responseCode) {

                    if (responseCode === 200){
                        var jsonObj = JSON.parse(data);

                        if (jsonObj.error == null){
                            document.getElementById("page_note").innerHTML = 'Success: ' + jsonObj.success;
                            window.location.href = '/cgi-bin/gpx_builder/gpx_builder.cgi';
                        }else{
                            document.getElementById("page_note").innerHTML = 'Error: ' + jsonObj.error;
                        }
                    }
             });*/
        }
    }
}
