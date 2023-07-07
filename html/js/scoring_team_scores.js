/**
  @file    scoring_team_scores.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This is all the JS for the team scores tab on the page /cgi-bin/scoring/scoring.cgi

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

/**
 * Called when the team scores tab is opened
 * This downloads the list of teams/scores and displays it
 */
function openTeamScoresTab() {
    showTeamScoresStatusRow("Loading...");

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

                document.getElementById("team_scores_total_p").innerText = "Total: " + jsonObj.teams.length.toString() + " competing teams";

         }, function(statusCode, statusText) {
             if (statusText && typeof statusText == 'string') {
                 showTeamScoresStatusRow("HTTP error: " + statusText);
             } else {
                 showTeamScoresStatusRow("HTTP error: Unknown");
             }
         });
}

/**
 * This clears the table and displays a status message on it
 *
 * @param {String} statusText The status message to show
 */
function showTeamScoresStatusRow(statusText) {
    var table = document.getElementById("team_scores_table");
    while (table.rows.length > 1) {
        table.deleteRow(-1);
    }
    var loadingRow = table.insertRow(-1);
    var loadingCell = loadingRow.insertCell(0);
    loadingCell.style.textAlign = "center";
    loadingCell.style.fontStyle = "italic";
    loadingCell.colSpan = 4;
    loadingCell.innerText = statusText;
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
