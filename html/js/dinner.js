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
var dinner_info = {};

// stores the users selection during changes to the number of meals
var dinner_selected_options = {};

// object with true/false for each form ID, used to keep track of if the categories (newer) version of the form is used
var dinner_has_categories = {};

/**
 * Converts a currency value (cents) to a string for the user
 *
 * @param {Number} value The currency value
 * @returns {String} The currency value formatted as a string
 */
function currencyToText(value) {
    if (value % 100 == 0)
        return "$" + (value / 100).toString();
    return "$" + (value / 100).toFixed(2);
}

function loadDinnerItems(url, displayID, dinner_form_id){
    downloadUrl(url, null,
        function(data, responseCode) {
            if (responseCode === 200){
                var json_data = JSON.parse(data);
                dinner_info[dinner_form_id] = json_data.dinner;
                dinner_selected_options[dinner_form_id] = {};
                displayDinnerItems(displayID, dinner_form_id);
            }
     });
}

function displayDinnerItems(displayID, dinner_form_id) {
  var output = "";
  var i;

  dinner_selected_options[dinner_form_id].categories = {};

  if (dinner_info[dinner_form_id].config && dinner_info[dinner_form_id].config.categories && dinner_info[dinner_form_id].config.categories.length > 0) {
      // This is a form with many meal choices

      dinner_has_categories[dinner_form_id] = true;

  } else {
      // This is a basic form (ie. set menu at a fixed cost)

      dinner_has_categories[dinner_form_id] = false;

      dinner_info[dinner_form_id].config.categories = dinner_info[dinner_form_id].items;
  }

  for (i = 0; i < dinner_info[dinner_form_id].config.categories.length; i++) {
      var category = dinner_info[dinner_form_id].config.categories[i];
      output += "<h2>" + (category.name_plural ? category.name_plural : category.name).toString() + "</h2>\n";
      output += "<div id=\"" + dinner_form_id.toString() + "_category_" + category.id.toString() + "_order_block\"></div>\n";
      output += "<input type=\"button\" id=\"" + dinner_form_id.toString() + "_add_button_" + category.id.toString() + "\" class=\"formOptionBox mealAddButton\" onclick=\"addNewMeal(" + dinner_form_id.toString() + "," + category.id.toString() + ")\" value=\"+ New " + category.name + "\" />\n";

      dinner_selected_options[dinner_form_id].categories[category.id] = [];
  }

  document.getElementById(displayID).innerHTML = output;
}

/**
 * Makes the HTML/elements for all the meals in the given category
 * Then puts it in the page (within the ..._order_block element)
 *
 * @param {Number} dinner_form_id The ID number of the form
 * @param {Number} category_id The ID number of the category (ie. adults or kids)
 */
function refreshOrderBlock(dinner_form_id, category_id) {
    var meals = dinner_selected_options[dinner_form_id].categories[category_id];

    var order_block = document.getElementById(dinner_form_id.toString() + "_category_" + category_id.toString() + "_order_block");
    order_block.innerHTML = "";

    var htmlOutput = "";
    var i;
    for (i = 0; i < meals.length; i++) {
        order_block.appendChild(makeMealOrderElement(dinner_form_id, category_id, i));
    }

    restoreOptions(dinner_form_id, category_id);
}

/**
 * Adds a new meal to the given form and category
 *
 * @param {Number} dinner_form_id The ID number of the form
 * @param {Number} category_id The ID number of the category (ie. adults or kids)
 */
function addNewMeal(dinner_form_id, category_id) {
    saveOptions(dinner_form_id, category_id);
    dinner_selected_options[dinner_form_id].categories[category_id].push({name:""});
    refreshOrderBlock(dinner_form_id, category_id);
}

/**
 * Removes a meal from the given form and category
 *
 * @param {Number} dinner_form_id The ID number of the form
 * @param {Number} category_id The ID number of the category (ie. adults or kids)
 * @param {Number} meal_idx The index (in the array of meals) to remove at
 */
function deleteMeal(dinner_form_id, category_id, meal_idx) {
    saveOptions(dinner_form_id, category_id);
    dinner_selected_options[dinner_form_id].categories[category_id].splice(meal_idx, 1);
    refreshOrderBlock(dinner_form_id, category_id);
}

