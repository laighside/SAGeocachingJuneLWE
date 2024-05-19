/**
  @file    form_elements.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This makes commonly used things like checkboxes, icons, etc.

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

/**
 * Makes a button with a delete icon
 *
 * @param {Function} onClick Function to call when the button is clicked
 * @returns {Object} The element to add to the document
 */
function makeDeleteIcon(onClick) {
    return makeIconButton(onClick, "/img/delete.svg", "Delete");
}

/**
 * Makes a button with a edit icon
 *
 * @param {Function} onClick Function to call when the button is clicked
 * @returns {Object} The element to add to the document
 */
function makeEditIcon(onClick) {
    return makeIconButton(onClick, "/img/edit.svg", "Edit");
}

/**
 * Makes a button with a tick icon
 *
 * @param {Function} onClick Function to call when the button is clicked
 * @returns {Object} The element to add to the document
 */
function makeTickIcon(onClick) {
    return makeIconButton(onClick, "/img/tick.svg");
}

/**
 * Makes a button with a expand down icon
 *
 * @param {Function} onClick Function to call when the button is clicked
 * @returns {Object} The element to add to the document
 */
function makeExpandDownIcon(onClick) {
    return makeIconButton(onClick, "/img/expand_circle_down.svg", "Expand");
}

/**
 * Makes a button with a expand up icon
 *
 * @param {Function} onClick Function to call when the button is clicked
 * @returns {Object} The element to add to the document
 */
function makeExpandUpIcon(onClick) {
    return makeIconButton(onClick, "/img/expand_circle_up.svg");
}

/**
 * Makes a button out of an icon image
 *
 * @param {Function} onClick Function to call when the button is clicked
 * @param {String} imageUrl URL to the SVG image to use as the button
 * @param {String} altText Alt text for the image element
 * @returns {Object} The element to add to the document
 */
function makeIconButton(onClick, imageUrl, altText) {
    var editIcon = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
    editIcon.classList.add("iconButton");
    editIcon.style.float = "right";
    editIcon.setAttribute("width", "20");
    editIcon.setAttribute("height", "20");
    if (altText) {
        var titleElement = document.createElementNS("http://www.w3.org/2000/svg", "title");
        titleElement.textContent = altText;
        editIcon.appendChild(titleElement);
    }
    var imageElement = document.createElementNS("http://www.w3.org/2000/svg", "image");
    if (imageUrl)
        imageElement.setAttributeNS("http://www.w3.org/1999/xlink", "href", imageUrl);
    imageElement.setAttribute("width", "20");
    imageElement.setAttribute("height", "20");
    editIcon.appendChild(imageElement);
    if (onClick)
        editIcon.addEventListener('click', onClick);
    return editIcon;
}

/**
 * Convenience function that creates a HTML element: <type id=id>text</type>
 *
 * @param {String} type The type of element
 * @param {String} text The innerText for the element
 * @param {String} id The id for the element
 * @returns {Object} The element
 */
function makeHtmlElement(type, text, id) {
    var element = document.createElement(type);
    if (id)
        element.id = id;
    if (text)
        element.innerText = text;
    return element;
}

/**
 * Makes a single line text input with tick icon save button on the right
 *
 * @param {String} inputId The id of the text input element
 * @param {Function} onSave Function to call when the save button is clicked
 * @returns {Object} The element to add to the document
 */
function makeLineEditBlock(inputId, onSave) {
    var lineEditContainer = document.createElement("div");
    lineEditContainer.style.display = "none";
    var inputBox = document.createElement("input");
    inputBox.id = inputId;
    inputBox.type = "text";
    inputBox.style.width = "100%";
    lineEditContainer.appendChild(inputBox);
    var saveIcon = makeTickIcon(onSave);
    saveIcon.classList.add("textboxTick");
    lineEditContainer.appendChild(saveIcon);
    return lineEditContainer;
}

/**
 * Makes a number input with tick icon save button on the right
 *
 * @param {String} inputId The id of the text input element
 * @param {Function} onSave Function to call when the save button is clicked
 * @param {String} inputStep The value for the step attribute of the input element
 * @returns {Object} The element to add to the document
 */
