/**
  @file    scoring_team_list.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This is all the JS for the team list tab on the page /cgi-bin/scoring/scoring.cgi

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

/**
 * Called when the team list tab is opened
 * This downloads the list of teams and places them on the page
 */
function openTeamListTab() {
    showTableStatusRow("Loading...", "team_list_table");

    downloadUrl('get_scores.cgi?include_non_compete=true', null,
            function(data, responseCode) {
                var jsonObj = JSON.parse(data);
                var table = document.getElementById("team_list_table");

                while (table.rows.length > 1) {
                    table.deleteRow(-1);
                }

                jsonObj.teams.sort(function(a, b) {
                    return compareStrings(a.team_name, b.team_name);
                })

                for (var i = 0; i < jsonObj.teams.length; i++) {
                    var jsonTeam = jsonObj.teams[i];
                    var row = table.insertRow(-1);
                    makeTeamListTableRowHtml(jsonTeam, row);
                }

                var unallocatedBox = document.getElementById("team_caches_-1");
                unallocatedBox.innerHTML = "";
                for (var j = 0; j < jsonObj.unallocated_caches.length; j++) {
                    unallocatedBox.appendChild(makeCacheIconElement(jsonObj.unallocated_caches[j]));
                }

         }, function(statusCode, statusText) {
             if (statusText && typeof statusText == 'string') {
                 showTableStatusRow("HTTP error: " + statusText, "team_list_table");
             } else {
                 showTableStatusRow("HTTP error: Unknown", "team_list_table");
             }
         });
}

/**
 * This makes the HTML elements for a single row of the team list table
 *
 * @param {Object} jsonTeam The data for the team
 * @param {Object} row The table row object to add the data to
 */
function makeTeamListTableRowHtml(jsonTeam, row) {
    if (!jsonTeam.competing)
        row.classList.add("nonCompeteTeam");

    row.id = "team_list_row_" + jsonTeam.team_id.toString();

    row.appendChild(makeEditableTextCell(jsonTeam.team_name, "team_name_" + jsonTeam.team_id.toString(), saveTeamName.bind(this, jsonTeam.team_id)));
    row.appendChild(makeEditableTextCell(jsonTeam.team_members, "team_members_" + jsonTeam.team_id.toString(), saveTeamMembers.bind(this, jsonTeam.team_id)));

    var cachesCell = document.createElement("td");
    cachesCell.id = "team_caches_" + jsonTeam.team_id.toString();
    cachesCell.dataset.teamId = jsonTeam.team_id.toString();
    cachesCell.addEventListener('drop', dropCache);
    cachesCell.addEventListener('dragover', allowDropCache);
    for (var i = 0; i < jsonTeam.caches.length; i++) {
        cachesCell.appendChild(makeCacheIconElement(jsonTeam.caches[i]));
    }
    row.appendChild(cachesCell);

    var menuItems = [
                {text:"Delete Team", onClick: deleteTeam.bind(this, jsonTeam.team_id)},
                {text:(jsonTeam.competing ? "Set Non-Competing" : "Set Competing"), onClick: setTeamCompeting.bind(this, jsonTeam.team_id), buttonId: ("competing_button_" + jsonTeam.team_id.toString())}
            ];
    var menuCell = document.createElement("td");
    menuCell.appendChild(makeDropDownMenu(jsonTeam.team_id, menuItems));
    row.appendChild(menuCell);
}

/**
 * This makes the HTML elements for a single cache icon (the yellow box with a number in it)
 *
 * @param {Number} cache_number The number of the cache
 */
function makeCacheIconElement(cache_number) {
    var cacheNumberStr = cache_number.toString();
    var cacheIcon = document.createElement("span");
    cacheIcon.appendChild(document.createTextNode(cacheNumberStr));
    cacheIcon.id = "cache_" + cacheNumberStr;
    cacheIcon.className = "cacheIcon";
    cacheIcon.draggable = "true";
    cacheIcon.dataset.number = cache_number;
    cacheIcon.addEventListener('dragstart', dragCache);
    return cacheIcon;
}

/**
 * Saves a edited team name, this sends the new name to the server
 *
 * @param {Number} team_id The ID number of the team being edited
 * @param {String} newValue The new name of the team
 * @param {Function} successCallback Function to call once the new value is successfully saved
 */
function saveTeamName(team_id, newValue, successCallback) {
    var jsonObj = {
        "team_id":team_id,
        "team_name":newValue
    };
    postUrl('save_team_info.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(){
                    if (successCallback)
                        successCallback();
                }, null);
         }, httpErrorResponseHandler);

}

/**
 * Saves a edited team member list, this sends the new list to the server
 *
 * @param {Number} team_id The ID number of the team being edited
 * @param {String} newValue The new list of team members
 * @param {Function} successCallback Function to call once the new value is successfully saved
 */
