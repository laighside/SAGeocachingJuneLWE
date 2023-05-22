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
 * Makes a button with a edit icon
 *
 * @param {Function} onClick Function to call when the button is clicked
 * @returns {Object} The element to add to the document
 */
function makeEditIcon(onClick) {
    return makeIconButton(onClick, "/img/edit.svg");
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
 * Makes a button out of an icon image
 *
 * @param {Function} onClick Function to call when the button is clicked
 * @param {String} imageUrl URL to the SVG image to use as the button
 * @returns {Object} The element to add to the document
 */
function makeIconButton(onClick, imageUrl) {
    var editIcon = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
    editIcon.classList.add("iconButton");
    editIcon.style.float = "right";
    editIcon.setAttribute("width", "20");
    editIcon.setAttribute("height", "20");
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