function makeNumberEditBlock(inputId, onSave, inputStep) {
    var numberEditContainer = document.createElement("div");
    numberEditContainer.style.display = "none";
    var inputBox = document.createElement("input");
    inputBox.id = inputId;
    inputBox.type = "number";
    if (inputStep)
        inputBox.step = inputStep;
    inputBox.style.width = "100px";
    numberEditContainer.appendChild(inputBox);
    var saveIcon = makeTickIcon(onSave);
    saveIcon.style.float = "none";
    numberEditContainer.appendChild(saveIcon);
    return numberEditContainer;
}

/**
 * Makes a checkbox
 *
 * @param {String} id The id of the checkbox
 * @param {String} label The text label to show next to the checkbox
 * @param {Boolean} checked The initial state of the checkbox
 * @param {Function} onChange Function to call when the checkbox state is changed
 * @returns {Object} The element to add to the document
 */
function makeCheckboxElement(id, label, checked, onChange) {
    var checkboxContainer = document.createElement("span");
    checkboxContainer.classList.add("checkbox_container");
    var labelElement = document.createElement("label");
    checkboxContainer.appendChild(labelElement);
    var labelSpan = document.createElement("span");
    labelSpan.innerText = label;
    labelElement.appendChild(labelSpan);
    var checkboxInput = document.createElement("input");
    checkboxInput.type = "checkbox";
    checkboxInput.id = id;
    checkboxInput.name = id;
    checkboxInput.value = "true";
    checkboxInput.checked = checked;
    if (onChange)
        checkboxInput.addEventListener('change', onChange);
    labelElement.appendChild(checkboxInput);
    var checkmarkSpan = document.createElement("span");
    checkmarkSpan.classList.add("checkmark");
    labelElement.appendChild(checkmarkSpan);
    return checkboxContainer;
}

/**
 * Makes a combo-box
 *
 * @param {String} id The id of the combo-box
 * @param {Array} options List of options to show in the combo-box
 * @param {Function} onChange Function to call when the combo-box state is changed
 * @returns {Object} The element to add to the document
 */
function makeComboBoxElement(id, options, onChange) {
    var selectElement = document.createElement("select");
    selectElement.id = id;
    selectElement.name = id;
    for (var i = 0; i < options.length; i++) {
        var selectOption = document.createElement("option");
        selectOption.innerText = options[i].display_text;
        selectOption.value = options[i].value;
        if (options[i].selected)
            selectOption.selected = "true";
        selectElement.appendChild(selectOption);
    }
    if (onChange)
        selectElement.addEventListener('change', onChange);
    return selectElement;
}

/**
 * Makes a table cell with a span to dispaly value
 * Used to make part of an editable cell
 *
 * @param {String} initialValue The inital value to display
 * @param {String} idStr The id of element(s)
 */
function makeEditableCell(initialValue, idStr) {
    var valueCell = document.createElement("td");
    valueCell.style = "position:relative;";
    var valueCellSet = document.createElement("div");
    valueCellSet.id = "table_cell_set_" + idStr;
    valueCellSet.dataset.value = initialValue;
    var valueSpan = document.createElement("span");
    valueSpan.id = "table_cell_value_span_" + idStr;
    valueSpan.innerText = initialValue;
    valueCellSet.appendChild(valueSpan);
    valueCellSet.appendChild(makeEditIcon(startEditCell.bind(this, idStr)));
    valueCell.appendChild(valueCellSet);

    return valueCell;
}

/**
 * Makes a table cell with a single line text input with tick icon save button on the right
 *
 * @param {String} initialText The initial value to display
 * @param {String} idStr The id of element(s)
 * @param {Function} saveFunction Function to call to save a new value
 */
function makeEditableTextCell(initialText, idStr, saveFunction) {
    var valueCell = makeEditableCell(initialText, idStr);

    var valueCellEdit = makeLineEditBlock("table_cell_input_" + idStr, saveTextEdit.bind(this, idStr, saveFunction));
    valueCellEdit.id = "table_cell_edit_" + idStr;
    valueCell.appendChild(valueCellEdit);

    return valueCell;
}

