/**
  @file    scoring_team_scores.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This is all the JS for the team scores tab on the page /cgi-bin/scoring/scoring.cgi

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

var trad_hide_points = [];

/**
 * Called when the team scores tab is opened
 * This downloads the list of teams/scores and displays it
 */
function openTeamScoresTab() {
    showTableStatusRow("Loading...", "team_scores_table");
    document.getElementById("scoring_errors").innerHTML = "";

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

                if (jsonObj.warning_cache_not_in_handout)
                    addErrorMessage("Warning: There are caches that have not been handed out");
                if (jsonObj.warning_cache_not_in_gpx)
                    addErrorMessage("Warning: There are caches that are not in the GPX file");

                if (jsonObj.trad_points)
                    trad_hide_points = jsonObj.trad_points.filter(function(x){return x.hide_or_find === "H"});

                for (var i = 0; i < jsonObj.teams.length; i++) {
                    var jsonTeam = jsonObj.teams[i];
                    makeTeamScoresTableRowHtml(jsonTeam, table);
                    table.appendChild(makeTradFindPointsTableRowHtml(jsonTeam, jsonObj.cache_list, jsonObj.number_game_caches));
                    table.appendChild(makeExtrasFindPointsTableRowHtml(jsonTeam, jsonObj.extras_points));
                    table.appendChild(makeHidePointsTableRowHtml(jsonTeam, jsonObj.cache_list, jsonObj.use_totals_for_best_cache_calculation));
                    table.appendChild(makePenaltiesPointsTableRowHtml(jsonTeam));
                }

                // Work out total trad points
                var total_points = 0;
                var cache_count = 0;
                for (var i = 0; i < jsonObj.cache_list.length; i++) {
                    if (jsonObj.cache_list[i].has_coordinates) {
                        cache_count++;
                        total_points += jsonObj.cache_list[i].total_find_points;
                    }
                }
                var totals_html = "Total traditional points: " + total_points.toString() + " points<br />Average points/cache: " + ((cache_count > 0) ? (total_points / cache_count) : 0).toFixed(3) + " points per cache (" + cache_count.toString() + " caches in GPX file)";

                document.getElementById("team_scores_total_p").innerHTML = "Number of teams: " + jsonObj.teams.length.toString() + " competing teams<br />" + totals_html;

         }, function(statusCode, statusText) {
             if (statusText && typeof statusText == 'string') {
                 showTeamScoresStatusRow("HTTP error: " + statusText);
             } else {
                 showTeamScoresStatusRow("HTTP error: Unknown");
             }
         });
}

/**
 * This makes the HTML elements for a single row of table
 *
 * @param {Object} jsonData The data for the row
 */
function makeTeamScoresTableRowHtml(jsonData, table) {
    var row = table.insertRow(-1);
    row.appendChild(makeHtmlElement("td", jsonData.team_name));
    row.appendChild(makeExpandCell(jsonData.trad_find_points.toString(), "find_points_" + jsonData.team_id.toString(), expandOnChange));
    row.appendChild(makeExpandCell(jsonData.extra_find_points.toString(), "find_points_extras_" + jsonData.team_id.toString(), expandOnChange));
    row.appendChild(makeExpandCell(jsonData.hide_points.toString(), "hide_points_" + jsonData.team_id.toString(), expandOnChange));
    row.appendChild(makeExpandCell(jsonData.penalties.toString(), "penalties_" + jsonData.team_id.toString(), expandOnChange));

    var total_score = jsonData.trad_find_points + jsonData.extra_find_points + jsonData.hide_points + jsonData.penalties;
    var calcScoreCell = document.createElement("td");
    calcScoreCell.id = "calc_score_cell_" + jsonData.team_id.toString();
    calcScoreCell.innerText = total_score.toString();
    if (jsonData.final_score)
        if (total_score !== jsonData.final_score / 10)
            calcScoreCell.className = "red_background";
    row.appendChild(calcScoreCell);

    var score_cell = row.insertCell(6);
    score_cell.id = 'score_cell_' + jsonData.team_id.toString();
    score_cell.dataset.value = jsonData.final_score;

    var savedHtml = '<span id="final_score_display_' + jsonData.team_id.toString() + '"></span><svg class="iconButton" style="float:right;" onclick="editFinalScore(' + jsonData.team_id.toString() + ')" width="20" height="20"><image xlink:href="/img/edit.svg" width="20" height="20"></image></svg>';
    score_cell.innerHTML = savedHtml;
    document.getElementById("final_score_display_" + jsonData.team_id.toString()).innerText = (score_cell.dataset.value === 'null' ? '' : (Number(score_cell.dataset.value) === -1000 ? 'DSQ/DNF' : Number(score_cell.dataset.value) / 10));
}

