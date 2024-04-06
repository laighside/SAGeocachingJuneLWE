/**
  @file    registration_form.js
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  This is all the JS for the page /cgi-bin/registration/registration_form.cgi

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */

var currentTab = 0; // Current tab is set to be the first tab (0)

var total_cost = 0; // order cost without surcharge
var total_cost_with_card_surcharge = 0; // order cost with surcharge

var idempotency_key;

// Load the stripe payments object
var stripe = Stripe(stripePublicKey);

/**
 * Loads the data for the registration form, called from the registration page
 */
function loadRegForm() {
    var ls_email = localStorage.getItem("email");
    if (ls_email)
        document.getElementById("email").value = ls_email;
    var ls_username = localStorage.getItem("gc_username");
    if (ls_username)
        document.getElementById("gc_username").value = ls_username;
    var ls_phone = localStorage.getItem("phone");
    if (ls_phone)
        document.getElementById("phone").value = ls_phone;

    setInnerTextIfExists(document.getElementById("display_price_event_adult"), price_event_adult);
    setInnerTextIfExists(document.getElementById("display_price_event_child"), price_event_child);
    setInnerHTMLIfExists(document.getElementById("display_price_camping"), price_camping_html);
    setInnerTextIfExists(document.getElementById("display_price_dinner_adult"), price_dinner_adult);
    setInnerTextIfExists(document.getElementById("display_price_dinner_child"), price_dinner_child);

    document.getElementById("payment_card").oninput = function (){paymentTypeChanged("payment")};
    document.getElementById("payment_bank").oninput = function (){paymentTypeChanged("payment")};
    document.getElementById("payment_cash").oninput = function (){paymentTypeChanged("payment")};

    idempotency_key = uuid();

    camping_options.map(function (option) {
        if (!option.available && option.available !== 0) {
            // available not set, do nothing
        } else if (option.available <= 0) {
            document.getElementById("camping_" + option.id_string + "_label_text").innerHTML += " <span style=\"font-weight:bold;color:black;\">(sold out)</span>";
            document.getElementById("camping_" + option.id_string).disabled = true;
            document.getElementById("camping_" + option.id_string + "_label").style.color = '#cccccc';
            document.getElementById("camping_" + option.id_string).checked = false;
        } else if (option.available <= 10) {
            document.getElementById("camping_" + option.id_string + "_label_text").innerHTML += " <span style=\"font-weight:bold;\">(only " + option.available.toString() + " left)</span>";
        }
    });

    var powered_camping_sites_remain = 0;
    camping_options.filter(function (option) {
        return (option.price_code === "powered")
    }).map(function (option) {
        if (option.available && option.available > 0) {
            powered_camping_sites_remain += option.available;
        } else if (option.available === null) {
            powered_camping_sites_remain += 1000;
        }
    });

    var remaining_camping_label = document.getElementById("remaining_camping_label");
    if (typeof(remaining_camping_label) != 'undefined' && remaining_camping_label != null) {
        if (powered_camping_sites_remain > 10) {
            document.getElementById("remaining_camping_label").style.display = "none";
        } else if (powered_camping_sites_remain <= 0) {
            document.getElementById("remaining_camping_label").innerHTML = "There are no powered sites remaining.";
        } else if (powered_camping_sites_remain == 1) {
            document.getElementById("remaining_camping_label").innerHTML = "There is only 1 powered site remaining. Book ASAP before we run out.";
        } else {
            document.getElementById("remaining_camping_label").innerHTML = "There are only " + powered_camping_sites_remain.toString() + " powered sites remaining. Book ASAP before we run out.";
        }
    }
}

/**
 * Sets the innerText for an element if it exists
 *
 * @param {Object} item The element
 * @param {String} value The value to set the innerText to
 */
function setInnerTextIfExists(item, value) {
    if (typeof(item) != 'undefined' && item != null){
        item.innerText = value;
    }
}

/**
 * Sets the innerHTML for an element if it exists
 *
 * @param {Object} item The element
 * @param {String} value The value to set the innerHTML to
 */
function setInnerHTMLIfExists(item, value) {
    if (typeof(item) != 'undefined' && item != null){
        item.innerHTML = value;
    }
}

/**
 * Validates the user input for each page of the form
 *
 * @returns {Boolean} True if the input is valid, false otherwise
 */