/**
 * Makes a table cell with a number input with tick icon save button on the right
 *
 * @param {Number} initialValue The initial value to display
 * @param {String} idStr The id of element(s)
 * @param {Function} saveFunction Function to call to save a new value
 * @param {Boolean} intOnly Set to true to only accept integer input
 * @param {String} inputStep The value for the step attribute of the input element
 */
function makeEditableNumberCell(initialValue, idStr, saveFunction, intOnly, inputStep) {
    var valueCell = makeEditableCell(initialValue ? initialValue.toString() : 0, idStr);

    var valueCellEdit = makeNumberEditBlock("table_cell_input_" + idStr, saveNumberEdit.bind(this, idStr, saveFunction, intOnly), inputStep);
    valueCellEdit.id = "table_cell_edit_" + idStr;
    valueCell.appendChild(valueCellEdit);

    return valueCell;
}

/**
 * Starts the editing of a input field, called when the user clicks a edit button
 * This just shows the input element
 *
 * @param {String} idStr The ID (suffix) of the text edit cell
 */
function startEditCell(idStr) {
    document.getElementById("table_cell_set_" + idStr).style.display = "none";
    document.getElementById("table_cell_edit_" + idStr).style.display = "block";

    var dataElement = document.getElementById("table_cell_set_" + idStr);
    document.getElementById("table_cell_input_" + idStr).value = dataElement.dataset.value;
}

/**
 * Saves the editing of a text input field, called when the user clicks a save button
 *
 * @param {String} idStr The ID (suffix) of the text edit cell
 */
function saveTextEdit(idStr, saveFunction) {
    var newValue = document.getElementById("table_cell_input_" + idStr).value;
    saveFunction(newValue, saveEditSuccess.bind(this, idStr));
}

/**
 * Saves the editing of a number input field, called when the user clicks a save button
 *
 * @param {String} idStr The ID (suffix) of the text edit cell
 */
function saveNumberEdit(idStr, saveFunction, intOnly) {
    var inputElement = document.getElementById("table_cell_input_" + idStr);
    var newValue;
    if (intOnly) {
        newValue = parseInt(inputElement.value);
    } else {
        newValue = parseFloat(inputElement.value);
    }
    inputElement.value = newValue;
    if (saveFunction) {
        saveFunction(newValue, saveEditSuccess.bind(this, idStr));
    } else {
        saveEditSuccess(idStr);
    }
}

/**
 * Removes the input field from view, called after changes have been saved successfully
 *
 * @param {String} idStr The ID (suffix) of the text edit cell
 */
function saveEditSuccess(idStr) {
    document.getElementById("table_cell_set_" + idStr).style.display = "block";
    document.getElementById("table_cell_edit_" + idStr).style.display = "none";

    var dataElement = document.getElementById("table_cell_set_" + idStr);
    var newValue = document.getElementById("table_cell_input_" + idStr).value;
    dataElement.dataset.value = newValue;
    document.getElementById("table_cell_value_span_" + idStr).innerText = newValue;
}

/**
 * Makes a table cell with an expand button in it
 *
 * @param {String} cellValue The value to display in the cell
 * @param {String} idStr The id of element(s)
 * @param {Function} onChange Function to call when the expanded status is changed
 */
function makeExpandCell(cellValue, idStr, onChange) {
    var expandCell = document.createElement("td");
    expandCell.style = "position:relative;";
    var valueCellSet = document.createElement("div");
    valueCellSet.id = "table_cell_set_" + idStr;
    var valueSpan = document.createElement("span");
    valueSpan.id = "table_cell_value_span_" + idStr;
    valueSpan.innerText = cellValue;
    valueCellSet.appendChild(valueSpan);
    var expandDownIcon = makeExpandDownIcon(toggleExpandCell.bind(this, true, idStr, onChange))
    expandDownIcon.id = "expand_down_icon_" + idStr;
    valueCellSet.appendChild(expandDownIcon);
    var expandUpIcon = makeExpandUpIcon(toggleExpandCell.bind(this, false, idStr, onChange))
    expandUpIcon.id = "expand_up_icon_" + idStr;
    expandUpIcon.style.display = "none";
    valueCellSet.appendChild(expandUpIcon);
    expandCell.appendChild(valueCellSet);

    return expandCell;
}

