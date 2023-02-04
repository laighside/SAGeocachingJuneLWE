/**
  @file    scoring.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This is all the JS for the page /cgi-bin/scoring/scoring.cgi

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

/**
 * Loads the initial state of the cache allocation, and puts the drag/drop icons next to the teams
 *
 * @param {Array} cacheList A list of caches and teams they are allocated to
 */
function loadCaches(cacheList) {
    for (var i = 0; i < cacheList.length; i++) {
        if (cacheList[i].cache && cacheList[i].team) {
            var cacheNumberStr = cacheList[i].cache.toString();
            var teamIdStr = cacheList[i].team.toString();
            var cacheIcon = document.createElement("span");
            cacheIcon.appendChild(document.createTextNode(cacheNumberStr));
            cacheIcon.id = "cache_" + cacheNumberStr;
            cacheIcon.className = "cacheIcon";
            cacheIcon.draggable = "true";
            cacheIcon.dataset.number = cacheList[i].cache;
            cacheIcon.addEventListener('dragstart', function() {drag(event)}, false);
            var teamElement = document.getElementById("team_caches_" + teamIdStr);
            if (typeof(teamElement) == 'undefined' || teamElement == null) {
                teamElement = document.getElementById("team_caches_-1");
            }
            teamElement.appendChild(cacheIcon);
        }
    }
}

/**
 * Runs the auto team importer, called when the user clicks that button
 * The page refreshes after running so the new cache data is loaded
 */