function saveTeamMembers(team_id, newValue, successCallback) {
    var jsonObj = {
        "team_id":team_id,
        "team_members":newValue
    };
    postUrl('save_team_info.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(){
                    if (successCallback)
                        successCallback();
                }, null);
         }, httpErrorResponseHandler);

}

/**
 * Runs the auto team importer, called when the user clicks that button
 * The page refreshes after running so the new cache data is loaded
 */
function autoTeamImport() {
    if (confirm("This will overwrite all existing team data. Are you sure you want to run the auto import?")) {
        var jsonObj = {
            "confirm":true
        };
        postUrl('auto_team_import.cgi', JSON.stringify(jsonObj), null,
                function(data, responseCode) {
                    httpResponseHandler(data, responseCode, false, function(){location.reload();}, null);
             }, httpErrorResponseHandler);
    }
}

/**
 * Adds a new team to the list, called when the user clicks the add team button
 */
function addNewTeam(){
    var team_name = prompt("Enter the new team's name:")
    if (team_name && team_name.length) {
        var jsonObj = {
            "team_id":0,
            "team_name":team_name
        };
        postUrl('save_team_info.cgi', JSON.stringify(jsonObj), null,
                function(data, responseCode) {
                    httpResponseHandler(data, responseCode, false, function(jsonResponse){
                        if (jsonResponse.success && typeof jsonResponse.team_id === 'number') {

                            var jsonTeam = {
                                team_id: jsonResponse.team_id,
                                competing: true,
                                team_name: team_name,
                                team_members: ""
                            }

                            var table = document.getElementById("team_list_table");
                            var row = table.insertRow(-1);
                            makeTeamListTableRowHtml(jsonTeam, row);

                        }

                    }, null);
             }, httpErrorResponseHandler);
    }
}

/**
 * Deletes a team from the list, called when the user clicks a delete button
 *
 * @param {Number} team_id The ID number of the team to delete
 */
function deleteTeam(team_id) {
    var team_name = document.getElementById("table_cell_set_team_name_" + team_id.toString()).dataset.value;
    if (confirm("Are you sure you want to delete team \"" + team_name + "\"?")) {
        var jsonObj = {
            "team_id":team_id,
            "delete_team":true
        };
        postUrl('save_team_info.cgi', JSON.stringify(jsonObj), null,
                function(data, responseCode) {
                    httpResponseHandler(data, responseCode, false, function(jsonResponse){
                        if (jsonResponse.success) {

                            var caches_box = document.getElementById("team_caches_" + team_id.toString());
                            var unallocated_box = document.getElementById("team_caches_-1");
                            while (caches_box.childNodes.length > 0) {
                                unallocated_box.appendChild(caches_box.childNodes[0]);
                            }
                            var row = caches_box.parentNode;
                            row.parentNode.removeChild(row);

                        }

                    }, null);
             }, httpErrorResponseHandler);
    }
}

/**
 * Sets a team as competing or non-competing (toggles the current value)
 *
 * @param {Number} team_id The ID number of the team
 * @param {Object} menu_item The competing/non-competing button element
 */
function setTeamCompeting(team_id) {
    var row = document.getElementById("team_list_row_" + team_id.toString());
    var new_competing_value = row.classList.contains("nonCompeteTeam");
    var jsonObj = {
        "team_id":team_id,
        "competing":new_competing_value
    };
    postUrl('save_team_info.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(){

                    var menu_item = document.getElementById("competing_button_" + team_id.toString());
                    if (new_competing_value) {
                        row.classList.remove("nonCompeteTeam");
                        menu_item.innerText = 'Set Non-Competing';
                    } else {
                        row.classList.add("nonCompeteTeam");
                        menu_item.innerText = 'Set Competing';
                    }

                }, null);
         }, httpErrorResponseHandler);
}



/* Drag and Drop functions for the cache allocation editing */
function allowDropCache(ev) {
    ev.preventDefault();
}
function dragCache(ev) {
    ev.dataTransfer.setData("text", ev.target.id);
    ev.dataTransfer.setData("id", ev.target.dataset.number);
}
function dropCache(ev) {
    ev.preventDefault();
    var cacheIconId = ev.dataTransfer.getData("text");
    var oldParent = document.getElementById(cacheIconId).parentNode;
    ev.target.appendChild(document.getElementById(cacheIconId));

    var jsonObj = {
        "team_id":Number(ev.target.dataset.teamId),
        "cache_number":Number(ev.dataTransfer.getData("id"))
    };
    postUrl('save_team_info.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, null, function(){
                    // if an error occurs, put the cache back where it came from
                    oldParent.appendChild(document.getElementById(cacheIconId));
                });
         }, httpErrorResponseHandler);
}