/**
 * Makes the HTML/elements for a single meal selection box (the blue coloured divs)
 *
 * @param {Number} dinner_form_id The ID number of the form
 * @param {Number} category_id The ID number of the category (ie. adults or kids)
 * @param {Number} meal_idx The index (in the array of meals) to create the box for
 * @returns {Object} The element to add to the document
 */
function makeMealOrderElement(dinner_form_id, category_id, meal_idx) {
    var category = dinner_info[dinner_form_id].config.categories.filter(function(item) {return item.id === category_id})[0];

    var divBox = document.createElement("div");
    divBox.className = "formOptionBox";
    var heading = document.createElement("h3");
    var titleSpan = document.createElement("span");
    titleSpan.innerText = category.name + " #" + (meal_idx+1).toString() + ": ";
    heading.appendChild(titleSpan)
    heading.appendChild(makeDeleteIcon(deleteMeal.bind(this, dinner_form_id, category_id, meal_idx)));
    divBox.appendChild(heading);

    var optionsDiv = document.createElement("div");

    var output = "";
    var id_str = "dinner_" + dinner_form_id.toString() + "_category_" + category_id.toString() + "_meal_" + meal_idx.toString();
    output += makeOptionHTML(id_str + "_name", {type: "text", question: "Name", placeholder: "Name..."});

    var options_array = [];

    if (dinner_info[dinner_form_id].config.courses) {
        var courses = dinner_info[dinner_form_id].config.courses.filter(function(item) {return item.category_id === category_id});
        var j;
        for (j = 0; j < courses.length; j++) {
            var menu_items = dinner_info[dinner_form_id].items.filter(function(item) {return (item.meal_category_id === category_id) && (item.course_id === courses[j].id)});
            var options = menu_items.map(function(item) {return item.name + (item.price ? (" (" + currencyToText(item.price) + ")") : "")});
            var options_ids = menu_items.map(function(item) {return item.id});
            output += makeOptionHTML(id_str + "_course_" + courses[j].id.toString(), {type: "select", question: courses[j].name, values: ["None", ...options], value_ids: [0, ...options_ids]});
        }

        if (dinner_info[dinner_form_id].config.categories) {
            var category = dinner_info[dinner_form_id].config.categories.filter(function(item) {return item.id === category_id})[0];
            if (category && category.options)
                options_array = category.options;
        }

    } else {
        options_array = dinner_info[dinner_form_id].items.filter(function(item) {return item.id === category_id})[0].options;
    }

    if (options_array && options_array.length > 0) {
        var j;
        for (j = 0; j < options_array.length; j++) {
            output += makeOptionHTML(id_str + "_option_" + options_array[j].id.toString(), options_array[j]);
        }

    }


    optionsDiv.innerHTML = output;
    divBox.appendChild(optionsDiv);

    var btn_p = document.createElement("p");
    btn_p.style.textAlign = "right";
    btn_p.appendChild(makeTextButton(deleteMeal.bind(this, dinner_form_id, category_id, meal_idx), "Remove meal"))
    divBox.appendChild(btn_p);

    return divBox;
}

