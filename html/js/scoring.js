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
 * Called when the leaderboard tab is opened
 * This downloads the list of teams/scores and displays it the team placings order
 */
function openLeaderboardTab() {
    showTableStatusRow("Loading...", "scoreboard_table");

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
