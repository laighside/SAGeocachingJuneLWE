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
}

formatDateTime();
