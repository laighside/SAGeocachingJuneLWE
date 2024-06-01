/**
  @file    scoring_points_setup.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This is all the JS for the points setup tab on the page /cgi-bin/scoring/scoring.cgi

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

var specialCacheIdCounter = 0;

// (constant) list of id numbers for the items in the game_find_points_trads table
var findTableIds = {
    onePointPerCache: 1,
    walkingPoints: 2,
    zonePoints: 3,
    creativePoints: 4,
    cacheSpacingPoints: 5
};

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

                if (jsonObj.success === false) {
                    if (jsonObj.error && typeof jsonObj.error == 'string') {
                        showTableStatusRow("HTTP error: " + jsonObj.error, "find_points_trads_table");
                    } else {
                        showTableStatusRow("HTTP error: Unknown", "find_points_trads_table");
                    }
                } else {
                    loadFindPointsTradsTable(jsonObj.find_points_trads);
                    loadFindPointsExtrasTable(jsonObj.find_points_extras);
                    loadZonesTable(jsonObj.zones, jsonObj.kml_file_list);
                }

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

    var typeComboBoxCell = document.createElement("td");
    var comboBoxOptions = [
        {
            value: "F",
            display_text: "Find & Hide",
            selected: (jsonData.hide_or_find === "F")
        }, {
            value: "H",
            display_text: "Hide only",
            selected: (jsonData.hide_or_find === "H")
        }
    ]
    typeComboBoxCell.appendChild(makeComboBoxElement("find_points_trads_type_" + jsonData.id.toString(), comboBoxOptions, saveFindPointsTradsType.bind(this, jsonData.id)));
    row.appendChild(typeComboBoxCell);

    var optionsCell = document.createElement("td");
    optionsCell.appendChild(makeFindPointsTradsOptionsHtml(jsonData.id, jsonData.config));
    row.appendChild(optionsCell);
}

/**
 * This makes the HTML elements for the options cell in the find points (trads) table
 *
 * @param {Number} id_number The row id number
 * @param {Object} jsonData The config data for the cell
 */
