/**
  @file    utils.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions used for HTTP requests

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

/**
* Returns an XMLHttp instance to use for asynchronous
* downloading. This method will never throw an exception, but will
* return NULL if the browser does not support XmlHttp for any reason.
* @return {XMLHttpRequest|Null}
*/
function createXmlHttpRequest() {
    try {
        if (typeof ActiveXObject != 'undefined') {
            return new ActiveXObject('Microsoft.XMLHTTP');
        } else if (window["XMLHttpRequest"]) {
            return new XMLHttpRequest();
        }
    } catch (e) {
        changeStatus(e);
    }
    return null;
};

/**
 * This functions wraps XMLHttpRequest open/send function.
 * It lets you specify a URL and will call the callback if
 * it gets a status code of 200.
 * @param {String} url The URL to retrieve
 * @param {Boolean} xml Set to true to parse the response as XML
 * @param {Function} callback The function to call once retrieved.
 * @param {Function} errorCallback This function is called if the server returns an error
*/
function downloadUrl(url, xml, callback, errorCallback) {
    var status = -1;
    var request = createXmlHttpRequest();
    if (!request) {
        return false;
    }

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            try {
                status = request.status;
            } catch (e) {
                // Usually indicates request timed out in FF.
                if (errorCallback)
                    errorCallback(0, "Request Error (likey timeout)");
            }
            if (status == 200) {
                if (xml) {
                    callback(request.responseXML, request.status);
                } else {
                    callback(request.responseText, request.status);
                }
              request.onreadystatechange = function() {};
            } else {
                if (errorCallback)
                    errorCallback(request.status, request.statusText);
            }
        }
    }
    request.open('GET', url, true);
    try {
        request.send(null);
    } catch (e) {
        changeStatus(e);
    }
};

/**
 * Makes a HTTP POST request
 *
 * @param {String} url The URL to make the request to
 * @param {String} data The data to post
 * @param {Boolean} xml Set to true to parse the response as XML
 * @param {Function} callback This function is called with the response data
 * @param {Function} errorCallback This function is called if the server returns an error
 */
function postUrl(url, data, xml, callback, errorCallback) {
    var status = -1;
    var request = createXmlHttpRequest();
    if (!request) {
        if (errorCallback)
            errorCallback(0, "Unable to create XmlHttpRequest");
        return false;
    }

    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            try {
                status = request.status;
            } catch (e) {
                // Usually indicates request timed out in FF.
                if (errorCallback)
                    errorCallback(0, "Request Error (likey timeout)");
            }
            if (status == 200) {
                if (xml){
                    callback(request.responseXML, request.status);
                } else {
                    callback(request.responseText, request.status);
                }
              request.onreadystatechange = function() {};
            } else {
                if (errorCallback)
                    errorCallback(request.status, request.statusText);
            }
        }
    }
    request.open('POST', url, true);
    try {
        request.send(data);
    } catch (e) {
        changeStatus(e);
    }
};

/**
 * Looks at the JSON response from the JLWE server and displays the success message or error
 * JSON format: {success:true,message:"Success Message"}   OR   {success:false,error:"Error Message"}
 *
 * @param {String} data The response data
 * @param {Number} responseCode The HTTP code for the response
 * @param {Boolean} silentOnSuccess If set to true, no message box will be shown unless an error occurs
 * @param {Function} successCallback This function is called after a success response
 * @param {Function} errorCallback This function is called if the server returns an error
 */
function httpResponseHandler(data, responseCode, silentOnSuccess, successCallback, errorCallback) {
    if (responseCode === 200){
        var jsonObj = JSON.parse(data);
        if (jsonObj.success !== undefined){
            if (jsonObj.success === true) {
                // success
                if (silentOnSuccess === false) {
                    if (typeof jsonObj.message == 'string') {
                        alert("Success: " + jsonObj.message);
                    } else {
                        alert("Success");
                    }
                }

                if (successCallback)
                    successCallback(jsonObj);

            } else {
                // server responded with an error
                if (typeof jsonObj.error == 'string') {
                    alert("Error: " + jsonObj.error);
                } else {
                    alert("Error: Unspecified error");
                }
                if (errorCallback)
                    errorCallback();
            }
        } else {
            // Server response was not in expected format, bad JSON
            alert("Error: Invalid server response");
            if (errorCallback)
                errorCallback();
        }
    }
}

/**
 * Displays an error message box
 *
 * @param {String} statusText The error text
 * @param {Number} statusCode The HTTP code for the response
 */
function httpErrorResponseHandler(statusCode, statusText) {
    if (statusText && typeof statusText == 'string') {
        alert("HTTP error: " + statusText);
    } else {
        alert("HTTP error: Unknown");
    }
}

/**
 * Parses the given XML string and returns the parsed document in a
 * DOM data structure. This function will return an empty DOM node if
 * XML parsing is not supported in this browser.
 * @param {string} str XML string.
 * @return {Element|Document} DOM.
 */
function xmlParse(str) {
  if (typeof ActiveXObject != 'undefined' && typeof GetObject != 'undefined') {
    var doc = new ActiveXObject('Microsoft.XMLDOM');
    doc.loadXML(str);
    return doc;
  }

  if (typeof DOMParser != 'undefined') {
    return (new DOMParser()).parseFromString(str, 'text/xml');
  }

  return createElement('div', null);
}

/**
 * Appends a JavaScript file to the page.
 * @param {string} url
 */
function downloadScript(url) {
  var script = document.createElement('script');
  script.src = url;
  document.body.appendChild(script);
}
