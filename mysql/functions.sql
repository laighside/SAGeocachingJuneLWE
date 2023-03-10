-- SQL Functions for JLWE database
--
-- This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
-- https://github.com/laighside/SAGeocachingJuneLWE
--

/**
 * addCSPReport This adds an entry to the csp_reports table
 */
DROP FUNCTION IF EXISTS addCSPReport;
DELIMITER $$
CREATE FUNCTION addCSPReport(user_agentIn VARCHAR(500), contentIn LONGTEXT, userIP VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    INSERT INTO csp_reports (time_received, IP_address, user_agent, content) VALUES(NOW(), userIP, user_agentIn, contentIn);
    RETURN 0;
END$$
DELIMITER ;

/**
 * addEmailAddress This adds an entry to the email_list table
 */
DROP FUNCTION IF EXISTS addEmailAddress;
DELIMITER $$
CREATE FUNCTION addEmailAddress(emailAddress VARCHAR(200), token_verify VARCHAR(150), token_unsub VARCHAR(150), userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    IF (EXISTS(SELECT * FROM email_list WHERE email = emailAddress AND unsubscribed = 0)) THEN
        RETURN 1;
    ELSE
        IF (EXISTS(SELECT * FROM email_list WHERE email = emailAddress)) THEN
            UPDATE email_list SET unsubscribed = 0, verify_token = token_verify, unsub_token = token_unsub WHERE email = emailAddress;
            SET dummy = log_user_event(userIP, username, CONCAT("Email \"",  emailAddress, "\" re-subscribed to mailing list"));
        ELSE
            INSERT INTO email_list (email, verify_token, unsub_token) VALUES(emailAddress, token_verify, token_unsub);
            SET dummy = log_user_event(userIP, username, CONCAT("Email \"",  emailAddress, "\" added to mailing list"));
        END IF;
        RETURN 0;
    END IF;
END$$
DELIMITER ;

/**
 * addNewTeam This adds an entry to the game_teams table
 */
DROP FUNCTION IF EXISTS addNewTeam;
DELIMITER $$
CREATE FUNCTION addNewTeam(team_nameIn VARCHAR(100), userIn VARCHAR(100), userIP VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    INSERT INTO game_teams (team_name, team_members) VALUES (team_nameIn, "");
    RETURN LAST_INSERT_ID();
END$$
DELIMITER ;

/**
 * addStripeSessionID This sets the stripe_session_id for a given userKey
 */
DROP FUNCTION IF EXISTS addStripeSessionID;
DELIMITER $$
CREATE FUNCTION addStripeSessionID(userKey VARCHAR(100), stripe_session_idIn VARCHAR(200)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    IF (EXISTS(SELECT * FROM event_registrations WHERE idempotency = userKey)) THEN
        UPDATE event_registrations SET stripe_session_id = stripe_session_idIn WHERE idempotency = userKey;
        RETURN 1;
    ELSE
        IF (EXISTS(SELECT * FROM camping WHERE idempotency = userKey)) THEN
            UPDATE camping SET stripe_session_id = stripe_session_idIn WHERE idempotency = userKey;
            RETURN 2;
        ELSE
            IF (EXISTS(SELECT * FROM sat_dinner WHERE idempotency = userKey)) THEN
                UPDATE sat_dinner SET stripe_session_id = stripe_session_idIn WHERE idempotency = userKey;
                RETURN 3;
            ELSE
                IF (EXISTS(SELECT * FROM merch_orders WHERE idempotency = userKey)) THEN
                    UPDATE merch_orders SET stripe_session_id = stripe_session_idIn WHERE idempotency = userKey;
                    RETURN 4;
                ELSE
                    RETURN 0;
                END IF;
            END IF;
        END IF;
    END IF;
END$$
DELIMITER ;

/**
 * addUserCache This adds an entry to the user_hidden_caches table
 */
DROP FUNCTION IF EXISTS addUserCache;
DELIMITER $$
CREATE FUNCTION addUserCache(team_nameIn VARCHAR(100), phone_numberIn VARCHAR(100), cache_numberIn int, cache_nameIn VARCHAR(100), lat DOUBLE, lon DOUBLE, public_hintIn VARCHAR(1000), detailed_hintIn VARCHAR(1000), camoIn TINYINT, permanentIn TINYINT, private_propertyIn TINYINT, zone_bonusIn INT, osm_distanceIn INT, actual_distanceIn INT, userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
        INSERT INTO user_hidden_caches (team_name, phone_number, cache_number, cache_name, latitude, longitude, public_hint, detailed_hint, camo, permanent, private_property, zone_bonus, osm_distance, actual_distance, timestamp, IP_address) VALUES(team_nameIn, phone_numberIn, cache_numberIn, cache_nameIn, lat, lon, public_hintIn, detailed_hintIn, camoIn, permanentIn, private_propertyIn, zone_bonusIn, osm_distanceIn, actual_distanceIn, NOW(), userIP);
    SET dummy = log_user_event(userIP, username, CONCAT("User submitted cache number ",  CONVERT(cache_numberIn, CHAR(50)), "; id: ", CONVERT(LAST_INSERT_ID(), CHAR(50))));
        RETURN 0;
END$$
DELIMITER ;

/**
 * changeStatus This sets the status for a given file
 */
DROP FUNCTION IF EXISTS changeStatus;
DELIMITER $$
CREATE FUNCTION changeStatus(href VARCHAR(200), newStatus TINYINT, userIn VARCHAR(45), userIP VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    UPDATE files SET public = newStatus WHERE CONCAT(directory,filename) = href;
    SET dummy = log_user_event(userIP, userIn, CONCAT("Status of file \"", href, "\" changed to ", CAST(newStatus AS CHAR)));
    RETURN 0;
END$$
DELIMITER ;

/**
 * clearEmailList This clears the email_list table
 */
DROP FUNCTION IF EXISTS clearEmailList;
DELIMITER $$
CREATE FUNCTION clearEmailList(userIP VARCHAR(50), usernameIn VARCHAR(100)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    SET dummy = log_user_event(userIP, usernameIn, CONCAT("Table email_list was cleared"));
    DELETE FROM email_list;
    RETURN 0;
END$$
DELIMITER ;

/**
 * clearCacheHandoutList This clears and resets the cache_handout table
 */
DROP FUNCTION IF EXISTS clearCacheHandoutList;
DELIMITER $$
CREATE FUNCTION clearCacheHandoutList(userIP VARCHAR(50), usernameIn VARCHAR(100), cacheCount INT) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    DECLARE i INT;
    SET dummy = log_user_event(userIP, usernameIn, CONCAT("Table cache_handout was cleared and reset to ", CONVERT(cacheCount, CHAR), " caches"));
    DELETE FROM cache_handout;

    SET i = 0;
    createLoop: LOOP
        SET i = i + 1;
        INSERT INTO cache_handout (cache_number, owner_name, returned, team_id) VALUES(i, "", 0, -1);

        IF i = cacheCount THEN
            LEAVE createLoop;
        END IF;
    END LOOP createLoop;

    RETURN 0;
END$$
DELIMITER ;

/**
 * clearRegistrations This clears the event_registrations, camping, sat_dinner tables
 */
DROP FUNCTION IF EXISTS clearRegistrations;
DELIMITER $$
CREATE FUNCTION clearRegistrations(userIP VARCHAR(50), usernameIn VARCHAR(100)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    SET dummy = log_user_event(userIP, usernameIn, CONCAT("Tables event_registrations, camping, sat_dinner were cleared"));
    DELETE FROM event_registrations;
    DELETE FROM camping;
    DELETE FROM sat_dinner;
    RETURN 0;
END$$
DELIMITER ;

/**
 * clearTeamList This clears the game_teams table
 */
DROP FUNCTION IF EXISTS clearTeamList;
DELIMITER $$
CREATE FUNCTION clearTeamList(userIP VARCHAR(50), usernameIn VARCHAR(100)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    SET dummy = log_user_event(userIP, usernameIn, CONCAT("Table game_teams was cleared"));
    DELETE FROM game_teams;
    UPDATE cache_handout SET team_id = -1;
    RETURN 0;
END$$
DELIMITER ;

/**
 * createFile This adds an entry to the files table
 */
DROP FUNCTION IF EXISTS createFile;
DELIMITER $$
CREATE FUNCTION createFile(filenameIn VARCHAR(50), directoryIn VARCHAR(50), yearIn INT, publicIn TINYINT, userIn VARCHAR(45), sizeIn INT, userIP VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    IF (EXISTS(SELECT * FROM files WHERE filename = filenameIn AND directory = directoryIn)) THEN
        RETURN 1;
    ELSE
        INSERT INTO files (filename, directory, owner, size, year, public) VALUES(filenameIn, directoryIn, userIn, sizeIn, yearIn, publicIn);
        SET dummy = log_user_event(userIP, userIn, CONCAT("Created file/directory: \"",  filenameIn, "\""));
    END IF;
    RETURN 0;
END$$
DELIMITER ;

/**
 * createWebpageImage This adds an entry to the webpage_images table
 */
DROP FUNCTION IF EXISTS createWebpageImage;
DELIMITER $$
CREATE FUNCTION createWebpageImage(filenameIn VARCHAR(100), sizeIn INT, userIn VARCHAR(100), userIP VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    IF (EXISTS(SELECT * FROM webpage_images WHERE filename = filenameIn)) THEN
        RETURN 1;
    ELSE
        INSERT INTO webpage_images (filename, owner, file_size) VALUES(filenameIn, userIn, sizeIn);
        SET dummy = log_user_event(userIP, userIn, CONCAT("Uploaded image \"",  filenameIn, "\""));
    END IF;
    RETURN 0;
END$$
DELIMITER ;

/**
 * deleteCache This deletes an entry from the caches table
 */
DROP FUNCTION IF EXISTS deleteCache;
DELIMITER $$
CREATE FUNCTION deleteCache(id_number INT, userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    DELETE FROM caches WHERE cache_number = id_number;
    SET dummy = log_user_event(userIP, username, CONCAT("Cache number ",  id_number, " deleted"));
    RETURN 0;
END$$
DELIMITER ;

/**
 * deleteFile This deletes an entry from the files table
 */
DROP FUNCTION IF EXISTS deleteFile;
DELIMITER $$
CREATE FUNCTION deleteFile(href VARCHAR(200), userIn VARCHAR(100), userIP VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    DELETE FROM files WHERE CONCAT(directory,filename) = href;
    SET dummy = log_user_event(userIP, userIn, CONCAT("Deleted file \"",  href, "\""));
    RETURN 0;
END$$
DELIMITER ;

/**
 * deleteImage This deletes an entry from the webpage_images table
 */
DROP FUNCTION IF EXISTS deleteImage;
DELIMITER $$
CREATE FUNCTION deleteImage(file_name VARCHAR(200), userIn VARCHAR(100), userIP VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    DELETE FROM webpage_images WHERE filename = file_name;
    SET dummy = log_user_event(userIP, userIn, CONCAT("Deleted image: \"",  file_name, "\""));
    RETURN 0;
END$$
DELIMITER ;

/**
 * deleteTeam This deletes an entry from the game_teams table
 */
DROP FUNCTION IF EXISTS deleteTeam;
DELIMITER $$
CREATE FUNCTION deleteTeam(team_idIn INT, userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    IF (EXISTS(SELECT * FROM game_teams WHERE team_id = team_idIn)) THEN
        DELETE FROM game_teams WHERE team_id = team_idIn;
            RETURN 0;
    END IF;
    RETURN 1;
END$$
DELIMITER ;

/**
 * insertCamping This adds an entry to the camping table
 */
DROP FUNCTION IF EXISTS insertCamping;
DELIMITER $$
CREATE FUNCTION insertCamping(idempotencyIn VARCHAR(100), email_addressIn VARCHAR(100), gc_usernameIn VARCHAR(100), phone_numberIn VARCHAR(100), livemodeIn TINYINT, number_peopleIn int, camping_typeIn VARCHAR(50), arrive_dateIn INT, leave_dateIn INT, camping_commentIn VARCHAR(1000), payment_typeIn VARCHAR(10), userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    IF (EXISTS(SELECT * FROM camping WHERE idempotency = idempotencyIn)) THEN
        RETURN (SELECT registration_id FROM camping WHERE idempotency = idempotencyIn);
    ELSE
        INSERT INTO camping (ip_address, timestamp, idempotency, email_address, gc_username, phone_number, livemode, number_people, camping_type, arrive_date, leave_date, camping_comment, payment_type) VALUES (userIP, NOW(), idempotencyIn, email_addressIn, gc_usernameIn, phone_numberIn, livemodeIn, number_peopleIn, camping_typeIn, arrive_dateIn, leave_dateIn, camping_commentIn, payment_typeIn);
        RETURN LAST_INSERT_ID();
    END IF;
END$$
DELIMITER ;

/**
 * insertContactForm This adds an entry to the contact_form table
 */
DROP FUNCTION IF EXISTS insertContactForm;
DELIMITER $$
CREATE FUNCTION insertContactForm(from_nameIn TEXT, email_addressIn VARCHAR(100), messageIn TEXT, userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    INSERT INTO contact_form (ip_address, timestamp, from_name, email_address, message, status) VALUES (userIP, NOW(), from_nameIn, email_addressIn, messageIn, 'O');
    RETURN 1;
END$$
DELIMITER ;

/**
 * insertDinner This adds an entry to the sat_dinner table
 */
DROP FUNCTION IF EXISTS insertDinner;
DELIMITER $$
CREATE FUNCTION insertDinner(idempotencyIn VARCHAR(100), email_addressIn VARCHAR(100), gc_usernameIn VARCHAR(100), phone_numberIn VARCHAR(100), livemodeIn TINYINT, number_adultsIn int, number_childrenIn int, dinner_commentIn VARCHAR(1000), payment_typeIn VARCHAR(10), userIP VARCHAR(50), username VARCHAR(50), number_adults_op1In int, number_adults_op2In int, number_adults_op3In int, number_children_op1In int, number_children_op2In int, number_children_op3In int, number_dessert_op1In int, number_dessert_op2In int, number_dessert_op3In int) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    IF (EXISTS(SELECT * FROM sat_dinner WHERE idempotency = idempotencyIn)) THEN
        RETURN (SELECT registration_id FROM sat_dinner WHERE idempotency = idempotencyIn);
    ELSE
        INSERT INTO sat_dinner (ip_address, timestamp, idempotency, email_address, gc_username, phone_number, livemode, number_adults, number_children, dinner_comment, payment_type, number_adults_op1, number_adults_op2, number_adults_op3, number_children_op1, number_children_op2, number_children_op3, number_dessert_op1, number_dessert_op2, number_dessert_op3) VALUES (userIP, NOW(), idempotencyIn, email_addressIn, gc_usernameIn, phone_numberIn, livemodeIn, number_adultsIn, number_childrenIn, dinner_commentIn, payment_typeIn, number_adults_op1In, number_adults_op2In, number_adults_op3In, number_children_op1In, number_children_op2In, number_children_op3In, number_dessert_op1In, number_dessert_op2In, number_dessert_op3In);
        RETURN LAST_INSERT_ID();
    END IF;
END$$
DELIMITER ;

/**
 * insertPayment This adds an entry to the payment_log table
 */
DROP FUNCTION IF EXISTS insertPayment;
DELIMITER $$
CREATE FUNCTION insertPayment(userKeyIn VARCHAR(100), timestampIn INT, amount_receivedIn INT, typeIn VARCHAR(100)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    INSERT INTO payment_log (user_key, timestamp, amount_received, payment_type) VALUES (userKeyIn, timestampIn, amount_receivedIn, typeIn);
    RETURN 1;
END$$
DELIMITER ;

/**
 * insertRegistration This adds an entry to the event_registrations table
 */
DROP FUNCTION IF EXISTS insertRegistration;
DELIMITER $$
CREATE FUNCTION insertRegistration(idempotencyIn VARCHAR(100), email_addressIn VARCHAR(100), gc_usernameIn VARCHAR(100), phone_numberIn VARCHAR(100), livemodeIn TINYINT, real_names_adultsIn VARCHAR(100), real_names_childrenIn VARCHAR(100), number_adultsIn int, number_childrenIn int, past_jlweIn TINYINT, have_lanyardIn TINYINT, campingIn VARCHAR(10), dinnerIn VARCHAR(10), payment_typeIn VARCHAR(10), userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    IF (EXISTS(SELECT * FROM event_registrations WHERE idempotency = idempotencyIn)) THEN
        RETURN (SELECT registration_id FROM event_registrations WHERE idempotency = idempotencyIn);
    ELSE
        INSERT INTO event_registrations (ip_address, timestamp, idempotency, email_address, gc_username, phone_number, livemode, real_names_adults, real_names_children, number_adults, number_children, past_jlwe, have_lanyard, camping, dinner, payment_type) VALUES (userIP, NOW(), idempotencyIn, email_addressIn, gc_usernameIn, phone_numberIn, livemodeIn, real_names_adultsIn, real_names_childrenIn, number_adultsIn, number_childrenIn, past_jlweIn, have_lanyardIn, campingIn, dinnerIn, payment_typeIn);
        SET dummy = log_user_event(userIP, username, CONCAT("User registration received for ", gc_usernameIn, " (", email_addressIn, ") id: ", CONVERT(LAST_INSERT_ID(), CHAR(50))));
        RETURN LAST_INSERT_ID();
    END IF;
END$$
DELIMITER ;

/**
 * insertStripeEvent This adds an entry to the stripe_event_log table
 */
DROP FUNCTION IF EXISTS insertStripeEvent;
DELIMITER $$
CREATE FUNCTION insertStripeEvent(idIn VARCHAR(100), timestampIn INT, livemodeIn TINYINT, typeIn VARCHAR(100), api_versionIn VARCHAR(100), payment_intentIn VARCHAR(100), cs_idIn VARCHAR(100), amountIn INT, amount_receivedIn INT, messageIn VARCHAR(1000)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    IF (EXISTS(SELECT * FROM stripe_event_log WHERE id = idIn)) THEN
        RETURN 0;
    ELSE
        IF (SUBSTRING(typeIn, 1, 15) = 'payment_intent.' OR typeIn = 'charge.refunded') THEN
            IF (LENGTH(messageIn) > 0) THEN
                INSERT INTO stripe_event_log (id, timestamp, livemode, type, api_version, payment_intent, amount, amount_received, message) VALUES (idIn, timestampIn, livemodeIn, typeIn, api_versionIn, payment_intentIn, amountIn, amount_receivedIn, messageIn);
            ELSE
                INSERT INTO stripe_event_log (id, timestamp, livemode, type, api_version, payment_intent, amount, amount_received) VALUES (idIn, timestampIn, livemodeIn, typeIn, api_versionIn, payment_intentIn, amountIn, amount_receivedIn);
            END IF;
        ELSEIF (typeIn = 'checkout.session.completed') THEN
            INSERT INTO stripe_event_log (id, timestamp, livemode, type, api_version, payment_intent, cs_id) VALUES (idIn, timestampIn, livemodeIn, typeIn, api_versionIn, payment_intentIn, cs_idIn);
        ELSE
            INSERT INTO stripe_event_log (id, timestamp, livemode, type, api_version) VALUES (idIn, timestampIn, livemodeIn, typeIn, api_versionIn);
        END IF;
        RETURN 1;
    END IF;
END$$
DELIMITER ;

/**
 * insertStripeFee This adds an entry to the stripe_card_fees table
 */
DROP FUNCTION IF EXISTS insertStripeFee;
DELIMITER $$
CREATE FUNCTION insertStripeFee(idempotencyIn VARCHAR(100), feeIn int, userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    IF (EXISTS(SELECT * FROM stripe_card_fees WHERE idempotency = idempotencyIn)) THEN
        UPDATE stripe_card_fees SET stripe_card_fees.fee = feeIn WHERE stripe_card_fees.idempotency = idempotencyIn;
        RETURN 0;
    ELSE
        INSERT INTO stripe_card_fees (idempotency, fee) VALUES (idempotencyIn, feeIn);
        RETURN 1;
    END IF;
END$$
DELIMITER ;

/**
 * loginAttempt This adds an entry to the login_attempts table
 */
DROP FUNCTION IF EXISTS loginAttempt;
DELIMITER $$
CREATE FUNCTION loginAttempt(usernameIn VARCHAR(100), IPin VARCHAR(100), correctIn INT) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    INSERT INTO login_attempts (attempt_time, username, ip_address, correct) VALUES (NOW(), usernameIn, IPin, correctIn);
    RETURN 0;
END$$
DELIMITER ;

/**
 * log_user_event This adds an entry to the user_log table
 */
DROP FUNCTION IF EXISTS log_user_event;
DELIMITER $$
CREATE FUNCTION log_user_event(userIPin VARCHAR(50), usernameIn VARCHAR(50), event_text VARCHAR(200)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
	INSERT INTO user_log (timestamp, userIP, username, action) VALUES(UNIX_TIMESTAMP(), userIPin, usernameIn, event_text);
	RETURN 0;
END$$
DELIMITER ;

/**
 * renameFile This changes the name and/or directory of a file in the files table
 */
DROP FUNCTION IF EXISTS renameFile;
DELIMITER $$
CREATE FUNCTION renameFile(oldFolder VARCHAR(500), oldFile VARCHAR(500), newFolder VARCHAR(500), newFile VARCHAR(500), userIn VARCHAR(100), userIP VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    UPDATE files SET filename = newFile, directory = newFolder WHERE directory = oldFolder AND filename = oldFile;
    RETURN 0;
END$$
DELIMITER ;

/**
 * resetPasswordRequest This sets the reset_token for a given user's email, or returns 1 of the user's email isn't known
 */
DROP FUNCTION IF EXISTS resetPasswordRequest;
DELIMITER $$
CREATE FUNCTION resetPasswordRequest(email_address VARCHAR(200), token_reset VARCHAR(150), userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    IF (EXISTS(SELECT * FROM users WHERE email = email_address)) THEN
        UPDATE users SET reset_token = token_reset WHERE email = email_address;
        SET dummy = log_user_event(userIP, username, CONCAT("Password reset sent to \"",  email_address, "\""));
        RETURN 0;
    ELSE
        RETURN 1;
    END IF;
END$$
DELIMITER ;

/**
 * setBestCache This sets the winner for the given award id
 */
DROP FUNCTION IF EXISTS setBestCache;
DELIMITER $$
CREATE FUNCTION setBestCache(var_idIn VARCHAR(200), var_valueIn VARCHAR(200), userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    IF (EXISTS(SELECT * FROM best_caches WHERE id = var_idIn)) THEN
        UPDATE best_caches SET best_caches.cache = var_valueIn WHERE best_caches.id = var_idIn;
        RETURN 0;
    END IF;
    RETURN 1;
END$$
DELIMITER ;

/**
 * setCacheDetails This adds an entry, or updates an existing entry in the caches table
 */
DROP FUNCTION IF EXISTS setCacheDetails;
DELIMITER $$

CREATE FUNCTION setCacheDetails(cache_numberIn int, cache_nameIn VARCHAR(100), team_nameIn VARCHAR(100), lat DOUBLE, lon DOUBLE, public_hintIn VARCHAR(1000), detailed_hintIn VARCHAR(1000), camoIn TINYINT, permanentIn TINYINT, private_propertyIn TINYINT, zone_bonusIn INT, osm_distanceIn INT, actual_distanceIn INT, userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    IF (EXISTS(SELECT * FROM caches WHERE cache_number = cache_numberIn)) THEN
        UPDATE caches SET cache_name = cache_nameIn, team_name = team_nameIn, latitude = lat, longitude = lon, public_hint = public_hintIn, detailed_hint = detailed_hintIn, camo = camoIn, permanent = permanentIn, private_property = private_propertyIn, zone_bonus = zone_bonusIn, osm_distance = osm_distanceIn, actual_distance = actual_distanceIn WHERE cache_number = cache_numberIn;
        SET dummy = log_user_event(userIP, username, CONCAT("Cache number ",  CONVERT(cache_numberIn, CHAR(50)), " was updated"));
        RETURN cache_numberIn;
    ELSE
        INSERT INTO caches (cache_number, cache_name, team_name, latitude, longitude, public_hint, detailed_hint, camo, permanent, private_property, zone_bonus, osm_distance, actual_distance) VALUES(cache_numberIn, cache_nameIn, team_nameIn, lat, lon, public_hintIn, detailed_hintIn, camoIn, permanentIn, private_propertyIn, zone_bonusIn, osm_distanceIn, actual_distanceIn);
        SET dummy = log_user_event(userIP, username, CONCAT("Cache number ",  CONVERT(cache_numberIn, CHAR(50)), " was created"));
        RETURN 0;
    END IF;
END$$
DELIMITER ;

/**
 * setCacheStatus This sets the status for an entry in the user_hidden_caches table
 */
DROP FUNCTION IF EXISTS setCacheStatus;
DELIMITER $$
CREATE FUNCTION setCacheStatus(cacheIDnumber INT, newStatus CHAR(1), userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    IF (EXISTS(SELECT * FROM user_hidden_caches WHERE id_number = cacheIDnumber)) THEN
        UPDATE user_hidden_caches SET user_hidden_caches.status = newStatus WHERE id_number = cacheIDnumber;
        SET dummy = log_user_event(userIP, username, CONCAT("Status for user cache (id: ",  cacheIDnumber, ") was updated to \"", newStatus, "\""));
        RETURN 0;
    END IF;
    RETURN 1;
END$$
DELIMITER ;

/**
 * setContactFormStatus This sets the status for a given message ID in the contact_form table
 */
DROP FUNCTION IF EXISTS setContactFormStatus;
DELIMITER $$
CREATE FUNCTION setContactFormStatus(messageId VARCHAR(100), newStatus CHAR(1), userIP VARCHAR(50), username VARCHAR(100)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    IF (EXISTS(SELECT * FROM contact_form WHERE id = messageId)) THEN
        UPDATE contact_form SET contact_form.status = newStatus WHERE id = messageId;
    END IF;
    RETURN 0;
END$$
DELIMITER ;

/**
 * setEmailForwarder This adds an entry, updates an existing entry, or deletes an entry in the email_forwarders table
 */
DROP FUNCTION IF EXISTS setEmailForwarder;
DELIMITER $$
CREATE FUNCTION setEmailForwarder(sourceEmail VARCHAR(500), destinationEmail VARCHAR(500), deleteEmail TINYINT, userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    IF (deleteEmail > 0) THEN
        DELETE FROM email_forwarders WHERE source = sourceEmail;
        SET dummy = log_user_event(userIP, username, CONCAT("Email forwarder for \"",  sourceEmail, "@jlwe.org\" was deleted"));
        RETURN 3;
    ELSE
        IF (EXISTS(SELECT * FROM email_forwarders WHERE source = sourceEmail)) THEN
            UPDATE email_forwarders SET destination = destinationEmail WHERE source = sourceEmail;
            SET dummy = log_user_event(userIP, username, CONCAT("Email forwarder for \"",  sourceEmail, "@jlwe.org\" was edited"));
            RETURN 2;
        ELSE
            INSERT INTO email_forwarders (source, destination) VALUES(sourceEmail, destinationEmail);
            SET dummy = log_user_event(userIP, username, CONCAT("Email forwarder for \"",  sourceEmail, "@jlwe.org\" was created"));
            RETURN 1;
        END IF;
    END IF;
    RETURN 0;
END$$
DELIMITER ;

/**
 * setHandoutCacheOwner This sets the owner_name for an entry in the cache_handout table
 */
DROP FUNCTION IF EXISTS setHandoutCacheOwner;
DELIMITER $$
CREATE FUNCTION setHandoutCacheOwner(cache_numberIn int, ownerIn VARCHAR(500), userIP VARCHAR(100), username VARCHAR(100)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    UPDATE cache_handout SET owner_name = ownerIn WHERE cache_number = cache_numberIn;
    RETURN 0;
END$$
DELIMITER ;

/**
 * setHandoutCacheReturned This sets the returned status for an entry in the cache_handout table
 */
DROP FUNCTION IF EXISTS setHandoutCacheReturned;
DELIMITER $$
CREATE FUNCTION setHandoutCacheReturned(cache_numberIn int, returnedIn VARCHAR(500), userIP VARCHAR(100), username VARCHAR(100)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    UPDATE cache_handout SET returned = returnedIn WHERE cache_number = cache_numberIn;
    RETURN 0;
END$$
DELIMITER ;

/**
 * setHandoutCacheTeam This sets the team_id for an entry in the cache_handout table
 */
DROP FUNCTION IF EXISTS setHandoutCacheTeam;
DELIMITER $$
CREATE FUNCTION setHandoutCacheTeam(cache_numberIn INT, teamIn INT, userIP VARCHAR(100), username VARCHAR(100)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    IF (EXISTS(SELECT * FROM cache_handout WHERE cache_number = cache_numberIn)) THEN
        UPDATE cache_handout SET team_id = teamIn WHERE cache_number = cache_numberIn;
        RETURN 0;
    END IF;
    RETURN 1;
END$$
DELIMITER ;

/**
 * setNotesMD This adds an entry to the admin_notes table
 */
DROP FUNCTION IF EXISTS setNotesMD;
DELIMITER $$
CREATE FUNCTION setNotesMD(markdownIn LONGTEXT, userID INT, userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    INSERT INTO admin_notes (user_id, markdown) VALUES(userID, markdownIn);
    RETURN 0;
END$$
DELIMITER ;

/**
 * setPasswordHash This sets the password hash for a given user_id in the users table
 */
DROP FUNCTION IF EXISTS setPasswordHash;
DELIMITER $$
CREATE FUNCTION setPasswordHash(new_hash VARCHAR(100), target_user_id INT, userIP VARCHAR(50), usernameIn VARCHAR(100)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    IF (EXISTS(SELECT * FROM users WHERE user_id = target_user_id)) THEN
        UPDATE users SET pass_hash = new_hash, reset_token = '' WHERE user_id = target_user_id;
        SET dummy = log_user_event(userIP, usernameIn, CONCAT("Password for user id: ", CONVERT(target_user_id, CHAR(50)), " was changed"));
        RETURN 1;
    ELSE
        RETURN 0;
    END IF;
END$$
DELIMITER ;

/**
 * setRegistrationStatus This sets the status for a given userKey in the registrations, dinner and camping tables
 */
DROP FUNCTION IF EXISTS setRegistrationStatus;
DELIMITER $$
CREATE FUNCTION setRegistrationStatus(userKey VARCHAR(100), newStatus CHAR(1), userIP VARCHAR(50), username VARCHAR(100)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    IF (EXISTS(SELECT * FROM event_registrations WHERE idempotency = userKey)) THEN
        UPDATE event_registrations SET event_registrations.status = newStatus WHERE idempotency = userKey;
    END IF;
    IF (EXISTS(SELECT * FROM sat_dinner WHERE idempotency = userKey)) THEN
        UPDATE sat_dinner SET sat_dinner.status = newStatus WHERE idempotency = userKey;
    END IF;
    IF (EXISTS(SELECT * FROM camping WHERE idempotency = userKey)) THEN
        UPDATE camping SET camping.status = newStatus WHERE idempotency = userKey;
    END IF;
    IF (EXISTS(SELECT * FROM merch_orders WHERE idempotency = userKey)) THEN
        UPDATE merch_orders SET merch_orders.status = newStatus WHERE idempotency = userKey;
    END IF;
    RETURN 0;
END$$
DELIMITER ;

/**
 * setPasswordHash This sets the competing status for a given team_id in the game_teams table
 */
DROP FUNCTION IF EXISTS setTeamCompeting;
DELIMITER $$
CREATE FUNCTION setTeamCompeting(team_idIn INT, competingIn TINYINT, userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    IF (EXISTS(SELECT * FROM game_teams WHERE team_id = team_idIn)) THEN
        UPDATE game_teams SET competing = competingIn WHERE team_id = team_idIn;
        RETURN 0;
    END IF;
    RETURN 1;
END$$
DELIMITER ;

/**
 * setTeamFinalScore This sets the final_score for a given team_id in the game_teams table
 */
DROP FUNCTION IF EXISTS setTeamFinalScore;
DELIMITER $$
CREATE FUNCTION setTeamFinalScore(team_idIn INT, final_scoreIn VARCHAR(500), userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    IF (EXISTS(SELECT * FROM game_teams WHERE team_id = team_idIn)) THEN
        UPDATE game_teams SET final_score = final_scoreIn WHERE team_id = team_idIn;
        RETURN 0;
    END IF;
    RETURN 1;
END$$
DELIMITER ;

/**
 * setTeamFinalScore This sets the team_members for a given team_id in the game_teams table
 */
DROP FUNCTION IF EXISTS setTeamMembers;
DELIMITER $$
CREATE FUNCTION setTeamMembers(team_idIn INT, team_membersIn VARCHAR(500), userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    IF (EXISTS(SELECT * FROM game_teams WHERE team_id = team_idIn)) THEN
        UPDATE game_teams SET team_members = team_membersIn WHERE team_id = team_idIn;
        RETURN 0;
    END IF;
    RETURN 1;
END$$
DELIMITER ;

/**
 * setTeamName This sets the team_name for a given team_id in the game_teams table
 */
DROP FUNCTION IF EXISTS setTeamName;
DELIMITER $$
CREATE FUNCTION setTeamName(team_idIn INT, team_nameIn VARCHAR(500), userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    IF (EXISTS(SELECT * FROM game_teams WHERE team_id = team_idIn)) THEN
        UPDATE game_teams SET team_name = team_nameIn WHERE team_id = team_idIn;
        RETURN 0;
    END IF;
    RETURN 1;
END$$
DELIMITER ;

/**
 * setToken This adds an entry to the user_tokens table
 */
DROP FUNCTION IF EXISTS setToken;
DELIMITER $$
CREATE FUNCTION setToken(usernameIn VARCHAR(100), IPin VARCHAR(100), tokenIn VARCHAR(150)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    IF (EXISTS(SELECT * FROM users WHERE username = usernameIn)) THEN
        INSERT INTO user_tokens (token, username, ip_address, expire_time) VALUES (tokenIn, usernameIn, IPin, DATE_ADD(NOW(), INTERVAL 7 DAY));
        RETURN 0;
    ELSE
        RETURN 1;
    END IF;
END$$
DELIMITER ;

/**
 * setUserPerm This adds, or updates an entry in the user_permissions table
 */
DROP FUNCTION IF EXISTS setUserPerm;
DELIMITER $$
CREATE FUNCTION setUserPerm (target_username VARCHAR(200), perm_name VARCHAR(200), setting INT, userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    IF (EXISTS(SELECT * FROM permission_list WHERE permission_list.permission_id = perm_name) AND EXISTS(SELECT * FROM users WHERE users.username = target_username)) THEN
        IF (EXISTS(SELECT * FROM user_permissions WHERE user_permissions.permission = perm_name AND user_permissions.user = target_username)) THEN
            UPDATE user_permissions SET value = setting WHERE user_permissions.permission = perm_name AND user_permissions.user = target_username;
        ELSE
            INSERT INTO user_permissions (user, permission, value) VALUES(target_username, perm_name, setting);
        END IF;
        SET dummy = log_user_event(userIP, username, CONCAT("Permission ", perm_name , " for ", target_username, " was changed to ", CONVERT(setting, CHAR(50))));
        RETURN 0;
    ELSE
        RETURN 1;
    END IF;
END$$
DELIMITER ;

/**
 * setUserPreferences This updates an entry in the user_preferences table
 */
DROP FUNCTION IF EXISTS setUserPreferences;
DELIMITER $$
CREATE FUNCTION setUserPreferences(user_idIn INT, email_reg_dailyIn CHAR(1), email_reg_everyIn INT, email_merch_dailyIn CHAR(1), email_merch_everyIn INT, userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    IF (EXISTS(SELECT * FROM user_preferences WHERE user_id = user_idIn)) THEN
        UPDATE user_preferences SET user_preferences.email_reg_daily = email_reg_dailyIn, user_preferences.email_reg_every = email_reg_everyIn, user_preferences.email_merch_daily = email_merch_dailyIn, user_preferences.email_merch_every = email_merch_everyIn WHERE user_id = user_idIn;
        RETURN 0;
    END IF;
    RETURN 1;
END$$
DELIMITER ;

/**
 * setVariable This updates an entry in the vars table
 */
DROP FUNCTION IF EXISTS setVariable;
DELIMITER $$
CREATE FUNCTION setVariable(var_nameIn VARCHAR(200), var_valueIn VARCHAR(200), userIP VARCHAR(50), username VARCHAR(50), ignoreEditable INT) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    IF (EXISTS(SELECT * FROM vars WHERE name = var_nameIn)) THEN
        IF ((SELECT editable FROM vars WHERE name = var_nameIn) != 0 OR ignoreEditable != 0) THEN
            UPDATE vars SET vars.value = var_valueIn WHERE vars.name = var_nameIn;
            SET dummy = log_user_event(userIP, username, CONCAT("Variable ",  var_nameIn, " was updated to \"", var_valueIn, "\""));
            RETURN 0;
        END IF;
    END IF;
    RETURN 1;
END$$
DELIMITER ;

/**
 * setWebpageHTML This adds, or updates an entry in the webpages table, an entry is also added to the webpage_history table
 */
DROP FUNCTION IF EXISTS setWebpageHTML;
DELIMITER $$
CREATE FUNCTION setWebpageHTML(pathIn VARCHAR(100), page_nameIn VARCHAR(100), htmlIn LONGTEXT, userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    DECLARE page_idIn INT;
    SET page_idIn = 0;
    IF (EXISTS(SELECT * FROM webpages WHERE path = pathIn)) THEN
        SET page_idIn = (SELECT page_id FROM webpages WHERE path = pathIn);
    ELSE
        SET page_idIn = 0;
    END IF;
    IF (page_idIn > 0) THEN
        IF ((SELECT editable FROM webpages WHERE page_id = page_idIn) != 0) THEN
            UPDATE webpages SET page_name = page_nameIn, html = htmlIn WHERE path = pathIn;
            INSERT INTO webpage_history (page_id, timestamp, page_name, html) VALUES (page_idIn, NOW(), page_nameIn, htmlIn);
            SET dummy = log_user_event(userIP, username, CONCAT("Webpage ",  pathIn, " was updated"));
            RETURN 1;
        ELSE
            RETURN 2;
        END IF;
    ELSE
        INSERT INTO webpages (path, page_name, html) VALUES (pathIn, page_nameIn, htmlIn);
        SET dummy = log_user_event(userIP, username, CONCAT("Webpage ",  pathIn, " was created"));
        RETURN 0;
    END IF;
    RETURN 2;
END$$
DELIMITER ;

/**
 * setZonePoints This adds, updates, or deletes an entry in the zones table
 */
DROP FUNCTION IF EXISTS setZonePoints;
DELIMITER $$
CREATE FUNCTION setZonePoints(kml_nameIn VARCHAR(200), zone_nameIn VARCHAR(200), pointsIn INT, del INT, userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE dummy INT;
    IF (EXISTS(SELECT * FROM zones WHERE kml_file = kml_nameIn)) THEN
        IF (del = 0) THEN
            UPDATE zones SET name = zone_nameIn, points = pointsIn WHERE kml_file = kml_nameIn;
            SET dummy = log_user_event(userIP, username, CONCAT("Zone ",  kml_nameIn, " was updated to \"", zone_nameIn, "\" with ", CONVERT(pointsIn, CHAR(50)), " points"));
            RETURN 1;
        ELSE
            DELETE FROM zones WHERE kml_file = kml_nameIn;
            SET dummy = log_user_event(userIP, username, CONCAT("Zone ",  kml_nameIn, " was deleted"));
            RETURN 2;
        END IF;
    ELSE
        IF (del = 0) THEN
            INSERT INTO zones (kml_file, name, points) VALUES (kml_nameIn, zone_nameIn, pointsIn);
            SET dummy = log_user_event(userIP, username, CONCAT("Zone ",  kml_nameIn, " was created"));
            RETURN 3;
        ELSE
            RETURN 0;
        END IF;
    END IF;
    RETURN 0;
END$$
DELIMITER ;

/**
 * unsubEmail This sets an entry to unsubscribed in the email_list table
 */
DROP FUNCTION IF EXISTS unsubEmail;
DELIMITER $$
CREATE FUNCTION unsubEmail(token_unsub VARCHAR(150), userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE result INT;
    DECLARE dummy INT;
    SET result = (SELECT COUNT(*) FROM email_list WHERE unsub_token = token_unsub);
    IF (result = 1) THEN
        UPDATE email_list SET unsubscribed = 1, verify = 0 WHERE unsub_token = token_unsub;
        SET dummy = log_user_event(userIP, username, CONCAT("Email was unsubscribed (token: ",  token_unsub, " )"));
    END IF;
    RETURN result;
END$$
DELIMITER ;

/**
 * verifyEmail This sets an entry to verified in the email_list table
 */
DROP FUNCTION IF EXISTS verifyEmail;
DELIMITER $$
CREATE FUNCTION verifyEmail(token_verify VARCHAR(150), userIP VARCHAR(50), username VARCHAR(50)) RETURNS INT
    NOT DETERMINISTIC
BEGIN
    DECLARE result INT;
    SET result = (SELECT COUNT(*) FROM email_list WHERE verify_token = token_verify AND unsubscribed = 0);
    IF (result = 1) THEN
        UPDATE email_list SET verify = 1 WHERE verify_token = token_verify;
    END IF;
    RETURN result;
END$$
DELIMITER ;

--
-- End of Functions
--