function makeFindPointsTradsOptionsHtml(id_number, jsonData) {
    var baseElement = document.createElement("div");
    if (jsonData) {

        var subTable = document.createElement("table");
        subTable.classList.add("grey_table_border");
        subTable.style.margin = "auto";

        if (id_number === findTableIds.onePointPerCache) { // Base points

            var row_1 = document.createElement("tr");
            var cell_1_1 = document.createElement("th");
            cell_1_1.colSpan = 2;
            cell_1_1.innerText = "Special caches";
            row_1.appendChild(cell_1_1);
            subTable.appendChild(row_1);

            var row_2 = document.createElement("tr");
            var cell_2_1 = document.createElement("td");
            cell_2_1.innerText = "Cache";
            cell_2_1.style.textAlign = "center";
            row_2.appendChild(cell_2_1);
            var cell_2_2 = document.createElement("td");
            cell_2_2.innerText = "Points";
            cell_2_2.style.textAlign = "center";
            row_2.appendChild(cell_2_2);
            subTable.appendChild(row_2);

            var row_3 = document.createElement("tr");
            row_3.id = "special_caches_none_row";
            row_3.style.display = (jsonData.length > 0) ? "none" : "table-row";
            var cell_3_1 = document.createElement("td");
            cell_3_1.colSpan = 2;
            cell_3_1.innerText = "None";
            cell_3_1.style.fontStyle = "italic";
            cell_3_1.style.textAlign = "center";
            row_3.appendChild(cell_3_1);
            subTable.appendChild(row_3);

            jsonData.sort(function(a, b) {return a.cache - b.cache});
            for (var i = 0; i < jsonData.length; i++) {
                subTable.appendChild(makeSpecialCacheTableRow(specialCacheIdCounter++, jsonData[i]));
            }

            var row_4 = document.createElement("tr");
            row_4.id = "special_caches_add_new_row";
            var cell_4 = document.createElement("td");
            cell_4.colSpan = 2;
            cell_4.style.textAlign = "center";
            var addButton = document.createElement("input");
            addButton.type = "button";
            addButton.value = "Add New";
            addButton.style.fontSize = "14px";
            addButton.style.padding = "0.2em 1em";
            addButton.addEventListener('click', addSpecialCache);
            cell_4.appendChild(addButton);
            row_4.appendChild(cell_4);
            subTable.appendChild(row_4);
        }

        if (id_number === findTableIds.walkingPoints) { // Walking points
            var row_1 = document.createElement("tr");
            var cell_1_1 = document.createElement("td");
            cell_1_1.innerText = "Metres per point:";
            cell_1_1.style.textAlign = "right";
            row_1.appendChild(cell_1_1);
            row_1.appendChild(makeEditableNumberCell(jsonData.distance, "walking_points_distance_value_" + id_number.toString(), saveWalkingPointsConfig.bind(this, id_number), true, "1"));
            subTable.appendChild(row_1);

            var row_2 = document.createElement("tr");
            var cell_2_1 = document.createElement("td");
            cell_2_1.innerText = "Max points:";
            cell_2_1.style.textAlign = "right";
            row_2.appendChild(cell_2_1);
            row_2.appendChild(makeEditableNumberCell(jsonData.max_points, "walking_points_max_value_" + id_number.toString(), saveWalkingPointsConfig.bind(this, id_number), true, "1"));
            subTable.appendChild(row_2);
        }

        if (id_number === findTableIds.creativePoints) { // Creative hide points
            var row_1 = document.createElement("tr");
            var cell_1_1 = document.createElement("td");
            cell_1_1.innerText = "Points per cache:";
            cell_1_1.style.textAlign = "right";
            row_1.appendChild(cell_1_1);
            row_1.appendChild(makeEditableNumberCell(jsonData.points, "creative_hides_point_value_" + id_number.toString(), saveCreativePointsConfig.bind(this, id_number), true, "1"));
            subTable.appendChild(row_1);
        }

        if (id_number === findTableIds.cacheSpacingPoints) { // cache spacing points
            var row_1 = document.createElement("tr");
            var cell_1_1 = document.createElement("td");
            cell_1_1.innerText = "Metres per point:";
            cell_1_1.style.textAlign = "right";
            row_1.appendChild(cell_1_1);
            row_1.appendChild(makeEditableNumberCell(jsonData.distance, "cache_spacing_points_distance_value_" + id_number.toString(), saveCacheSpacingPointsConfig.bind(this, id_number), true, "1"));
            subTable.appendChild(row_1);

            var row_2 = document.createElement("tr");
            var cell_2_1 = document.createElement("td");
            cell_2_1.innerText = "Max points:";
            cell_2_1.style.textAlign = "right";
            row_2.appendChild(cell_2_1);
            row_2.appendChild(makeEditableNumberCell(jsonData.max_points, "cache_spacing_points_max_value_" + id_number.toString(), saveCacheSpacingPointsConfig.bind(this, id_number), true, "1"));
            subTable.appendChild(row_2);

            var row_3 = document.createElement("tr");
            var cell_3 = document.createElement("td");
            cell_3.innerHTML = "Points are applied<br />on a per cache basis";
            cell_3.colSpan = "2";
            cell_3.style.fontStyle = "italic";
            cell_3.style.textAlign = "center";
            row_3.appendChild(cell_3);
            subTable.appendChild(row_3);
        }

        baseElement.appendChild(subTable);

    }
    return baseElement;
}

/**
 * This makes the HTML elements for one row in the special cache points table
 *
 * @param {Number} id Unique id number for the row
 * @param {Object} jsonData The number and point value for the cache
 */