function validateForm() {
    var valid = true;
    removeFormMessages();
    var tabs = document.getElementsByClassName("formTab");

    if (currentTab == 0) {

        // Email, username or phone number can't be blank
        var element = document.getElementById("email");
        if (element.value == "") {
            element.className += " invalid";
            addFormMessage("Email:", "Please enter a valid email address");
            valid = false;
        }
        element = document.getElementById("gc_username");
        if (element.value == "") {
            element.className += " invalid";
            addFormMessage("GC Username:", "Please enter a valid username");
            valid = false;
        }
        element = document.getElementById("phone");
        if (element.value == "") {
            element.className += " invalid";
            addFormMessage("Phone Number:", "Please enter a valid phone number");
            valid = false;
        }

    }

    if (tabs[currentTab].id == "eventTab") {
        if (isRadioChecked("past_jlwe") == false) {
            setRadioClass("past_jlwe", "invalid");
            addFormMessage("Past June LWE:", "Please select if you've been to a June LWE before or not");
            valid = false;
        }
        if (isRadioChecked("camping") == false) {
            setRadioClass("camping", "invalid");
            addFormMessage("Camping:", "Please select if you wish to book camping site or not");
            valid = false;
        }
        if (dinner_form_enabled) {
            if (isRadioChecked("dinner") == false) {
              setRadioClass("dinner", "invalid");
              addFormMessage("Saturday Dinner:", "Please select if you will attending the Saturday dinner event or not");
              valid = false;
            }
        }
        return valid;

    }

    if (tabs[currentTab].id == "campingTab") {
        if (parseInt(document.getElementById("number_people_camping").value) <= 0) {
            addFormMessage("Number of people:", "There must be at least one person staying on your campsite");
            valid = false;
        }
        if (isRadioChecked("camping_type") == false) {
            setRadioClass("camping_type", "invalid");
            addFormMessage("Camping:", "Please select if you would like a powered or unpowered site");
            valid = false;
        }
        if (parseInt(document.getElementById("camping_leave").value) - parseInt(document.getElementById("camping_arrive").value) < 1) {
            addFormMessage("Camping dates:", "Invalid camping dates, you need to stay for at least one night");
            valid = false;
        }
        return valid;
    }

    if (tabs[currentTab].id == "dinnerTab") {
        // always valid
        return valid;
    }

    if (currentTab == summary_page) {
        if (isRadioChecked("payment") == false) {
            setRadioClass("payment", "invalid");
            addFormMessage("Payment:", "Please select select a valid payment type");
            valid = false;
        }
        return valid;
    }

    return valid; // return the valid status
}

/**
 * Called when the user moves to the next/previous page of the form
 * This is used to add/remove the camping/dinner pages depending on the user's choices
 */
function leavingTab(tabNumber) {
    // Save camping/dinner status if leaving the 1st tab
    if (tabNumber == 0 && event_form == true) {
        camping = document.getElementById("camping_yes").checked;
        dinner = dinner_form_enabled ? document.getElementById("dinner_yes").checked : false;
        document.getElementById("number_people_camping").value = parseInt(document.getElementById("number_adults").value) + parseInt(document.getElementById("number_children").value);
    }
}

/**
 * This skips over camping/dinner tabs if not needed
 *
 * @param {Integer} n n=1 for next page, n=-1 for previous page
 */
function skipTabs(n) {
    // Loop is to deal with user either going forward or backwards through the form.
    if (event_form) {
        var i;
        for (i = 0; i < 3; i++) {
            if (currentTab == 1 && camping == false) {
                currentTab = currentTab + n;
            }
            if (dinner_form_enabled) {
                if (currentTab == 2 && dinner == false) {
                    currentTab = currentTab + n;
                }
            }
        }
    }
}

function dinnerOptionsToString(options) {
    var output = "";
    if (options) {
        var i;
        for (i = 0; i < options.length; i++) {
            output += "<br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
            for (var prop in options[i]) {
                // skip loop if the property is from prototype
                if (!options[i].hasOwnProperty(prop)) continue;
                if (typeof options[i][prop] == 'string') {
                     output += options[i][prop] + ", ";
                }
            }
            output = output.substring(0, output.length - 2);
        }
    }
    return output;
}

/**
 * This updates the summary tab with the prices of the items the user has chosen
 */
