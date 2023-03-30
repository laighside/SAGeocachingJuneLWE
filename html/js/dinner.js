/**
  @file    dinner.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This is the JS for the dinner page within the registration page /cgi-bin/registration/registration_form.cgi

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

// stores the list of items available for purchase (downloaded from server)
var dinner_info = null;

// stores the users selection during changes to the number of meals
var dinner_selected_options = {};

function loadDinnerItems(url, displayID){
    downloadUrl(url, null,
        function(data, responseCode) {
            if (responseCode === 200){
                var json_data = JSON.parse(data);
                dinner_info = json_data.dinner;
                displayDinnerItems(displayID);
            }
     });
}

function displayDinnerItems(displayID) {
  var output = "";
  var i;
  for (i = 0; i < dinner_info.items.length; i++) {
      var id_as_string = dinner_info.items[i].id.toString();
      output += "<p>How many " + dinner_info.items[i].name_plural + " would you like to order: <input type=\"number\" id=\"item_" + id_as_string + "_count\" name=\"item_" + id_as_string + "_count\" min=\"0\" max=\"10\" value=\"0\" onchange=\"itemCountChanged(" + id_as_string + ")\"></p>\n";
      output += "<div id=\"item_" + id_as_string + "_options_block\"></div>\n";
  }
  document.getElementById(displayID).innerHTML = output;
}

function itemCountChanged(item_id) {
  saveOptions(item_id);
  var item_count = parseInt(document.getElementById("item_" + item_id.toString() + "_count").value);
  var htmlOutput = "";
  var i;
  for (i = 0; i < item_count; i++) {
    var options_html = makeOptionsHTML(i, item_id);
    if (options_html.length > 0) {
      htmlOutput += options_html;
    }
  }
  document.getElementById("item_" + item_id.toString() + "_options_block").innerHTML = htmlOutput;
  restoreOptions(item_id);
}

function makeOptionsHTML(i, item_id) {
  var item = dinner_info.items.filter(function(item) {return item.id === item_id})[0];
  if (item) {
    if (item.options.length > 0) {
      var output = "<div class=\"formOptionBox\"><p>" + item.name + " #" + (i+1).toString() + ": </p>";
      output += "<div>"
      var j;
      for (j = 0; j < item.options.length; j++) {
        output += makeOptionHTML("item_" + item_id.toString() + "_" + i.toString() + "_option_" + item.options[j].id.toString(), item.options[j]);
      }
      output += "</div></div>";
      return output;
    } else {
      return "";
    }
  } else {
    return "";
  }
}

function makeOptionHTML(id_str, option) {
    var output = "<div><div style=\"display: inline-block;vertical-align: middle;\"><p>" + option.question + ": </p></div>";

    if (option.type === 'select') {
        output += "<div style=\"display: inline-block;vertical-align: middle;\"><p>";
        output += "<select id=\"" + id_str + "\" name=\"" + id_str + "\">";
        var select_options = option.values;
        var i;
        for (i = 0; i < select_options.length; i++) {
            output += "<option value=\"" + select_options[i] + "\">" + select_options[i] + "</option>";
        }
        output += "</select>";
    }

    if (option.type === 'checkbox') {
        output += "<div class=\"margin\" style=\"display: inline-block;vertical-align: middle;\"><p style=\"line-height:2em;\">"
        var checkbox_options = option.values;
        var j;
        for (j = 0; j < checkbox_options.length; j++) {
            output += "<span class=\"checkbox_container\"><label>" + checkbox_options[j];
            output += "    <input type=\"checkbox\" id=\"" + id_str + "_value_" + j.toString() + "\" value=\"true\" />";
            output += "    <span class=\"checkmark\"></span>";
            output += "  </label></span><br />";
        }
    }

    if (option.type === 'text' || option.type === '') {
        output += "<input type=\"text\" style=\"width:200px;\" id=\"" + id_str + "\" name=\"" + id_str + "\"  oninput=\"this.className = ''\">";
    }
    return output + "</p></div></div>";
}

function saveOptions(item_id) {
    var item_count = parseInt(document.getElementById("item_" + item_id.toString() + "_count").value);
    var item = dinner_info.items.filter(function(item) {return item.id === item_id})[0];

    var result = [];
    var i;
    for (i = 0; i < item_count; i++) {
        var item_options = {};
        var j;
        for (j = 0; j < item.options.length; j++) {
            if (item.options[j].type === 'select') {
                var ele = document.getElementById("item_" + item_id.toString() + "_" + i.toString() + "_option_" + item.options[j].id.toString());
                if (ele) {
                    item_options[item.options[j].id.toString()] = ele.value;
                }
            } else if (item.options[j].type === 'checkbox') {
                var checkboxes = [];
                var k;
                for (k = 0; k < item.options[j].values.length; k++) {
                    var cb = document.getElementById("item_" + item_id.toString() + "_" + i.toString() + "_option_" + item.options[j].id.toString() + "_value_" + k.toString());
                    if (cb) {
                        checkboxes.push(cb.checked);
                    } else {
                        checkboxes.push(false);
                    }
                }
                item_options[item.options[j].id.toString()] = checkboxes;
            } else { // text
                var text_ele = document.getElementById("item_" + item_id.toString() + "_" + i.toString() + "_option_" + item.options[j].id.toString());
                if (text_ele) {
                    item_options[item.options[j].id.toString()] = text_ele.value;
                }
            }
        }
        result.push(item_options);
    }
    dinner_selected_options[item_id.toString()] = result;
}

function restoreOptions(item_id) {
    var item = dinner_info.items.filter(function(item) {return item.id === item_id})[0];

    var options_array = dinner_selected_options[item_id.toString()];
    if (options_array) {
        var i;
        for (i = 0; i < options_array.length; i++) {
            var item_options = options_array[i];
            if (item_options) {
                var j;
                for (j = 0; j < item.options.length; j++) {
                    if (item.options[j].type === 'select') {
                        var ele = document.getElementById("item_" + item_id.toString() + "_" + i.toString() + "_option_" + item.options[j].id.toString());
                        if (ele && item_options[item.options[j].id.toString()]) {
                            ele.value = item_options[item.options[j].id.toString()];
                        }
                    } else if (item.options[j].type === 'checkbox') {
                        var cb_values = item_options[item.options[j].id.toString()];
                        if (cb_values) {
                            var k;
                            for (k = 0; k < cb_values.length; k++) {
                                var cb = document.getElementById("item_" + item_id.toString() + "_" + i.toString() + "_option_" + item.options[j].id.toString() + "_value_" + k.toString());
                                if (cb) {
                                    cb.checked = cb_values[k];
                                }
                            }
                        }
                    } else {
                        var text_ele = document.getElementById("item_" + item_id.toString() + "_" + i.toString() + "_option_" + item.options[j].id.toString());
                        if (text_ele && item_options[item.options[j].id.toString()]) {
                            text_ele.value = item_options[item.options[j].id.toString()];
                        }
                    }
                }
            }
        }
    }
}