function makeOptionHTML(id_str, option) {
    var output = "<div><div style=\"display: inline-block;vertical-align: middle;\"><p>" + option.question + ": </p></div>";

    if (option.type === 'select') {
        output += "<div style=\"display: inline-block;vertical-align: middle;\"><p>";
        output += "<select id=\"" + id_str + "\" name=\"" + id_str + "\">";
        var select_options = option.values;
        var i;
        for (i = 0; i < select_options.length; i++) {
            var value_id = (option.value_ids != null && option.value_ids[i] != null) ? option.value_ids[i] : select_options[i];
            output += "<option value=\"" + value_id.toString() + "\">" + select_options[i] + "</option>";
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
        output += "<div style=\"display: inline-block;vertical-align: middle;\"><p>";
        output += "<input type=\"text\" style=\"width:200px;\" id=\"" + id_str + "\" name=\"" + id_str + "\" " + (option.placeholder ? "placeholder=\"" + option.placeholder + "\"" : "") + ">";
    }
    return output + "</p></div></div>";
}

/**
 * Saves the current state of the textboxes/comboboxes/checkboxes to the dinner_selected_options variable
 * Used before "re-building" the HTML elements
 *
 * @param {Number} dinner_form_id The ID number of the form
 * @param {Number} category_id The ID number of the category (ie. adults or kids)
 */
function saveOptions(dinner_form_id, category_id) {
    var meal_count = dinner_selected_options[dinner_form_id].categories[category_id].length;

    var i;
    for (i = 0; i < meal_count; i++) {
        var meal = {};
        var id_str = "dinner_" + dinner_form_id.toString() + "_category_" + category_id.toString() + "_meal_" + i.toString();

        var name_ele = document.getElementById(id_str + "_name");
        if (name_ele)
            meal.name = name_ele.value;

        var options_array = [];

        if (dinner_info[dinner_form_id].config.courses) {

            meal.courses = {};
            var courses = dinner_info[dinner_form_id].config.courses.filter(function(item) {return item.category_id === category_id});
            var j;
            for (j = 0; j < courses.length; j++) {
                var ele = document.getElementById(id_str + "_course_" + courses[j].id.toString());
                if (ele) {
                    meal.courses[courses[j].id.toString()] = Number(ele.value);
                }
            }

            if (dinner_info[dinner_form_id].config.categories) {
                var category = dinner_info[dinner_form_id].config.categories.filter(function(item) {return item.id === category_id})[0];
                if (category && category.options)
                    options_array = category.options;
            }

        } else {
            options_array = dinner_info[dinner_form_id].items.filter(function(item) {return item.id === category_id})[0].options;
        }
        if (options_array && options_array.length > 0) {
            meal.item_options = {};
            var j;
            for (j = 0; j < options_array.length; j++) {
                if (options_array[j].type === 'select') {
                    var ele = document.getElementById(id_str + "_option_" + options_array[j].id.toString());
                    if (ele) {
                        meal.item_options[options_array[j].id.toString()] = ele.value;
                    }
                } else if (options_array[j].type === 'checkbox') {
                    var checkboxes = [];
                    var k;
                    for (k = 0; k < options_array[j].values.length; k++) {
                        var cb = document.getElementById(id_str + "_option_" + options_array[j].id.toString() + "_value_" + k.toString());
                        if (cb) {
                            checkboxes.push(cb.checked);
                        } else {
                            checkboxes.push(false);
                        }
                    }
                    meal.item_options[options_array[j].id.toString()] = checkboxes;
                } else { // text
                    var text_ele = document.getElementById(id_str + "_option_" + options_array[j].id.toString());
                    if (text_ele) {
                        meal.item_options[options_array[j].id.toString()] = text_ele.value;
                    }
                }
            }

        }

        dinner_selected_options[dinner_form_id].categories[category_id][i] = meal;
    }

}

/**
 * Restores the state of the textboxes/comboboxes/checkboxes from the dinner_selected_options variable
 * Used after "re-building" the HTML elements
 *
 * @param {Number} dinner_form_id The ID number of the form
 * @param {Number} category_id The ID number of the category (ie. adults or kids)
 */
function restoreOptions(dinner_form_id, category_id) {
    var meal_count = dinner_selected_options[dinner_form_id].categories[category_id].length;

    var i;
    for (i = 0; i < meal_count; i++) {
        var meal = dinner_selected_options[dinner_form_id].categories[category_id][i];
        var id_str = "dinner_" + dinner_form_id.toString() + "_category_" + category_id.toString() + "_meal_" + i.toString();

        var name_ele = document.getElementById(id_str + "_name");
        if (name_ele)
            name_ele.value = meal.name;


        if (meal.courses) {
            var courses = dinner_info[dinner_form_id].config.courses.filter(function(item) {return item.category_id === category_id});
            var j;
            for (j = 0; j < courses.length; j++) {
                var ele = document.getElementById(id_str + "_course_" + courses[j].id.toString());
                if (ele && meal.courses[courses[j].id.toString()] != null) {
                    ele.value = meal.courses[courses[j].id.toString()];
                }
            }
        }

        var options_array = [];

        if (dinner_info[dinner_form_id].config.categories) {
            var category = dinner_info[dinner_form_id].config.categories.filter(function(item) {return item.id === category_id})[0];
            if (category && category.options)
                options_array = category.options;
        } else {
            options_array = dinner_info[dinner_form_id].items.filter(function(item) {return item.id === category_id})[0].options;
        }

        if (meal.item_options && options_array && options_array.length > 0) {

            var j;
            for (j = 0; j < options_array.length; j++) {
                if (options_array[j].type === 'select') {
                    var ele = document.getElementById(id_str + "_option_" + options_array[j].id.toString());
                    if (ele && meal.item_options[options_array[j].id.toString()]) {
                        ele.value = meal.item_options[options_array[j].id.toString()];
                    }
                } else if (options_array[j].type === 'checkbox') {
                    var cb_values = meal.item_options[options_array[j].id.toString()];
                    if (cb_values) {
                        var k;
                        for (k = 0; k < cb_values.length; k++) {
                            var cb = document.getElementById(id_str + "_option_" + options_array[j].id.toString() + "_value_" + k.toString());
                            if (cb) {
                                cb.checked = cb_values[k];
                            }
                        }
                    }
                } else {
                    var text_ele = document.getElementById(id_str + "_option_" + options_array[j].id.toString());
                    if (text_ele && meal.item_options[options_array[j].id.toString()]) {
                        text_ele.value = meal.item_options[options_array[j].id.toString()];
                    }
                }
            }

        }

    }
}

/**
 * Calls the saveOptions function for all categories in a given form
 *
 * @param {Number} dinner_form_id The ID number of the form
 */
function saveAllDinnerOptions(dinner_form_id) {
    for (var i = 0; i < dinner_info[dinner_form_id].config.categories.length; i++) {
        var category_id = dinner_info[dinner_form_id].config.categories[i].id;
        saveOptions(dinner_form_id, category_id);
    }
}

/**
 * Calculates the total cost for the user's selections on a given form
 *
 * @param {Number} dinner_form_id The ID number of the form
 * @returns {Number} The total cost in cents
 */
function getDinnerCost(dinner_form_id) {
    var total_cost = 0;

    var has_categories = dinner_has_categories[dinner_form_id];

    for (var k = 0; k < dinner_info[dinner_form_id].config.categories.length; k++) {
        var category_id = dinner_info[dinner_form_id].config.categories[k].id;

        var meal_count = dinner_selected_options[dinner_form_id].categories[category_id].length;

        var i;
        for (i = 0; i < meal_count; i++) {
            var meal = dinner_selected_options[dinner_form_id].categories[category_id][i];
            var id_str = "dinner_" + dinner_form_id.toString() + "_category_" + category_id.toString() + "_meal_" + i.toString();

            if (meal.courses && has_categories === true) {
                var courses = dinner_info[dinner_form_id].config.courses.filter(function(item) {return item.category_id === category_id});
                var j;
                for (j = 0; j < courses.length; j++) {
                    if (meal.courses[courses[j].id.toString()] != null) {
                        var meal_id = meal.courses[courses[j].id.toString()];
                        if (meal_id) {
                            var meal_item = dinner_info[dinner_form_id].items.filter(function(item) {return (item.id === meal_id)})[0];
                            if (meal_item && meal_item.price)
                                total_cost += meal_item.price;
                        }
                    }
                }
            }

            if (meal.item_options && has_categories === false) {
                var item = dinner_info[dinner_form_id].items.filter(function(item) {return item.id === category_id})[0];
                if (item && item.price)
                    total_cost += item.price;
            }

        }
    }

    return total_cost;
}

/**
 * Creates the sub-text to put on the invoice for the user's selections on a given form
 *
 * @param {Number} dinner_form_id The ID number of the form
 * @returns {String} The sub-text
 */
function getDinnerSubtext(dinner_form_id) {
    var text = "";

    var started = false;
    for (var k = 0; k < dinner_info[dinner_form_id].config.categories.length; k++) {
        var category = dinner_info[dinner_form_id].config.categories[k];
        var meal_count = dinner_selected_options[dinner_form_id].categories[category.id].length;

        if (meal_count > 0) {
            if (started)
                text += ", ";

            var name_plural = (category.name_plural ? category.name_plural : category.name);
            text += meal_count.toString() + " " + (meal_count == 1 ? category.name : name_plural);

            started = true;
        }

    }
    return text;
}
