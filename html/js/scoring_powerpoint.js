/**
  @file    scoring.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This is all the JS for the powerpoint tab on the page /cgi-bin/scoring/scoring.cgi

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

/**
 * Converts a number to string with correct suffix, eg. 1st, 2nd, 3rd, etc.
 * Negative numbers have no suffix.
 *
 * @param {Number} number The number to convert to string
 * @returns {String} The number as a string plus the ordinal suffix
 */
function numberToOrdinal(number) {
  if (number < 0)
      return number.toString();

  var suffix = "th";
  if (number % 100 < 11 || number % 100 > 13) {
    switch (number % 10) {
      case 1:
        suffix = "st";
        break;
      case 2:
        suffix = "nd";
        break;
      case 3:
        suffix = "rd";
        break;
    }
  }
  return number.toString() + suffix;
}

/**
 * Called when the powerpoint tab is opened
 * This downloads the list of slides and places them on the page
 */
function openPowerpointTab() {
    var powerpoint_slides = document.getElementById("powerpoint_slides");
    powerpoint_slides.innerHTML = "<p style=\"text-align:center;font-style:italic;\">Loading...</p>";

    downloadUrl('get_slides.cgi', null,
                function(data, responseCode) {
                    var jsonObj = JSON.parse(data);
                    powerpoint_slides.innerHTML = "";

                    for (var i = 0; i < jsonObj.length; i++) {
                        var jsonSlide = jsonObj[i];
                        var slide = makeSlideHtml(jsonSlide);
                        powerpoint_slides.appendChild(slide);
                    }

             }, httpErrorResponseHandler);
}

/**
 * This makes the HTML elements for a single slide
 *
 * @param {Object} jsonSlide The title, id, enabled status and data for the slide
 */
function makeSlideHtml(jsonSlide) {
    var slide = document.createElement("div");
    slide.id = "slide_" + jsonSlide.id.toString();
    slide.dataset.slideId = jsonSlide.id;
    slide.classList.add("powerpoint_slide");
    if (!jsonSlide.enabled)
        slide.classList.add("powerpoint_slide_disabled");
    slide.draggable = document.getElementById("slideReorderToggleCB").checked;
    slide.addEventListener('dragstart', dragStartSlide);
    slide.addEventListener('drop', droppedSlide);
    slide.addEventListener('dragenter', cancelDefault);
    slide.addEventListener('dragover', cancelDefault);
    var titleBox = document.createElement("div");
    titleBox.id = "slide_title_" + jsonSlide.id.toString();
    titleBox.dataset.title = jsonSlide.title;
    titleBox.style.position = "relative";
    slide.appendChild(titleBox);
    var heading = document.createElement("h3");
    heading.id = "slide_title_block_set_" + jsonSlide.id.toString();
    var titleSpan = document.createElement("span");
    titleSpan.id = "slide_title_set_" + jsonSlide.id.toString();
    titleSpan.innerText = jsonSlide.title;
    heading.appendChild(titleSpan)
    titleBox.appendChild(heading);
    if (jsonSlide.type === "generic") {
        heading.appendChild(makeEditIcon(editSlideContent.bind(this, jsonSlide.id)));
        var titleEditBlock = makeLineEditBlock("slide_title_input_" + jsonSlide.id.toString(), saveSlideContent.bind(this, jsonSlide.id));
        titleEditBlock.id = "slide_title_block_edit_" + jsonSlide.id.toString();
        titleBox.appendChild(titleEditBlock);
        titleBox.dataset.content = jsonSlide.data;
    }

    if (jsonSlide.type !== "winner" && jsonSlide.type !== "runnerup" && jsonSlide.type !== "naga" && jsonSlide.type !== "scores") {
        var enabledCheckbox = makeCheckboxElement("slide_enabled_" + jsonSlide.id.toString(), "Enabled", jsonSlide.enabled, function(){slideEnabledChanged(jsonSlide.id)});
        slide.appendChild(enabledCheckbox);
    }

    if (jsonSlide.type === "winner" || jsonSlide.type === "runnerup" || jsonSlide.type === "naga") {
        if (jsonSlide.data)
            slide.appendChild(makeTeamScoreElement(jsonSlide.data));
    }
    if (jsonSlide.type === "scores" || jsonSlide.type === "disqualified") {
        for (var i = 0; i < jsonSlide.data.length; i++) {
            slide.appendChild(makeTeamScoreElement(jsonSlide.data[i]));
        }
    }
    if (jsonSlide.type === "best_caches") {
        if (jsonSlide.data)
            slide.appendChild(makeBestCachesTable(jsonSlide.data));
    }
    if (jsonSlide.type === "generic") {
        var paragraphBlock = document.createElement("div");
        paragraphBlock.id = "slide_content_block_set_" + jsonSlide.id.toString();
        var paragraph = document.createElement("p");
        paragraph.id = "slide_content_set_" + jsonSlide.id.toString();
        paragraph.innerText = jsonSlide.data;
        paragraphBlock.appendChild(paragraph);
        var editButton = document.createElement("input");
        editButton.type = "button";
        editButton.value = "Edit";
        editButton.addEventListener('click', editSlideContent.bind(this, jsonSlide.id));
        paragraphBlock.appendChild(editButton);
        slide.appendChild(paragraphBlock);

        var textEditBlock = document.createElement("div");
        textEditBlock.id = "slide_content_block_edit_" + jsonSlide.id.toString();
        textEditBlock.style.display = "none";
        var textEdit = document.createElement("textarea");
        textEdit.id = "slide_content_input_" + jsonSlide.id.toString();
        textEditBlock.appendChild(textEdit);
        var saveButton = document.createElement("input");
        saveButton.type = "button";
        saveButton.value = "Save";
        saveButton.addEventListener('click', saveSlideContent.bind(this, jsonSlide.id));
        textEditBlock.appendChild(saveButton);
        slide.appendChild(textEditBlock);
    }

    return slide;
}