/**
 * Makes the table row to show the trad find points for a single team
 *
 * @param {Object} jsonData The team data for the row
 * @returns {Object} The row element
 */
function makeTradFindPointsTableRowHtml(jsonData, cache_list, number_game_caches) {
    var baseRow = document.createElement("tr");
    baseRow.id = "row_team_find_points_" + jsonData.team_id.toString();
    baseRow.style.display = "none";
    var baseCell = document.createElement("td");
    baseCell.colSpan = 7;

    var subTable = document.createElement("table");
    subTable.classList.add("grey_table_border");
    subTable.style.margin = "auto";

    for (var y = 0; y < Math.ceil(number_game_caches/10); y++) {
        var cacheRow = document.createElement("tr");
        for (var x = 0; x < 10; x++) {
            var cacheNumber = y * 10 + x + 1;
            if (cacheNumber > number_game_caches)
                continue;

            var cacheCell = document.createElement("td");
            cacheCell.style.textAlign = "center";

            var cacheIcon = document.createElement("span");
            cacheIcon.innerText = cacheNumber.toString();
            cacheIcon.className = "cacheIcon";
            cacheCell.appendChild(cacheIcon);

            cacheCell.appendChild(document.createElement("br"));
            var pointsSpan = document.createElement("span");

            if (cache_list[cacheNumber - 1] && cache_list[cacheNumber - 1].has_coordinates) {

                if (jsonData.trad_finds[cacheNumber - 1])
                    cacheCell.className = "green_background";

                pointsSpan.innerText = cache_list[cacheNumber - 1].total_find_points.toString() + " pts";
            } else {
                cacheCell.className = "grey_background";
                pointsSpan.innerText = "-";

                if (jsonData.trad_finds[cacheNumber - 1])
                    addErrorMessage("Error: Find recorded on a cache that is not in the GPX file (cache " + cacheNumber.toString() + ", team " + jsonData.team_id.toString() + ")");
            }
            cacheCell.appendChild(pointsSpan);

            cacheRow.appendChild(cacheCell);
        }
        subTable.appendChild(cacheRow);
    }

    baseCell.appendChild(subTable);
    baseRow.appendChild(baseCell);

    return baseRow;
}

/**
 * Makes the table row to show the extras find points for a single team
 *
 * @param {Object} jsonData The team data for the row
 * @returns {Object} The row element
 */
function makeExtrasFindPointsTableRowHtml(jsonData, extras_points) {
    var baseRow = document.createElement("tr");
    baseRow.id = "row_team_find_points_extras_" + jsonData.team_id.toString();
    baseRow.style.display = "none";
    var baseCell = document.createElement("td");
    baseCell.colSpan = 7;

    var subTable = document.createElement("table");
    subTable.classList.add("grey_table_border");
    subTable.style.margin = "auto";

    for (var i = 0; i < extras_points.length; i++) {
        var cacheRow = document.createElement("tr");
        cacheRow.appendChild(makeHtmlElement("td", extras_points[i].long_name));

        var value = jsonData.extra_finds[extras_points[i].id.toString()];
        if (value == null)
            value = 0;
        cacheRow.appendChild(makeEditableNumberCell(value, "extras_edit_" + jsonData.team_id.toString() + "_" + extras_points[i].id.toString(), saveExtraFind.bind(this, jsonData.team_id, extras_points[i].id), true, "1"));

        cacheRow.appendChild(makeHtmlElement("td", "x " + extras_points[i].point_value.toString()));
        subTable.appendChild(cacheRow);
    }

    baseCell.appendChild(subTable);
    baseRow.appendChild(baseCell);

    return baseRow;
}

/**
 * Makes the table row to show the hide points for a single team
 *
 * @param {Object} jsonData The team data for the row
 * @returns {Object} The row element
 */
