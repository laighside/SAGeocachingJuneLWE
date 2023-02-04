/**
  @file    page_tab_tools.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Functions for show/hide page tabs (used on admin side of website)

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

function openPageTab(e, tabName) {
    // hide all tabs and make their links as not current
    var i, tabcontent, tablinks;
    tabcontent = document.getElementsByClassName("pageTabContent");
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
    }
    tablinks = document.getElementsByClassName("pageTabLinks");
    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }

    // show the selected tab
    document.getElementById(tabName).style.display = "block";
    e.currentTarget.className += " active";
}