function makeSpecialCacheTableRow(id, jsonData) {
    var row_element = document.createElement("tr");
    row_element.id = "special_cache_row_" + id.toString();
    row_element.appendChild(makeEditableNumberCell(jsonData.cache, "special_cache_number_" + id.toString(), saveSpecialCachesConfig.bind(this, id), true, "1"));
    row_element.appendChild(makeEditableNumberCell(jsonData.points, "special_cache_points_" + id.toString(), saveSpecialCachesConfig.bind(this, id), true, "1"));

    var iconCell = document.createElement("td");
    iconCell.style.textAlign = "center";
    iconCell.appendChild(makeDeleteIcon(deleteSpecialCache.bind(this, id)));
    row_element.appendChild(iconCell);

    return row_element;
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

    row.appendChild(makeEditableTextCell(jsonData.short_name, "find_points_extras_short_name_" + jsonData.id.toString(), saveFindPointsExtraShortName.bind(this, jsonData.id)));
    row.appendChild(makeEditableTextCell(jsonData.long_name, "find_points_extras_long_name_" + jsonData.id.toString(), saveFindPointsExtraLongName.bind(this, jsonData.id)));
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
 * Saves the hide/find type of a find points trads item, this sends the new status to the server
 *
 * @param {Number} id The ID number of the item
 */
function saveFindPointsTradsType(id) {
    var newValue = document.getElementById("find_points_trads_type_" + id.toString()).value;

    var jsonObj = {
        type: "find_trads",
        id: id,
        hide_or_find: newValue
    };
    postUrl('set_points.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, null, null);
         }, httpErrorResponseHandler);
}

/**
 * Saves the config for a find points trads item, this sends the new config to the server
 *
 * @param {Number} id The ID number of the item
 * @param {Object} newValue The new config value
 * @param {Function} successCallback Callback function when new value is successfully sent to server
 */
