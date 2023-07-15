/**
  @file    scoring_points_setup.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This is all the JS for the points setup tab on the page /cgi-bin/scoring/scoring.cgi

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

/**
 * Called when the points setup tab is opened
 * This downloads the list of points and places them on the page
 */
function openPointsSetupTab() {
    showTableStatusRow("Loading...", "find_points_trads_table");
    showTableStatusRow("Loading...", "find_points_extras_table");
    showTableStatusRow("Loading...", "zones_table");

    downloadUrl('get_points.cgi', null,
            function(data, responseCode) {
                var jsonObj = JSON.parse(data);

                loadFindPointsTradsTable(jsonObj.find_points_trads);
                loadFindPointsExtrasTable(jsonObj.find_points_extras);
                loadZonesTable(jsonObj.zones, jsonObj.kml_file_list);

         }, function(statusCode, statusText) {
             if (statusText && typeof statusText == 'string') {
                 showTableStatusRow("HTTP error: " + statusText, "find_points_trads_table");
             } else {
                 showTableStatusRow("HTTP error: Unknown", "find_points_trads_table");
             }
         });
}

/**
 * This makes the HTML elements for the find points (trads) table
 *
 * @param {Object} jsonData The data for the table
 */
function loadFindPointsTradsTable(jsonData) {
    var table = document.getElementById("find_points_trads_table");

    while (table.rows.length > 1) {
        table.deleteRow(-1);
    }

    for (var i = 0; i < jsonData.length; i++) {
        var row = table.insertRow(-1);
        makeFindPointsTradsRowHtml(jsonData[i], row);
    }
}

/**
 * This makes the HTML elements for a single row of the find points (trads) table
 *
 * @param {Object} jsonData The data for the row
 * @param {Object} row The table row object to add the data to
 */
function makeFindPointsTradsRowHtml(jsonData, row) {
    row.id = "find_points_trads_row_" + jsonData.id.toString();

    var checkboxCell = document.createElement("td");
    checkboxCell.style.textAlign = "center";
    checkboxCell.appendChild(makeCheckboxElement("find_points_trads_checkbox_" + jsonData.id.toString(), "", jsonData.enabled, saveFindPointsTradsEnabled.bind(this, jsonData.id)));
    row.appendChild(checkboxCell);

    var nameCell = document.createElement("td");
    nameCell.innerText = jsonData.name;
    row.appendChild(nameCell);
}

/**
 * This makes the HTML elements for the find points (extras) table
 *
 * @param {Object} jsonData The data for the table
 */
function loadFindPointsExtrasTable(jsonData) {
    var table = document.getElementById("find_points_extras_table");

    while (table.rows.length > 1) {
        table.deleteRow(-1);
    }

    for (var i = 0; i < jsonData.length; i++) {
        var row = table.insertRow(-1);
        makeFindPointsExtrasRowHtml(jsonData[i], row);
    }
}

/**
 * This makes the HTML elements for a single row of the find points (extras) table
 *
 * @param {Object} jsonData The data for the row
 * @param {Object} row The table row object to add the data to
 */
function makeFindPointsExtrasRowHtml(jsonData, row) {
    row.id = "find_points_extras_row_" + jsonData.id.toString();

    var checkboxCell = document.createElement("td");
    checkboxCell.style.textAlign = "center";
    checkboxCell.appendChild(makeCheckboxElement("find_points_extras_checkbox_" + jsonData.id.toString(), "", jsonData.enabled, saveFindPointsExtraEnabled.bind(this, jsonData.id)));
    row.appendChild(checkboxCell);

    row.appendChild(makeEditableTextCell(jsonData.name, "find_points_extras_name_" + jsonData.id.toString(), saveFindPointsExtraName.bind(this, jsonData.id)));
    row.appendChild(makeEditableNumberCell(jsonData.point_value, "find_points_extras_value_" + jsonData.id.toString(), saveFindPointsExtraValue.bind(this, jsonData.id), true, "1"));

    var iconCell = document.createElement("td");
    iconCell.style.textAlign = "center";
    iconCell.appendChild(makeDeleteIcon(deleteFindPointsExtra.bind(this, jsonData.id)));
    row.appendChild(iconCell);
}