/**
 * This makes the HTML elements for a single team score (to be placed within a list of scores)
 *
 * @param {Object} team_score The position, name and score for the team
 */
function makeTeamScoreElement(team_score) {
    // When score = -100, this represents a disqualified team
    var isDisqualified = false;
    if (team_score.score <= -1000)
        isDisqualified = true;

    var paragraph = document.createElement("p");
    if (!isDisqualified) {
        var span1 = document.createElement("span");
        span1.innerText = numberToOrdinal(team_score.position) + " place: ";
        paragraph.appendChild(span1);
    }
    var span2 = document.createElement("span");
    span2.innerText = team_score.team_name;
    paragraph.appendChild(span2);
    var span3 = document.createElement("span");
    if (isDisqualified) {
        span3.innerText = " (DSQ/DNF)";
    } else {
        span3.innerText = " (" + (team_score.score / 10).toString() + " points)";
    }
    paragraph.appendChild(span3);
    return paragraph;
}

/**
 * This makes the HTML elements for the best caches table
 *
 * @param {Array} jsonData The list of best caches
 */
function makeBestCachesTable(jsonData) {
    var parent = document.createElement("div");
    var table = document.createElement("table");
    table.align = "center";
    parent.appendChild(table);
    var headerRow = document.createElement("tr");
    headerRow.innerHTML = "<th>Award</th><th style=\"min-width:300px;\" >Winning Cache</th>";
    table.appendChild(headerRow);

    for (var i = 0; i < jsonData.length; i++) {
        var row = document.createElement("tr");
        var titleCell = document.createElement("td");
        titleCell.innerText = jsonData[i].title;
        row.appendChild(titleCell);
        var valueCell = document.createElement("td");
        valueCell.style = "position:relative;";
        var valueCellSet = document.createElement("div");
        valueCellSet.id = "best_cache_set_" + jsonData[i].id.toString();
        valueCellSet.dataset.value = jsonData[i].award_value;
        var valueSpan = document.createElement("span");
        valueSpan.id = "best_cache_value_span_" + jsonData[i].id.toString();
        valueSpan.innerText = jsonData[i].award_value;
        valueCellSet.appendChild(valueSpan);
        valueCellSet.appendChild(makeEditIcon(editBestCache.bind(this, jsonData[i].id)));
        valueCell.appendChild(valueCellSet);

        var valueCellEdit = makeLineEditBlock("best_cache_input_" + jsonData[i].id.toString(), saveBestCache.bind(this, jsonData[i].id));
        valueCellEdit.id = "best_cache_edit_" + jsonData[i].id.toString();
        valueCell.appendChild(valueCellEdit);

        row.appendChild(valueCell);
        table.appendChild(row);
    }

    var noteParagraph = document.createElement("p");
    noteParagraph.innerText = "Don't just write \"Number 91\", go outside, have a look at what cache 91 is and then write \"91 - Giant Anchor\". This makes the presentation a lot less confusing for the audience!";
    parent.appendChild(noteParagraph);

    return parent;
}

