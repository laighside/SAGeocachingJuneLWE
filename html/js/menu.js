/**
  @file    menu.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Functions for show/hide the menu on mobile browsers

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

function showMenu(id) {
    document.getElementById('menu_row_' + id.toString()).classList.add('show_more_menu');
    document.addEventListener('mousedown', function fn(e){hideMenu(e,id,fn)}, false);
}

function hideMenu(e,id,fn) {
    var menu_row = document.getElementById('menu_row_' + id.toString())
    if (menu_row) {
        if (menu_row.contains(e.target)) {
            return;
        }
        menu_row.classList.remove('show_more_menu');
    }
    document.removeEventListener('mousedown', fn);
}
