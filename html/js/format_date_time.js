/**
  @file    format_date_time.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This shows date/time in the user's timezome
  Eg:
  <span class="date_time" data-value="1678769035"></span>
  becomes:
  <span class="date_time" data-value="1678769035">Tue Mar 14 2023 15:43:55</span>

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
function format_date_time_numberToOrdinal(number) {
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

function formatDateTime() {
    var elements = document.getElementsByClassName('date_time');
    var i;
    for (i = 0; i < elements.length; i++) {
        var unix_time = parseInt(elements[i].dataset.value);
        var date = new Date(unix_time * 1000);
        elements[i].innerText = date.toDateString() + ' ' + date.toTimeString().substring(0,8);
    }

    var date_elements = document.getElementsByClassName('date_only');
    for (i = 0; i < date_elements.length; i++) {
        var unix_time = parseInt(date_elements[i].dataset.value);
        var date = new Date(unix_time * 1000);
        var display_options = { weekday: 'long', month: 'long', day: 'numeric' };
        date_elements[i].innerText = date.toLocaleDateString("en-US", display_options);
    }

    var days = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];
    var months = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'];
    var date_elements = document.getElementsByClassName('date_time_simple');
    for (i = 0; i < date_elements.length; i++) {
        var unix_time = parseInt(date_elements[i].dataset.value);
        var date = new Date(unix_time * 1000);
        var text = days[date.getDay()] + ', ' + months[date.getMonth()] + ' ' + format_date_time_numberToOrdinal(date.getDate());
        text += ' at ' + (((date.getHours() + 11) % 12) + 1).toString() + ':' + (date.getMinutes() < 10 ? '0' : '') + date.getMinutes() + (date.getHours() < 12 ? 'am' : 'pm');
        date_elements[i].innerText = text;
    }
}

formatDateTime();