function saveFindPointsTradsConfig(id, newValue, successCallback) {
    var jsonObj = {
        type: "find_trads",
        id: id,
        config: newValue
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
 * Gets the current state of the special cache points table as an array
 *
 * @param {Number} skipId If set, the row with this ID number is excluded from the resulting array (used for deleting a row)
 */
function getSpecialCacheArray(skipId) {
    var newArray = [];
    for (var i = 0; i <= specialCacheIdCounter; i++) {
        if (i === skipId)
            continue;

        var newCacheNumberElement = document.getElementById("table_cell_input_special_cache_number_" + i.toString())
        var newCacheNumber = null;
        if (newCacheNumberElement)
            newCacheNumber = newCacheNumberElement.value;
        if (!newCacheNumber || newCacheNumber.length === 0) {
            newCacheNumberElement = document.getElementById("table_cell_set_special_cache_number_" + i.toString());
            if (newCacheNumberElement)
                newCacheNumber = newCacheNumberElement.dataset.value;
        }

        var newPointsValueElement = document.getElementById("table_cell_input_special_cache_points_" + i.toString());
        var newPointsValue = null;
        if (newPointsValueElement)
            newPointsValue = newPointsValueElement.value;
        if (!newPointsValue || newPointsValue.length === 0) {
            newPointsValueElement = document.getElementById("table_cell_set_special_cache_points_" + i.toString());
            if (newPointsValueElement)
                newPointsValue = newPointsValueElement.dataset.value;
        }

        if (newCacheNumber && newPointsValue)
            newArray.push({cache: parseInt(newCacheNumber), points: parseInt(newPointsValue)});
    }
    return newArray;
}

/**
 * Saves any changes to the special cache points config
 *
 * @param {Number} id Unused
 * @param {Object} newValue Unused
 * @param {Function} successCallback Callback function when new value is successfully sent to server
 */
function saveSpecialCachesConfig(id, newValue, successCallback) {
    var newArray = getSpecialCacheArray();
    saveFindPointsTradsConfig(findTableIds.onePointPerCache, newArray, successCallback);
}

/**
 * Deletes a row from the special cache points config table
 *
 * @param {Number} id The ID number of the row to delete
 */
function deleteSpecialCache(id) {
    var newArray = getSpecialCacheArray(id);
    saveFindPointsTradsConfig(findTableIds.onePointPerCache, newArray, function() {
        var row = document.getElementById("special_cache_row_" + id.toString());
        if (row) {
            row.parentNode.removeChild(row);
        }
        document.getElementById("special_caches_none_row").style.display = (newArray.length > 0) ? "none" : "table-row";
    });
}

/**
 * Add a new row to the special cache points config table
 */
function addSpecialCache() {
    var newArray = getSpecialCacheArray();
    newArray.push({cache: 1, points: 1})
    saveFindPointsTradsConfig(findTableIds.onePointPerCache, newArray, function() {
        var target = document.getElementById("special_caches_add_new_row");
        target.parentNode.insertBefore(makeSpecialCacheTableRow(specialCacheIdCounter++, {cache: 1, points: 1}), target);
        document.getElementById("special_caches_none_row").style.display = (newArray.length > 0) ? "none" : "table-row";
    });
}

/**
 * Saves any changes to the walking points config
 *
 * @param {Number} id Should always be 2
 * @param {Object} newValue Unused
 * @param {Function} successCallback Callback function when new value is successfully sent to server
 */
function saveWalkingPointsConfig(id, newValue, successCallback) {
    var newDistanceValue = document.getElementById("table_cell_input_walking_points_distance_value_" + id.toString()).value;
    if (!newDistanceValue || newDistanceValue.length === 0)
        newDistanceValue = document.getElementById("table_cell_set_walking_points_distance_value_" + id.toString()).dataset.value;

    var newMaxPointsValue = document.getElementById("table_cell_input_walking_points_max_value_" + id.toString()).value;
    if (!newMaxPointsValue || newMaxPointsValue.length === 0)
        newMaxPointsValue = document.getElementById("table_cell_set_walking_points_max_value_" + id.toString()).dataset.value;

    saveFindPointsTradsConfig(findTableIds.walkingPoints, {
                                  distance: parseInt(newDistanceValue),
                                  max_points: parseInt(newMaxPointsValue)
                              }, successCallback);
}

/**
 * Saves changes to the creative points config
 *
 * @param {Number} id Unused
 * @param {Object} newValue New point value
 * @param {Function} successCallback Callback function when new value is successfully sent to server
 */
function saveCreativePointsConfig(id, newValue, successCallback) {
    saveFindPointsTradsConfig(findTableIds.creativePoints, {points: parseInt(newValue)}, successCallback);
}

/**
 * Saves any changes to the cache spacing points config
 *
 * @param {Number} id Should always be 5
 * @param {Object} newValue Unused
 * @param {Function} successCallback Callback function when new value is successfully sent to server
 */
function saveCacheSpacingPointsConfig(id, newValue, successCallback) {
    var newDistanceValue = document.getElementById("table_cell_input_cache_spacing_points_distance_value_" + id.toString()).value;
    if (!newDistanceValue || newDistanceValue.length === 0)
        newDistanceValue = document.getElementById("table_cell_set_cache_spacing_points_distance_value_" + id.toString()).dataset.value;

    var newMaxPointsValue = document.getElementById("table_cell_input_cache_spacing_points_max_value_" + id.toString()).value;
    if (!newMaxPointsValue || newMaxPointsValue.length === 0)
        newMaxPointsValue = document.getElementById("table_cell_set_cache_spacing_points_max_value_" + id.toString()).dataset.value;

    saveFindPointsTradsConfig(findTableIds.cacheSpacingPoints, {
                                  distance: parseInt(newDistanceValue),
                                  max_points: parseInt(newMaxPointsValue)
                              }, successCallback);
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
 * Saves a edited find points extra short name, this sends the new name to the server
 *
 * @param {Number} id The ID number of the item being edited
 * @param {String} newValue The new short name of the item
 * @param {Function} successCallback Function to call once the new value is successfully saved
 */
function saveFindPointsExtraShortName(id, newValue, successCallback) {
    var jsonObj = {
        type: "find_extras",
        id: id,
        short_name: newValue
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
 * Saves a edited find points extra long name, this sends the new name to the server
 *
 * @param {Number} id The ID number of the item being edited
 * @param {String} newValue The new long name of the item
 * @param {Function} successCallback Function to call once the new value is successfully saved
 */
function saveFindPointsExtraLongName(id, newValue, successCallback) {
    var jsonObj = {
        type: "find_extras",
        id: id,
        long_name: newValue
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
    var item_name = document.getElementById("table_cell_set_find_points_extras_long_name_" + id.toString()).dataset.value;
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
