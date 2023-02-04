/**
  @file    uuid.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This generates a random UUID value (UUID version 4)

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

function uuid () {
    function getRandomSymbol (symbol) {
        var array;

        if (symbol === 'y') {
            array = ['8', '9', 'a', 'b'];
            return array[Math.floor(Math.random() * array.length)];
        }

        array = new Uint8Array(1);
        var crypto = window.crypto || window.msCrypto;
        crypto.getRandomValues(array);
        return (array[0] % 16).toString(16);
    }

    return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, getRandomSymbol);
}