function autoTeamImport(){
    if (confirm("This will overwrite all existing team data. Are you sure you want to run the auto import?") == true) {
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
    if (team_name != null) {
        var jsonObj = {
            "team_id":0,
            "team_name":team_name
        };
        postUrl('save_team_info.cgi', JSON.stringify(jsonObj), null,
                function(data, responseCode) {
                    httpResponseHandler(data, responseCode, false, function(jsonResponse){
                        if (jsonResponse.success && typeof jsonResponse.team_id === 'number') {

                            var team_id = jsonResponse.team_id;
                            var table = document.getElementById("game_teams_table");
                            var row = table.insertRow(-1);
                            row.innerHTML = '<td id="team_name_' + team_id.toString() + '" style="position:relative;"></td><td id="team_members_' + team_id.toString() + '" style="position:relative;"><td id="team_caches_' + team_id.toString() + '" ondrop="drop(event, this)" ondragover="allowDrop(event)" data-team-id="' + team_id.toString() + '"></td>';

                            var el = document.getElementById("team_name_" + team_id.toString());
                            var savedHtml = '<span id="team_name_display_' + team_id.toString() + '"></span><svg class="iconButton" style="float:right;" onclick="editTeamName(' + team_id.toString() + ')" width="20" height="20"><image xlink:href="/img/edit.svg" width="20" height="20"></image></svg>';
                            el.innerHTML = savedHtml;
                            el.dataset.value = team_name;
                            document.getElementById("team_name_display_" + team_id.toString()).innerText = team_name;

                            el = document.getElementById("team_members_" + team_id.toString());
                            savedHtml = '<span id="team_members_display_' + team_id.toString() + '"></span><svg class="iconButton" style="float:right;" onclick="editTeamMembers(' + team_id.toString() + ')" width="20" height="20"><image xlink:href="/img/edit.svg" width="20" height="20"></image></svg>';
                            el.innerHTML = savedHtml;
                            el.dataset.value = '';

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
    var team_name = document.getElementById("team_name_" + team_id.toString()).dataset.value;
    if (confirm("Are you sure you want to delete team \"" + team_name + "\"?") == true) {
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
 * Starts the editing of a team name, called when the user clicks a edit button
 * This just shows the text input element
 *
 * @param {Number} team_id The ID number of the team to edit
 */
function editTeamName(team_id) {
    var el = document.getElementById("team_name_" + team_id.toString());
    var editHtml = '<input id="team_name_input_' + team_id.toString() + '" type="text" style="width:100%;" /><svg class="iconButton textboxTick" onclick="saveTeamName(' + team_id.toString() + ')" width="20" height="20"><image xlink:href="/img/tick.svg" width="20" height="20"></image></svg>';
    el.innerHTML = editHtml;
    document.getElementById("team_name_input_" + team_id.toString()).value = el.dataset.value;
}

/**
 * Saves a edited team name, called when the user clicks a save button
 * This sends the new name to the server and removes the text input element on success
 *
 * @param {Number} team_id The ID number of the team being edited
 */
function saveTeamName(team_id) {
    var newValue = document.getElementById("team_name_input_" + team_id.toString()).value;

    var jsonObj = {
        "team_id":team_id,
        "team_name":newValue
    };
    postUrl('save_team_info.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(){

                    var el = document.getElementById("team_name_" + team_id.toString());
                    var savedHtml = '<span id="team_name_display_' + team_id.toString() + '"></span><svg class="iconButton" style="float:right;" onclick="editTeamName(' + team_id.toString() + ')" width="20" height="20"><image xlink:href="/img/edit.svg" width="20" height="20"></image></svg>';
                    el.innerHTML = savedHtml;
                    el.dataset.value = newValue;
                    document.getElementById("team_name_display_" + team_id.toString()).innerText = newValue;

                }, null);
         }, httpErrorResponseHandler);
}

/**
 * Starts the editing of a team member list, called when the user clicks a edit button
 * This just shows the text input element
 *
 * @param {Number} team_id The ID number of the team to edit
 */
function editTeamMembers(team_id) {
    var el = document.getElementById("team_members_" + team_id.toString());
    var editHtml = '<input id="team_members_input_' + team_id.toString() + '" type="text" style="width:100%;" /><svg class="iconButton textboxTick" onclick="saveTeamMembers(' + team_id.toString() + ')" width="20" height="20"><image xlink:href="/img/tick.svg" width="20" height="20"></image></svg>';
    el.innerHTML = editHtml;
    document.getElementById("team_members_input_" + team_id.toString()).value = el.dataset.value;
}

/**
 * Saves a edited team member list, called when the user clicks a save button
 * This sends the new member list to the server and removes the text input element on success
 *
 * @param {Number} team_id The ID number of the team being edited
 */
function saveTeamMembers(team_id) {
    var newValue = document.getElementById("team_members_input_" + team_id.toString()).value;

    var jsonObj = {
        "team_id":team_id,
        "team_members":newValue
    };
    postUrl('save_team_info.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(){

                    var el = document.getElementById("team_members_" + team_id.toString());
                    var savedHtml = '<span id="team_members_display_' + team_id.toString() + '"></span><svg class="iconButton" style="float:right;" onclick="editTeamMembers(' + team_id.toString() + ')" width="20" height="20"><image xlink:href="/img/edit.svg" width="20" height="20"></image></svg>';
                    el.innerHTML = savedHtml;
                    el.dataset.value = newValue;
                    document.getElementById("team_members_display_" + team_id.toString()).innerText = newValue;

                }, null);
         }, httpErrorResponseHandler);
}

/**
 * Sets a team as competing or non-competing (toggles the current value)
 *
 * @param {Number} team_id The ID number of the team
 * @param {Object} menu_item The competing/non-competing button element
 */
function setTeamCompeting(team_id, menu_item) {
    var row = document.getElementById("team_name_" + team_id.toString()).parentNode;
    var new_competing_value = row.classList.contains("nonCompeteTeam");
    var jsonObj = {
        "team_id":team_id,
        "competing":new_competing_value
    };
    postUrl('save_team_info.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(){

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
function allowDrop(ev) {
    ev.preventDefault();
}
function drag(ev) {
    ev.dataTransfer.setData("text", ev.target.id);
    ev.dataTransfer.setData("id", ev.target.dataset.number);
}
function drop(ev, dropItem) {
    ev.preventDefault();
    var cacheIconId = ev.dataTransfer.getData("text");
    var oldParent = document.getElementById(cacheIconId).parentNode;
    dropItem.appendChild(document.getElementById(cacheIconId));

    var jsonObj = {
        "team_id":Number(dropItem.dataset.teamId),
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

/**
 * Called when the team scores tab is opened
 * This downloads the list of teams/scores and displays it
 */
function openTeamScoresTab() {
    downloadUrl('get_scores.cgi', null,
            function(data, responseCode) {
                var jsonObj = JSON.parse(data);
                var table = document.getElementById("team_scores_table");

                while (table.rows.length > 1) {
                    table.deleteRow(-1);
                }

                jsonObj.teams.sort(function(a, b) {
                    return compareStrings(a.team_name, b.team_name);
                })

                for (var i = 0; i < jsonObj.teams.length; i++) {
                    var jsonTeam = jsonObj.teams[i];

                    var row = table.insertRow(-1);
                    row.insertCell(0).innerText = jsonTeam.team_name;
                    row.insertCell(1).innerText = jsonTeam.zone_points;
                    row.insertCell(2).innerText = jsonTeam.returned_points;
                    var score_cell = row.insertCell(3);
                    score_cell.id = 'score_cell_' + jsonTeam.team_id.toString();
                    score_cell.dataset.value = jsonTeam.final_score;

                    var savedHtml = '<span id="final_score_display_' + jsonTeam.team_id.toString() + '"></span><svg class="iconButton" style="float:right;" onclick="editFinalScore(' + jsonTeam.team_id.toString() + ')" width="20" height="20"><image xlink:href="/img/edit.svg" width="20" height="20"></image></svg>';
                    score_cell.innerHTML = savedHtml;
                    document.getElementById("final_score_display_" + jsonTeam.team_id.toString()).innerText = (score_cell.dataset.value === 'null' ? '' : (Number(score_cell.dataset.value) === -1000 ? 'DSQ/DNF' : Number(score_cell.dataset.value) / 10));
                }

         }, httpErrorResponseHandler);
}

/**
 * Case-insensitive comparison of two string (for sorting by alphabetical order)
 *
 * @param {String} a One string
 * @param {String} b Another string
 * @returns {Number} 1 or -1 depending on which comes first alphabetically, or 0 if the strings are the same
 */
function compareStrings(a, b) {
    a = a.toLowerCase();
    b = b.toLowerCase();
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

/**
 * Starts the editing of a team score, called when the user clicks a edit button
 * This just shows the number input element
 *
 * @param {Number} team_id The ID number of the team to edit
 */
function editFinalScore(team_id) {
    var el = document.getElementById("score_cell_" + team_id.toString());
    var editHtml = '<input id="score_cell_input_' + team_id.toString() + '" type="number" step="any" style="width:100px;" /><svg class="iconButton" onclick="saveFinalScore(' + team_id.toString() + ')" width="20" height="20"><image xlink:href="/img/tick.svg" width="20" height="20"></image></svg>';
    el.innerHTML = editHtml;
    document.getElementById("score_cell_input_" + team_id.toString()).value = (el.dataset.value === 'null' ? '' : (Number(el.dataset.value) === -1000 ? '' : Number(el.dataset.value) / 10));
}

/**
 * Saves a edited team score, called when the user clicks a save button
 * This sends the new score to the server and removes the number input element on success
 *
 * @param {Number} team_id The ID number of the team being edited
 */
function saveFinalScore(team_id) {
    var input_element = document.getElementById("score_cell_input_" + team_id.toString());
    var newValue = Math.max(Math.round(Number(input_element.value) * 10), -1000);
    if (input_element.value === '')
        newValue = null;

    var jsonObj = {
        "team_id":team_id,
        "final_score":newValue
    };
    postUrl('save_team_info.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(){

                    var el = document.getElementById("score_cell_" + team_id.toString());
                    var savedHtml = '<span id="final_score_display_' + team_id.toString() + '"></span><svg class="iconButton" style="float:right;" onclick="editFinalScore(' + team_id.toString() + ')" width="20" height="20"><image xlink:href="/img/edit.svg" width="20" height="20"></image></svg>';
                    el.innerHTML = savedHtml;
                    el.dataset.value = newValue;
                    document.getElementById("final_score_display_" + team_id.toString()).innerText = (el.dataset.value === 'null' ? '' : (Number(el.dataset.value) === -1000 ? 'DSQ/DNF' : Number(el.dataset.value) / 10));

                }, null);
         }, httpErrorResponseHandler);
}

/**
 * Called when the powerpoint tab is opened
 * This downloads the list of teams/scores and displays it the team placings order
 */
function openPowerpointTab() {
    downloadUrl('get_scores.cgi', null,
            function(data, responseCode) {
                var jsonObj = JSON.parse(data);
                var table = document.getElementById("scoreboard_table");

                while (table.rows.length > 1) {
                    table.deleteRow(-1);
                }

                // Sort by score
                jsonObj.teams.sort(function(a, b) {
                    if (a.final_score === null && b.final_score !== null)
                        return 1;
                    if (b.final_score === null && a.final_score !== null)
                        return -1;
                    if (a.final_score === null && b.final_score === null)
                        return compareStrings(a.team_name, b.team_name);

                    return (a.final_score > b.final_score) ? -1 : (a.final_score < b.final_score) ? 1 : compareStrings(a.team_name, b.team_name);
                })

                var previous_score = 0;
                var previous_position = 1;
                for (var i = 0; i < jsonObj.teams.length; i++) {
                    var jsonTeam = jsonObj.teams[i];

                    var score = jsonTeam.final_score;
                    var position = '';
                    if (score !== null && score !== -1000) {
                        if (score === previous_score) {
                            position = previous_position;
                        } else {
                            position = i + 1;
                            previous_score = score;
                            previous_position = i + 1;
                        }
                    }

                    var row = table.insertRow(-1);
                    row.insertCell(0).innerText = position;
                    row.insertCell(1).innerText = (score === null ? '-' : (score === -1000 ? 'DSQ/DNF' : Number(score) / 10));
                    row.insertCell(2).innerText = jsonTeam.team_name;
                    row.insertCell(3).innerText = jsonTeam.team_members;
                }

         }, httpErrorResponseHandler);
}

/**
 * Starts the editing of a best cache winner, called when the user clicks a edit button
 * This just shows the text input element
 *
 * @param {Number} award_id The ID number of the award to edit
 */
function editBestCache(award_id) {
    var el = document.getElementById("best_cache_" + award_id.toString());
    var editHtml = '<input id="best_cache_input_' + award_id.toString() + '" type="text" style="width:100%;" /><svg class="iconButton textboxTick" onclick="saveBestCache(' + award_id.toString() + ')" width="20" height="20"><image xlink:href="/img/tick.svg" width="20" height="20"></image></svg>';
    el.innerHTML = editHtml;
    document.getElementById("best_cache_input_" + award_id.toString()).value = el.dataset.value;
}

/**
 * Saves a edited best cache winner, called when the user clicks a save button
 * This sends the new best cache winner to the server and removes the text input element on success
 *
 * @param {Number} award_id The ID number of the award being edited
 */
function saveBestCache(award_id) {
    var newValue = document.getElementById("best_cache_input_" + award_id.toString()).value;

    var jsonObj = {
        "award_id":award_id,
        "cache_name":newValue
    };
    postUrl('set_best_cache.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(){

                    var el = document.getElementById("best_cache_" + award_id.toString());
                    var savedHtml = '<span id="best_cache_display_' + award_id.toString() + '"></span><svg class="iconButton" style="float:right;" onclick="editBestCache(' + award_id.toString() + ')" width="20" height="20"><image xlink:href="/img/edit.svg" width="20" height="20"></image></svg>';
                    el.innerHTML = savedHtml;
                    el.dataset.value = newValue;
                    document.getElementById("best_cache_display_" + award_id.toString()).innerText = newValue;

                }, null);
         }, httpErrorResponseHandler);
}