/**
 * This makes the HTML elements for the zones table
 *
 * @param {Object} jsonData The data for the table
 * @param {Array} kml_file_list The list of KML files that can be selected when creating a new zone
 */
function loadZonesTable(jsonData, kml_file_list) {
    var table = document.getElementById("zones_table");

    while (table.rows.length > 1) {
        table.deleteRow(-1);
    }

    for (var i = 0; i < jsonData.length; i++) {
        var row = table.insertRow(-1);
        makeZonesRowHtml(jsonData[i], row);
    }

    var newSelect = document.getElementById("add_zone_kmls");
    newSelect.innerHTML = "<option value=\"\"></select>";
    for (var j = 0; j < kml_file_list.length; j++) {
        var selectOption = document.createElement("option");
        selectOption.value = kml_file_list[j];
        selectOption.innerText = kml_file_list[j];
        newSelect.appendChild(selectOption);
    }

}

/**
 * This makes the HTML elements for a single row of the zones table
 *
 * @param {Object} jsonData The data for the row
 * @param {Object} row The table row object to add the data to
 */
function makeZonesRowHtml(jsonData, row) {
    row.id = "zones_row_" + jsonData.id.toString();

    var kmlCell = document.createElement("td");
    kmlCell.innerText = jsonData.kml_file;
    row.appendChild(kmlCell);

    row.appendChild(makeEditableTextCell(jsonData.name, "zones_name_" + jsonData.id.toString(), saveZoneName.bind(this, jsonData.id)));
    row.appendChild(makeEditableNumberCell(jsonData.points, "zones_value_" + jsonData.id.toString(), saveZonePoints.bind(this, jsonData.id), true, "1"));

    var deleteCell = document.createElement("td");
    var deleteButton = document.createElement("input");
    deleteButton.type = "button";
    deleteButton.value = "Delete";
    deleteButton.addEventListener('click', deleteZone.bind(this, jsonData.id));
    deleteCell.appendChild(deleteButton);
    row.appendChild(deleteCell);
}

/**
 * Saves the enabled status of a find points trads item, this sends the new status to the server
 *
 * @param {Number} id The ID number of the item
 */
function saveFindPointsTradsEnabled(id) {
    var newValue = document.getElementById("find_points_trads_checkbox_" + id.toString()).checked;

    var jsonObj = {
        type: "find_trads",
        id: id,
        enabled: newValue
    };
    postUrl('set_points.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, null, null);
         }, httpErrorResponseHandler);
}

/**
 * Saves the enabled status of a find points extra item, this sends the new status to the server
 *
 * @param {Number} id The ID number of the item
 */
function saveFindPointsExtraEnabled(id) {
    var newValue = document.getElementById("find_points_extras_checkbox_" + id.toString()).checked;

    var jsonObj = {
        type: "find_extras",
        id: id,
        enabled: newValue
    };
    postUrl('set_points.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, null, null);
         }, httpErrorResponseHandler);
}

/**
 * Saves a edited find points extra name, this sends the new name to the server
 *
 * @param {Number} id The ID number of the item being edited
 * @param {String} newValue The new name of the item
 * @param {Function} successCallback Function to call once the new value is successfully saved
 */
function saveFindPointsExtraName(id, newValue, successCallback) {
    var jsonObj = {
        type: "find_extras",
        id: id,
        name: newValue
    };
    postUrl('set_points.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(){
                    if (successCallback)
                        successCallback();
                }, null);
         }, httpErrorResponseHandler);
}

/**
 * Saves a edited find points extra value, this sends the new value to the server
 *
 * @param {Number} id The ID number of the item being edited
 * @param {String} newValue The new value of the item
 * @param {Function} successCallback Function to call once the new value is successfully saved
 */
