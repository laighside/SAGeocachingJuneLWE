/**
  @file    form_tools.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions used on multi-page forms (the ones with previous/next buttons at the bottom of the page)

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

/**
 * Works out if the user has selected one of the options within a radio-button collection
 *
 * @param {String} name The name of the radio-buttons
 * @returns {Boolean} True if the user has selected an option, false if no options are selected
 */
function isRadioChecked(name) {
    var chx = document.getElementsByName(name);
    for (var i = 0; i < chx.length; i++) {
        // Return true from the function on first match of a checked item
        if (chx[i].checked) {
            return true;
        }
    }
    return false;
}

/**
 * Sets the class for all the radio-buttons with a given name
 *
 * @param {String} radioName The name of the radio-buttons
 * @param {String} newClass The class to apply to the radio-buttons
 */
function setRadioClass(radioName, newClass) {
    var chx = document.getElementsByName(radioName);
    for (var i = 0; i < chx.length; i++) {
        chx[i].className = newClass;
    }
}

/**
 * Adds a message to the box above the previous/next buttons
 * Used for showing an error in the user's input
 *
 * @param {String} title The title of the message (shown in bold)
 * @param {String} message The message to display
 */
function addFormMessage(title, message) {
    var list = document.getElementById("formMessages");
    var li = document.createElement("li");
    li.innerHTML = "<span>" + title + "</span> " + message;
    list.appendChild(li);
    list.style.display = "block";
}

/**
 * Removes all the messages from the box above the previous/next buttons
 */
function removeFormMessages() {
    var list = document.getElementById("formMessages");
    while (list.firstChild) {
        list.removeChild(list.firstChild);
    }
    list.style.display = "none";
}

/**
 * Sets the color of the message box above the previous/next buttons
 * Usually red for errors and green for ok
 *
 * @param {String} newColor Either 'red' or 'green'
 */
function setFormMessagesColor(newColor) {
    var listStyle = document.getElementById("formMessages").style;
    if (newColor == "red") {
        listStyle.color = "#C04040";
        listStyle.backgroundColor = "#FFDDDD";
        listStyle.borderColor = "#DC9090";
    }
    if (newColor == "green") {
        listStyle.color = "#307010";
        listStyle.backgroundColor = "#D0FFC0";
        listStyle.borderColor = "#50B030";
    }
}

/**
 * Displays a given page of the form
 *
 * @param {Integer} n The index number of the page to display
 */
function showTab(n) {
    var tabs = document.getElementsByClassName("formTab");
    tabs[n].style.display = "block";
    // Only show the 'Previous' button if not on the first page
    if (n == 0) {
        document.getElementById("prevBtn").style.display = "none";
    } else {
        document.getElementById("prevBtn").style.display = "inline";
    }
    // Change the next button to submit if on the last page
    if (n == summary_page) {
        document.getElementById("nextBtn").setAttribute("value", "Submit");
    } else {
        document.getElementById("nextBtn").setAttribute("value", "Next");
    }
}

/**
 * Moves the form to either the next or previous page
 * This is called from the previous/next buttons
 *
 * @param {Integer} n n=1 for next page, n=-1 for previous page
 */
function nextPrev(n) {
    var tabs = document.getElementsByClassName("formTab");

    // Exit the function if any field in the current tab is invalid:
    if (typeof validateForm === "function") {
        if (n == 1 && !validateForm()) return false;
    }

    if (typeof leavingTab === "function" && n > 0) {
        leavingTab(currentTab);
    }

    // if you have reached the end of the form then submit it
    if ((currentTab + n) >= tabs.length) {
        if (typeof submitForm === "function") {
            submitForm();
        }
    } else {
        // Hide the current tab:
        tabs[currentTab].style.display = "none";
        // Increase or decrease the current tab by 1
        currentTab = currentTab + n;

        // skip tabs if required
        if (typeof skipTabs === "function") {
            skipTabs(n);
        }

        // Update summary if needed
        if (typeof updateSummary === "function") {
            if (currentTab == summary_page) {
                updateSummary();
            }
        }

        // scroll back to top of page if 'next' was clicked
        if (n == 1)
            window.scrollTo(0, 0);

        // Display the new page
        showTab(currentTab);
    }
}