/**
 * This is called when a slide enabled checkbox is clicked
 *
 * @param {Number} slide_id The ID number of the slide the enabled is being changed for
 */
function slideEnabledChanged(slide_id) {
    var checkbox = document.getElementById("slide_enabled_" + slide_id.toString());
    var slide = document.getElementById("slide_" + slide_id.toString());

    // save old state in case update request fails
    var oldState = !slide.classList.contains("powerpoint_slide_disabled");

    // Update the background color
    if (checkbox.checked) {
        slide.classList.remove("powerpoint_slide_disabled");
    } else {
        slide.classList.add("powerpoint_slide_disabled");
    }

    // HTTP request to save the new enabled state
    var jsonObj = {
        "slide_id":slide_id,
        "enabled":checkbox.checked
    };
    postUrl('set_slide.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, null, function() {
                    // if it fails, revert the checkbox to the old state
                    checkbox.checked = oldState;
                    if (checkbox.checked) {
                        slide.classList.remove("powerpoint_slide_disabled");
                    } else {
                        slide.classList.add("powerpoint_slide_disabled");
                    }
                });
         }, httpErrorResponseHandler);
}

/**
 * These are the functions for the drag and drop reordering of slides
 */
function dragStartSlide (e) {
    if (document.getElementById("slideReorderToggleCB").checked &&
            e.target.dataset &&
            e.target.dataset.slideId) {
        var index = $(e.target).index();
        e.dataTransfer.setData('slide_index', index);
    } else {
        e.dataTransfer.setData('slide_index', null);
    }
}
function droppedSlide (e) {
    // get old index
    let oldIndex = parseInt(e.dataTransfer.getData('slide_index'))
    if (!oldIndex && oldIndex !== 0)
        return;

    // Find the slide element (not the stuff within it)
    var slideElement = e.target;
    while (!slideElement.dataset.slideId) {
        slideElement = slideElement.parentElement;

        // reached the root element without finding the slide
        // ie. it was dropped somewhere else that isn't in/on a slide
        if (!slideElement)
            return;
    }

    cancelDefault(e);

    // get the new index
    let newIndex = $(slideElement).index();

    // remove dropped items at old position
    var powerpoint_slides = document.getElementById("powerpoint_slides");
    let dropped = powerpoint_slides.removeChild(powerpoint_slides.children[oldIndex]);

    // insert it at the new position
    powerpoint_slides.insertBefore(dropped, powerpoint_slides.children[newIndex]);

    // save the new order
    saveSlideOrder();
}
function cancelDefault (e) {
    e.preventDefault();
    e.stopPropagation();
    return false;
}

/**
 * Starts the editing of a best cache winner, called when the user clicks a edit button
 * This just shows the text input element
 *
 * @param {Number} award_id The ID number of the award to edit
 */