function updateSummary() {
    document.getElementById("summary_username").innerText = document.getElementById("gc_username").value;
    document.getElementById("summary_phone").innerText = document.getElementById("phone").value;
    document.getElementById("summary_email").innerText = document.getElementById("email").value;

    var event_cost = 0;
    if (event_form) {
        var num_adults = parseInt(document.getElementById("number_adults").value);
        var num_children = parseInt(document.getElementById("number_children").value);
        document.getElementById("summary_event_desc").innerHTML = "Event registration<br /><span>&nbsp;&nbsp;&nbsp;" + num_adults.toString() + " " + (num_adults === 1 ? "Adult" : "Adults") + ", " + num_children.toString() + " " + (num_children === 1 ? "Child" : "Children") + "</span>";
        event_cost = num_adults * price_event_adult + num_children * price_event_child;
        document.getElementById("summary_event_cost").innerHTML = "$" + event_cost.toFixed(2);
    } else {
        document.getElementById("event_row").style.display = 'none';
    }

    var camping_cost = 0;
    if (camping) {
        var number_people_camping = parseInt(document.getElementById("number_people_camping").value);
        var camping_nights = parseInt(document.getElementById("camping_leave").value) - parseInt(document.getElementById("camping_arrive").value);
        var camping_desc = "";
        var camping_type = "unknown";
        var camping_type_name = "Unknown Type";
        camping_options.map(function (option) {
            if (document.getElementById("camping_" + option.id_string).checked) {
                camping_type = option.id_string;
                camping_type_name = option.display_name;
            }
        });

        camping_cost = getCampingPrice(camping_type, number_people_camping, camping_nights) / 100;
        camping_desc = camping_type_name + ", " + number_people_camping.toString() + " " + (number_people_camping === 1 ? "person" : "people") + ", " + camping_nights.toString() + " night" + (camping_nights === 1 ? "" : "s");

        document.getElementById("summary_camping_desc").innerHTML = "Camping<br /><span>&nbsp;&nbsp;&nbsp;" + camping_desc + "</span>";
        document.getElementById("summary_camping_cost").innerHTML = "$" + camping_cost.toFixed(2);

        document.getElementById("camping_row").style.display = 'table-row';
    } else {
      document.getElementById("camping_row").style.display = 'none';
    }

    var dinner_cost = 0;
    if (dinner) {
        if (saveOptions) {
            saveOptions(1);
            saveOptions(2);
        }
        var dinner_number_adults = parseInt(document.getElementById("item_1_count").value);
        var dinner_number_children = parseInt(document.getElementById("item_2_count").value);
        var dinner_table_html = "Saturday dinner<br /><span>&nbsp;&nbsp;&nbsp;" + dinner_number_adults.toString() + " Adult meal" + (dinner_number_adults === 1 ? "" : "s") + dinnerOptionsToString(dinner_selected_options["1"]) + "<br/>";
        dinner_table_html += "&nbsp;&nbsp;&nbsp;" + dinner_number_children.toString() + " Child meal" + (dinner_number_children === 1 ? "" : "s") + dinnerOptionsToString(dinner_selected_options["2"]) + "</span>";
        document.getElementById("summary_dinner_desc").innerHTML = dinner_table_html;
        dinner_cost = dinner_number_adults * price_dinner_adult + dinner_number_children * price_dinner_child;
        document.getElementById("summary_dinner_cost").innerHTML = "$" + dinner_cost.toFixed(2);

        document.getElementById("dinner_row").style.display = 'table-row';
    } else {
        document.getElementById("dinner_row").style.display = 'none';
    }

    total_cost = event_cost + camping_cost + dinner_cost;
    total_cost_with_card_surcharge = (total_cost + 0.3) / (1 - 0.0175);
    var card_surcharge_cost = total_cost_with_card_surcharge - total_cost;
    document.getElementById("summary_card_surcharge_cost").innerHTML = "$" + card_surcharge_cost.toFixed(2);
    paymentTypeChanged('null');

    if (camping || dinner) {
        document.getElementById("payment_cash").disabled = true;
        document.getElementById("payment_cash_label").style.color = '#cccccc';
        document.getElementById("payment_cash").checked = false;
        document.getElementById("payment_note").style.display = 'block';
        document.getElementById("cash_payment_note").style.display = 'none';
    } else {
        document.getElementById("payment_cash").disabled = false;
        document.getElementById("payment_cash_label").style.color = '';
        document.getElementById("payment_note").style.display = 'none';
        document.getElementById("cash_payment_note").style.display = 'block';
    }

}

/**
 * This adds/removes the card surcharge when the user selects the payment type
 */