function makeHidePointsTableRowHtml(jsonData, cache_list, use_totals_for_best_cache_calculation) {
    var baseRow = document.createElement("tr");
    baseRow.id = "row_team_hide_points_" + jsonData.team_id.toString();
    baseRow.style.display = "none";
    var baseCell = document.createElement("td");
    baseCell.colSpan = 7;
    baseRow.appendChild(baseCell);

    var subTable = document.createElement("table");
    subTable.classList.add("grey_table_border");
    subTable.style.margin = "auto";

    var headerRow = document.createElement("tr");
    headerRow.appendChild(makeHtmlElement("th", "Cache"));
    for (var i = 0; i < trad_hide_points.length; i++)
        headerRow.appendChild(makeHtmlElement("th", trad_hide_points[i].item_name));
    if (use_totals_for_best_cache_calculation)
        headerRow.appendChild(makeHtmlElement("th", "Total"));
    subTable.appendChild(headerRow);

    for (var i = 0; i < jsonData.caches.length; i++) {
        var cacheRow = document.createElement("tr");

        var numberCell = document.createElement("td");
        var cacheIcon = document.createElement("span");
        cacheIcon.innerText = jsonData.caches[i].toString();
        cacheIcon.className = "cacheIcon";
        numberCell.appendChild(cacheIcon);
        cacheRow.appendChild(numberCell);

        var cache_data = cache_list[jsonData.caches[i] - 1];

        for (var j = 0; j < trad_hide_points.length; j++) {
            var value = trad_hide_points[j].points_list[jsonData.caches[i] - 1];
            var pointsCell = makeHtmlElement("td", value ? value.toString() : "");
            if (value > 0 && jsonData.hide_points_best_cache_lists[trad_hide_points[j].id.toString()].includes(jsonData.caches[i]))
                pointsCell.className = "green_background";
            cacheRow.appendChild(pointsCell);
        }

        if (use_totals_for_best_cache_calculation) {
            var totalPointsCell = document.createElement("td");
            totalPointsCell.innerText = cache_data ? cache_data.total_hide_points.toString() : "";
            var best_caches = Object.values(jsonData.hide_points_best_cache_lists)[0];
            if (best_caches && best_caches.includes(jsonData.caches[i]))
                totalPointsCell.className = "green_background";
            cacheRow.appendChild(totalPointsCell);
        }

        subTable.appendChild(cacheRow);
    }

    baseCell.appendChild(subTable);

    return baseRow;
}

/**
 * Makes the table row to show the penalty points for a single team
 *
 * @param {Object} jsonData The team data for the row
 * @returns {Object} The row element
 */
function makePenaltiesPointsTableRowHtml(jsonData) {
    var baseRow = document.createElement("tr");
    baseRow.id = "row_team_penalties_" + jsonData.team_id.toString();
    baseRow.style.display = "none";
    var baseCell = document.createElement("td");
    baseCell.colSpan = 7;

    var subTable = document.createElement("table");
    subTable.classList.add("grey_table_border");
    subTable.style.margin = "auto";

    var cacheRow = document.createElement("tr");
    cacheRow.appendChild(makeHtmlElement("td", "Caches not returned"));
    cacheRow.appendChild(makeHtmlElement("td", jsonData.not_returned_caches.toString()));
    cacheRow.appendChild(makeHtmlElement("td", "x -2"));
    subTable.appendChild(cacheRow);

    var lateRow = document.createElement("tr");
    lateRow.appendChild(makeHtmlElement("td", "Minutes late"));
    lateRow.appendChild(makeEditableNumberCell(jsonData.late, "late_edit_" + jsonData.team_id.toString(), saveExtraFind.bind(this, jsonData.team_id, -1), true, "1"));
    lateRow.appendChild(makeHtmlElement("td", "x -1"));
    subTable.appendChild(lateRow);

    baseCell.appendChild(subTable);
    baseRow.appendChild(baseCell);

    return baseRow;
}

/**
 * Sets the visibility of a row in the table, called whenever an row expand toggle is clicked
 *
 * @param {String} idStr The ID (suffix) of the row element
 * @param {Boolean} expandedState True for visible, false for hidden
 */
function expandOnChange(idStr, expandedState) {
    document.getElementById("row_team_" + idStr).style.display = expandedState ? "table-row" : "none";
}

/**
 * Saves any changes to a extras find item
 *
 * @param {Number} team_id The ID number of the team the find is for
 * @param {Number} extras_id The ID number of item the find is for
 * @param {Number} newValue The value of the find, or number of finds
 * @param {Function} successCallback Callback function when new value is successfully sent to server
 */
function saveExtraFind(team_id, extras_id, newValue, successCallback) {
    var jsonObj = {
        team_id: team_id,
        extras_id: extras_id,
        value: newValue
    };
    postUrl('set_find_value.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(){
                    if (successCallback)
                        successCallback();
                }, null);
         }, httpErrorResponseHandler);
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

                    // Update background color of calculated score cell
                    var calcScoreCell = document.getElementById("calc_score_cell_" + team_id.toString());
                    var calcScore = Number(calcScoreCell.innerText);
                    calcScoreCell.className = (calcScore === newValue / 10) ? "" : "red_background";

                }, null);
         }, httpErrorResponseHandler);
}

/**
 * Adds a message to the list of errors at the top of the page
 *
 * @param {String} text Error message (HTML encoded)
 */
function addErrorMessage(text) {
    var scoring_errors_element = document.getElementById("scoring_errors");
    scoring_errors_element.innerHTML += text + "<br />";
}