function editBestCache(award_id) {
    document.getElementById("best_cache_set_" + award_id.toString()).style.display = "none";
    document.getElementById("best_cache_edit_" + award_id.toString()).style.display = "block";

    var dataElement = document.getElementById("best_cache_set_" + award_id.toString());
    document.getElementById("best_cache_input_" + award_id.toString()).value = dataElement.dataset.value;
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

                    document.getElementById("best_cache_set_" + award_id.toString()).style.display = "block";
                    document.getElementById("best_cache_edit_" + award_id.toString()).style.display = "none";
                    var dataElement = document.getElementById("best_cache_set_" + award_id.toString());
                    dataElement.dataset.value = newValue;
                    document.getElementById("best_cache_value_span_" + award_id.toString()).innerText = newValue;

                }, null);
         }, httpErrorResponseHandler);
}

/**
 * Starts the editing of a slide content, called when the user clicks a edit button
 * This just shows the text input elements for the title and content
 *
 * @param {Number} slide_id The ID number of the slide to edit
 */
function editSlideContent(slide_id) {
    document.getElementById("slide_title_block_set_" + slide_id.toString()).style.display = "none";
    document.getElementById("slide_title_block_edit_" + slide_id.toString()).style.display = "block";
    document.getElementById("slide_content_block_set_" + slide_id.toString()).style.display = "none";
    document.getElementById("slide_content_block_edit_" + slide_id.toString()).style.display = "block";

    var dataElement = document.getElementById("slide_title_" + slide_id.toString());
    document.getElementById("slide_title_input_" + slide_id.toString()).value = dataElement.dataset.title;
    document.getElementById("slide_content_input_" + slide_id.toString()).value = dataElement.dataset.content;
}

/**
 * Saves a edited slide content, called when the user clicks a save button
 * This sends the new content to the server and removes the input element on success
 *
 * @param {Number} slide_id The ID number of the slide being edited
 */
function saveSlideContent(slide_id) {
    var newTitleValue = document.getElementById("slide_title_input_" + slide_id.toString()).value;
    var newContentValue = document.getElementById("slide_content_input_" + slide_id.toString()).value;

    var jsonObj = {
        "slide_id":slide_id,
        "title":newTitleValue,
        "content": newContentValue,
    };
    postUrl('set_slide.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, function(){

                    document.getElementById("slide_title_block_set_" + slide_id.toString()).style.display = "block";
                    document.getElementById("slide_title_block_edit_" + slide_id.toString()).style.display = "none";
                    document.getElementById("slide_content_block_set_" + slide_id.toString()).style.display = "block";
                    document.getElementById("slide_content_block_edit_" + slide_id.toString()).style.display = "none";
                    var dataElement = document.getElementById("slide_title_" + slide_id.toString());
                    dataElement.dataset.title = newTitleValue;
                    dataElement.dataset.content = newContentValue;
                    document.getElementById("slide_title_set_" + slide_id.toString()).innerText = newTitleValue;
                    document.getElementById("slide_content_set_" + slide_id.toString()).innerText = newContentValue;

                }, null);
         }, httpErrorResponseHandler);
}

/**
 * This is called when the "Enable slide reordering" toggle switch is changed
 * It sets the draggable state as required
 */
function slideReorderChanged() {
    var reorderEnabled = document.getElementById("slideReorderToggleCB").checked;
    var powerpoint_slides = document.getElementById("powerpoint_slides");

    for (var i = 0; i < powerpoint_slides.children.length; i++) {
        powerpoint_slides.children[i].draggable = reorderEnabled;
    }
}

/**
 * Saves the current order of the slides
 * This sends the list to the server
 */
function saveSlideOrder() {
    var idList = [];
    var powerpoint_slides = document.getElementById("powerpoint_slides");

    for (var i = 0; i < powerpoint_slides.children.length; i++) {
        idList.push(parseInt(powerpoint_slides.children[i].dataset.slideId));
    }

    var jsonObj = {
        "order":idList
    };
    postUrl('set_slide.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, true, null, null);
         }, httpErrorResponseHandler);
}