function paymentTypeChanged(name) {
    setRadioClass(name, '');

    if (document.getElementById("payment_card").checked) {
        document.getElementById("card_surcharge_row").style.display = 'table-row';
        document.getElementById("summary_total_cost").innerHTML = "$" + total_cost_with_card_surcharge.toFixed(2);
    } else {
        document.getElementById("card_surcharge_row").style.display = 'none';
        document.getElementById("summary_total_cost").innerHTML = "$" + total_cost.toFixed(2);
    }
}

/**
 * This is called when the user submits the form
 * It sends the data to the server then forwards to the Stripe page if required
 * Or forwards to the confirmation page for bank/cash payments
 */
function submitForm() {
    document.getElementById("loader").style.display = "block";

    // save local values
    localStorage.setItem("email", document.getElementById("email").value);
    localStorage.setItem("gc_username", document.getElementById("gc_username").value);
    localStorage.setItem('phone', document.getElementById("phone").value);

    var camping_json = "no"; //document.getElementById("camping_share").checked ? "share" : "no";
    if (camping) {
        var camping_type = "unknown";
        camping_options.map(function (option) {
            if (document.getElementById("camping_" + option.id_string).checked)
                camping_type = option.id_string;
        });
        camping_json = {
            camping_type: camping_type,
            number_people: parseInt(document.getElementById("number_people_camping").value),
            arrive_date: parseInt(document.getElementById("camping_arrive").value),
            leave_date: parseInt(document.getElementById("camping_leave").value),
            camping_comment: document.getElementById("camping_comments").value
        };
    }

    var dinner_json = dinner;
    if (dinner) {
        var dinner_number_adults = parseInt(document.getElementById("item_1_count").value);
        var dinner_number_children = parseInt(document.getElementById("item_2_count").value);
        dinner_json = {
            dinner_number_adults: dinner_number_adults,
            dinner_number_children: dinner_number_children,
            dinner_options_adults: dinner_selected_options["1"],
            dinner_options_children: dinner_selected_options["2"],
            dinner_comment: document.getElementById("dinner_comments").value
        };
    }

    var jsonOut = {
        idempotency: idempotency_key,
        mode: stripe._keyMode,
        gc_username: document.getElementById("gc_username").value,
        email: document.getElementById("email").value,
        phone: document.getElementById("phone").value,

        payment_type: (document.getElementById("payment_card").checked ? "card" : (document.getElementById("payment_bank").checked ? "bank" : "cash")),

        camping: camping_json,
        dinner: dinner_json
    };

    if (event_form) {
        jsonOut.email_gpx = document.getElementById("email_gpx").checked;
        jsonOut.real_names_adults = document.getElementById("real_names_adults").value;
        jsonOut.real_names_children = document.getElementById("real_names_children").value;
        jsonOut.number_adults = parseInt(document.getElementById("number_adults").value);
        jsonOut.number_children = parseInt(document.getElementById("number_children").value);
        jsonOut.past_jlwe = document.getElementById("past_jlwe_yes").checked;
        jsonOut.have_lanyard = false; //document.getElementById("lanyard_yes").checked;
    }

    postUrl(formPostURL, JSON.stringify(jsonOut), null,
        function(data, responseCode) {
            if (responseCode === 200){
                var jsonObj = JSON.parse(data);
                if (jsonObj.success !== undefined){
                    if (jsonObj.success === true) {
                        // success
                        if (jsonObj.stripeSessionId !== undefined){
                            stripe.redirectToCheckout({
                                sessionId: jsonObj.stripeSessionId
                            })
                        } else if (jsonObj.redirect !== undefined){
                            var url = jsonObj.redirect;
                            window.location.href = url;
                        } else {
                            document.getElementById("loader").style.display = "none";
                            alert("Unknown error");
                        }
                    } else {
                        // server responded with an error
                        document.getElementById("loader").style.display = "none";
                        if (typeof jsonObj.error == 'string') {
                            alert("Error: " + jsonObj.error);
                        } else {
                            alert("Error processing registration");
                        }
                    }
                } else {
                    // Server response was not in expected format, bad JSON
                    document.getElementById("loader").style.display = "none";
                    alert("Error processing registration: Invalid server response");
                }
            }
        }, function(statusCode, statusText) {
            document.getElementById("loader").style.display = "none";
            if (statusText && typeof statusText == 'string') {
                alert("HTTP error: " + statusText);
            } else {
                alert("HTTP error: Unknown");
            }
        });
}
