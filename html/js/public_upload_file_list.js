/**
  @file    public_upload_file_list.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This is all the JS for the page /cgi-bin/public_upload/list_public_upload.cgi

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

var file_list = [];

var loading_svg_html = "<img src=\"/img/loading.svg\" alt=\"Loading...\" style=\"max-height:22px;\" />";

function getGDFolderList() {
    downloadUrl('gd_list_files.cgi?type=folder&sharedWithMe=true', null,
        function(data, responseCode) {
            if (responseCode === 200) {
                var jsonObj = JSON.parse(data);
                if (jsonObj.error == null) {
                    var select_element = document.getElementById("gd_folder_select");
                    select_element.innerHTML = "";
                    jsonObj.files.forEach(item => {
                        var option_ele = document.createElement("option");
                        option_ele.innerText = item.name;
                        option_ele.value = item.id;
                        select_element.appendChild(option_ele);
                    });
                    onFolderSelectChange();
                }else{
                    document.getElementById("page_note").innerHTML = 'Error: ' + jsonObj.error;
                }
            }
     });
};

function onFolderSelectChange() {
    var all_names = Array.from(document.getElementsByClassName("table_file_row")).map(item => item.dataset.filename);
    all_names.forEach(filename => {
        document.getElementById("on_gd_cell_" + filename).innerHTML = loading_svg_html;
    });

    var folder_id = document.getElementById("gd_folder_select").value;
    if (folder_id == null || folder_id.length == 0) {
        all_names.forEach(filename => {
            document.getElementById("on_gd_cell_" + filename).innerText = "-";
        });
        return;
    }

    downloadUrl('gd_list_files.cgi?type=file&parent=' + folder_id, null,
        function(data, responseCode) {
            if (responseCode === 200) {
                var jsonObj = JSON.parse(data);
                if (jsonObj.error == null) {
                    file_list = jsonObj.files;
                    fileListChanged();
                } else {
                    document.getElementById("page_note").innerHTML = 'Error: ' + jsonObj.error;
                }
            }
     });
};

function checkboxAllChanged() {
    var all_checked = document.getElementById("checkbox_all").checked;
    var all_names = Array.from(document.getElementsByClassName("table_file_row")).map(item => item.dataset.filename);
    all_names.forEach(filename => {
        document.getElementById("checkbox_" + filename).checked = all_checked;
    });
}

async function sendFileToGD(file_name) {
    document.getElementById("on_gd_cell_" + file_name).innerHTML = loading_svg_html;

    var jsonObj = {
        "filename": file_name,
        "parent": document.getElementById("gd_folder_select").value
    };
    await fetch('gd_upload_file.cgi', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(jsonObj),
    }).then(response => {
        if (!response.ok) {
            throw new Error("HTTP error: " + response.status.toString());
        }
        return response.json();
    }).then(responseData => {
        if (responseData.error == null) {
            file_list.push(responseData);
            fileListChanged();
        } else {
            alert("Error: " + responseData.error);
        }
    }).catch(error => {
        alert("Error: " + error);
    });
}

function fileListChanged() {
    var all_names = Array.from(document.getElementsByClassName("table_file_row")).map(item => item.dataset.filename);
    all_names.forEach(filename => {
        document.getElementById("on_gd_cell_" + filename).innerText = file_list.map(item => item.name).includes(filename) ? "Yes" : "No";
    });
}

function rotateAntiClock(file_name) {
    var jsonObj = {
        "filename": file_name,
        "rotate": 270
    };
    postUrl('edit_image.cgi', JSON.stringify(jsonObj), null,
        function(data, responseCode) {
            httpResponseHandler(data, responseCode, false, function(jsonResponse){
                if (jsonResponse.success) {

                }
            }, null);
        }, httpErrorResponseHandler);
}

function rotateClockwise(file_name) {
    var jsonObj = {
        "filename": file_name,
        "rotate": 90
    };
    postUrl('edit_image.cgi', JSON.stringify(jsonObj), null,
        function(data, responseCode) {
            httpResponseHandler(data, responseCode, false, function(jsonResponse){
                if (jsonResponse.success) {

                }
            }, null);
        }, httpErrorResponseHandler);
}

function deleteImage(file_name) {
    if (confirm("Are you sure you wish to delete the file: " + file_name) == true) {
        var jsonObj = {
            "filename": file_name,
            "delete": true
        };
        postUrl('edit_image.cgi', JSON.stringify(jsonObj), null,
            function(data, responseCode) {
                httpResponseHandler(data, responseCode, false, function(jsonResponse){
                    if (jsonResponse.success) {
                        document.getElementById("table_row_" + file_name).style.display = "none";
                    }
                }, null);
         }, httpErrorResponseHandler);
    }
}

async function sendSelectedToGD() {
    var button_ele = document.getElementById("sendSelectedButton");
    button_ele.disabled = true;
    var all_names = Array.from(document.getElementsByClassName("table_file_row")).map(item => item.dataset.filename);
    var selected_names = all_names.filter(filename => {return document.getElementById("checkbox_" + filename).checked})

    for (var i = 0; i < selected_names.length; i++) {
        button_ele.value = "Sending file " + (i + 1).toString() + " of " + selected_names.length.toString() + "...";
        await sendFileToGD(selected_names[i]);
    }

    button_ele.value = "Send selected to Google Drive";
    button_ele.disabled = false;
}