/**
 * Called when a expand toggle is clicked, swaps the up/down arrow direction
 *
 * @param {Boolean} expandedState Is the expand option currently set to expand
 * @param {String} idStr The id (suffix) of expand button element
 * @param {Function} onChange Function to call when the expanded status is changed
 */
function toggleExpandCell(expandedState, idStr, onChange) {
    document.getElementById("expand_down_icon_" + idStr).style.display = expandedState ? "none" : "block";
    document.getElementById("expand_up_icon_" + idStr).style.display = expandedState ? "block" : "none";
    if (onChange)
        onChange(idStr, expandedState);
}

/**
 * Makes a three-dots icon with drop down menu the opens when clicked
 *
 * @param {Number} id_number A unique number to identify this menu
 * @param {Array} menuItems A list of items to display in the menu
 */
function makeDropDownMenu(id_number, menuItems) {
    var menuRoot = document.createElement("div");
    menuRoot.id = "menu_row_" + id_number.toString();
    var menuDropButton = document.createElement("button");
    menuDropButton.id = "more_button_row_" + id_number.toString();
    menuDropButton.classList.add("more_button");
    menuDropButton.addEventListener('click', showMenu.bind(this, id_number));
    menuDropButton.appendChild(document.createElement("span"));
    menuDropButton.appendChild(document.createElement("span"));
    menuDropButton.appendChild(document.createElement("span"));
    menuRoot.appendChild(menuDropButton);

    var menuDrop = document.createElement("div");
    menuDrop.classList.add("more_menu");
    var moreMenuCaret = document.createElement("div");
    moreMenuCaret.classList.add("more_menu_caret");
    var moreMenuCaretOuter = document.createElement("div");
    moreMenuCaretOuter.classList.add("more_menu_caret_outer");
    var moreMenuCaretInner = document.createElement("div");
    moreMenuCaretInner.classList.add("more_menu_caret_inner");
    moreMenuCaret.appendChild(moreMenuCaretOuter);
    moreMenuCaret.appendChild(moreMenuCaretInner);
    menuDrop.appendChild(moreMenuCaret);

    var moreMenuItems = document.createElement("ul");
    moreMenuItems.classList.add("more_menu_items");

    for (var i = 0; i < menuItems.length; i++) {
        moreMenuItems.appendChild(makeDropDownMenuItem(menuItems[i]));
    }

    menuDrop.appendChild(moreMenuItems);
    menuRoot.appendChild(menuDrop);

    return menuRoot;
}

/**
 * Makes a single menu item (to be placed within a drop down menu)
 *
 * @param {Object} menuItem The text and onClick function for the menu item.
 */
function makeDropDownMenuItem(menuItem) {
    var menuItemElement = document.createElement("li");
    var menuItemButton = document.createElement("button");
    if (menuItem.buttonId)
        menuItemButton.id = menuItem.buttonId;
    menuItemButton.classList.add("more_menu_btn");
    menuItemButton.type = "button";
    menuItemButton.innerText = menuItem.text;
    menuItemButton.addEventListener('click', menuItem.onClick);
    menuItemElement.appendChild(menuItemButton);
    return menuItemElement;
}

/**
 * This clears a table and displays a status message on it
 * Used for showing loading/error messages
 *
 * @param {String} statusText The status message to show
 * @param {String} table_id The element id of the table
 */
function showTableStatusRow(statusText, table_id) {
    var table = document.getElementById(table_id);
    while (table.rows.length > 1) {
        table.deleteRow(-1);
    }
    var loadingRow = table.insertRow(-1);
    var loadingCell = loadingRow.insertCell(0);
    loadingCell.style.textAlign = "center";
    loadingCell.style.fontStyle = "italic";
    loadingCell.colSpan = table.rows[0].cells.length;
    loadingCell.innerText = statusText;
}