function saveFindPointsExtraValue(id, newValue, successCallback) {
    var jsonObj = {
        type: "find_extras",
        id: id,
        point_value: newValue
    };
    postUrl('set_points.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(){
                    if (successCallback)
                        successCallback();
                }, null);
         }, httpErrorResponseHandler);
}

/**
 * This creates a new item in the find points extras table
 */
function addNewFindPointsExtra() {
    var jsonObj = {
        "type": "find_extras",
        "new": true
    };
    postUrl('set_points.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(jsonObj) {
                    var table = document.getElementById("find_points_extras_table");
                    var row = table.insertRow(-1);
                    makeFindPointsExtrasRowHtml(jsonObj, row);
                }, null);
         }, httpErrorResponseHandler);
}

/**
 * This deletes an item from the find points extras table
 *
 * @param {Number} id The ID number of the item to delete
 */
function deleteFindPointsExtra(id) {
    var item_name = document.getElementById("table_cell_set_find_points_extras_name_" + id.toString()).dataset.value;
    if (confirm("Are you sure you want to delete \"" + item_name + "\"?")) {
        var jsonObj = {
            "type": "find_extras",
            "id": id,
            "delete": true
        };
        postUrl('set_points.cgi', JSON.stringify(jsonObj), null,
                function(data, responseCode) {
                    httpResponseHandler(data, responseCode, true, function(jsonObj) {
                        var row = document.getElementById("find_points_extras_row_" + id.toString());
                        row.parentNode.removeChild(row);
                    }, null);
             }, httpErrorResponseHandler);
    }
}

/**
 * Saves a edited zone name, this sends the new name to the server
 *
 * @param {Number} id The ID number of the zone being edited
 * @param {String} newValue The new name of the zone
 * @param {Function} successCallback Function to call once the new value is successfully saved
 */
function saveZoneName(id, newValue, successCallback) {
    var jsonObj = {
        type: "zone",
        id: id,
        name: newValue
    };
    postUrl('set_points.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(){
                    if (successCallback)
                        successCallback();
                }, null);
         }, httpErrorResponseHandler);
}

/**
 * Saves a edited zone points, this sends the new point value to the server
 *
 * @param {Number} id The ID number of the zone being edited
 * @param {String} newValue The new points value of the zone
 * @param {Function} successCallback Function to call once the new value is successfully saved
 */
function saveZonePoints(id, newValue, successCallback) {
    var jsonObj = {
        type: "zone",
        id: id,
        point_value: newValue
    };
    postUrl('set_points.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(){
                    if (successCallback)
                        successCallback();
                }, null);
         }, httpErrorResponseHandler);
}

/**
 * This creates a new zone
 */
function addNewZone() {
    var kml = document.getElementById("add_zone_kmls").value;
    var jsonObj = {
        "type": "zone",
        "new": true,
        "kml_file": kml
    };
    postUrl('set_points.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(jsonObj) {
                    var table = document.getElementById("zones_table");
                    var row = table.insertRow(-1);
                    makeZonesRowHtml(jsonObj, row);
                }, null);
         }, httpErrorResponseHandler);
}

/**
 * This deletes a zone from the zones table
 *
 * @param {Number} id The ID number of the zone to delete
 */
function deleteZone(id) {
    var item_name = document.getElementById("table_cell_set_zones_name_" + id.toString()).dataset.value;
    if (confirm("Are you sure you want to delete \"" + item_name + "\"?")) {
        var jsonObj = {
            "type": "zone",
            "id": id,
            "delete": true
        };
        postUrl('set_points.cgi', JSON.stringify(jsonObj), null,
                function(data, responseCode) {
                    httpResponseHandler(data, responseCode, true, function(){
                        var row = document.getElementById("zones_row_" + id.toString());
                        row.parentNode.removeChild(row);
                    }, null);
             }, httpErrorResponseHandler);
    }
}
