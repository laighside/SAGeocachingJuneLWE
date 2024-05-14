-- Tables schema for JLWE database

-- This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
-- https://github.com/laighside/SAGeocachingJuneLWE

-- MySQL dump 10.13  Distrib 5.7.38, for Linux (x86_64)
-- Server version	8.0.29-0ubuntu0.20.04.3

--
-- Table structure for table `admin_notes`
--

DROP TABLE IF EXISTS `admin_notes`;
CREATE TABLE `admin_notes` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `timestamp` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `user_id` int NOT NULL,
  `markdown` longtext NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `best_caches`
--

DROP TABLE IF EXISTS `best_caches`;
CREATE TABLE `best_caches` (
  `id` int NOT NULL AUTO_INCREMENT,
  `dsp_order` int NOT NULL,
  `title` varchar(100) NOT NULL,
  `cache` varchar(100) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id_UNIQUE` (`id`),
  UNIQUE KEY `dsp_order_UNIQUE` (`dsp_order`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `best_caches`
--

LOCK TABLES `best_caches` WRITE;
INSERT INTO `best_caches` VALUES (6,1,'Best Natural',''),(7,2,'Most Entertaining',''),(8,3,'Most Creative',''),(9,4,'Best Unnatural',''),(10,5,'Best Overall','');
UNLOCK TABLES;

--
-- Table structure for table `blocked_ips`
--

DROP TABLE IF EXISTS `blocked_ips`;
CREATE TABLE `blocked_ips` (
  `ip` varchar(50) NOT NULL,
  PRIMARY KEY (`ip`),
  UNIQUE KEY `ip_UNIQUE` (`ip`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `cache_handout`
--

DROP TABLE IF EXISTS `cache_handout`;
CREATE TABLE `cache_handout` (
  `cache_number` int NOT NULL,
  `owner_name` varchar(500) NOT NULL DEFAULT '',
  `returned` int NOT NULL DEFAULT '0',
  `team_id` int NOT NULL DEFAULT '-1',
  PRIMARY KEY (`cache_number`),
  UNIQUE KEY `id_UNIQUE` (`cache_number`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `caches`
--

DROP TABLE IF EXISTS `caches`;
CREATE TABLE `caches` (
  `cache_number` int unsigned NOT NULL,
  `cache_name` varchar(100) NOT NULL DEFAULT '',
  `team_name` varchar(100) NOT NULL DEFAULT '',
  `latitude` double NOT NULL DEFAULT '0',
  `longitude` double NOT NULL DEFAULT '0',
  `public_hint` varchar(1000) NOT NULL DEFAULT '',
  `detailed_hint` varchar(1000) NOT NULL DEFAULT '',
  `camo` tinyint NOT NULL DEFAULT '0',
  `permanent` tinyint NOT NULL DEFAULT '0',
  `private_property` tinyint NOT NULL DEFAULT '0',
  `zone_bonus` int NOT NULL DEFAULT '0',
  `osm_distance` int NOT NULL DEFAULT '0',
  `actual_distance` int NOT NULL DEFAULT '-1',
  PRIMARY KEY (`cache_number`),
  UNIQUE KEY `cache_number_UNIQUE` (`cache_number`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `camping`
--

DROP TABLE IF EXISTS `camping`;
CREATE TABLE `camping` (
  `registration_id` int NOT NULL AUTO_INCREMENT,
  `ip_address` varchar(50) DEFAULT NULL,
  `timestamp` datetime DEFAULT NULL,
  `idempotency` varchar(100) NOT NULL,
  `email_address` varchar(500) NOT NULL,
  `gc_username` varchar(1000) NOT NULL,
  `phone_number` varchar(50) NOT NULL,
  `livemode` tinyint NOT NULL,
  `number_people` int NOT NULL DEFAULT '1',
  `camping_type` varchar(50) NOT NULL DEFAULT 'unpowered',
  `arrive_date` int NOT NULL DEFAULT '0',
  `leave_date` int NOT NULL DEFAULT '0',
  `camping_comment` varchar(1000) NOT NULL DEFAULT '',
  `payment_type` varchar(10) NOT NULL DEFAULT 'cash',
  `stripe_session_id` varchar(200) DEFAULT NULL,
  `status` char(1) NOT NULL DEFAULT 'P',
  PRIMARY KEY (`registration_id`),
  UNIQUE KEY `idempotency_UNIQUE` (`idempotency`),
  UNIQUE KEY `registration_id_UNIQUE` (`registration_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `camping_options`
--

DROP TABLE IF EXISTS `camping_options`;
CREATE TABLE `camping_options` (
  `id` int NOT NULL AUTO_INCREMENT,
  `id_string` varchar(20) NOT NULL,
  `display_name` text NOT NULL,
  `price_code` text NOT NULL,
  `total_available` int DEFAULT NULL,
  `active` int NOT NULL DEFAULT '1',
  `display_comment` text,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id_UNIQUE` (`id`),
  UNIQUE KEY `id_string_UNIQUE` (`id_string`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `camping_options`
--

LOCK TABLES `camping_options` WRITE;
INSERT INTO `camping_options` VALUES (1,'unpowered','Unpowered site','unpowered',NULL,1),(2,'powered','Powered site','powered',10,1);
UNLOCK TABLES;

--
-- Table structure for table `contact_form`
--

DROP TABLE IF EXISTS `contact_form`;
CREATE TABLE `contact_form` (
  `id` int NOT NULL AUTO_INCREMENT,
  `ip_address` varchar(50) DEFAULT NULL,
  `timestamp` datetime DEFAULT NULL,
  `from_name` text,
  `email_address` varchar(500) NOT NULL,
  `message` text,
  `status` char(1) NOT NULL DEFAULT 'P',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `csp_reports`
--

DROP TABLE IF EXISTS `csp_reports`;
CREATE TABLE `csp_reports` (
  `id` int NOT NULL AUTO_INCREMENT,
  `time_received` datetime NOT NULL,
  `IP_address` varchar(50) NOT NULL,
  `user_agent` varchar(500) NOT NULL,
  `content` longtext,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id_UNIQUE` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `dinner_menu`
--

CREATE TABLE `dinner_menu` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(100) NOT NULL,
  `name_plural` varchar(100) NOT NULL,
  `price` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `dinner_menu`
--

LOCK TABLES `dinner_menu` WRITE;
INSERT INTO `dinner_menu` VALUES (1, 'Adult meal', 'Adult meals', 0),(2, 'Child meal', 'Child meals', 0);
UNLOCK TABLES;

--
-- Table structure for table `dinner_menu_options`
--

CREATE TABLE `dinner_menu_options` (
  `id` int NOT NULL AUTO_INCREMENT,
  `menu_item_id` int NOT NULL,
  `display_order` int NOT NULL DEFAULT '1',
  `name` varchar(100) NOT NULL,
  `question` varchar(100) NOT NULL,
  `option_type` varchar(50) DEFAULT NULL,
  `option_values` text,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `email_forwarders`
--

DROP TABLE IF EXISTS `email_forwarders`;
CREATE TABLE `email_forwarders` (
  `source` varchar(500) NOT NULL,
  `destination` varchar(500) NOT NULL,
  PRIMARY KEY (`source`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `email_forwarders`
--

LOCK TABLES `email_forwarders` WRITE;
INSERT INTO `email_forwarders` VALUES ('admin',''),('contact','');
UNLOCK TABLES;

--
-- Table structure for table `email_list`
--

DROP TABLE IF EXISTS `email_list`;
CREATE TABLE `email_list` (
  `email` varchar(200) NOT NULL,
  `verify` tinyint NOT NULL DEFAULT '0',
  `verify_token` varchar(150) NOT NULL,
  `unsub_token` varchar(150) NOT NULL,
  `unsubscribed` tinyint NOT NULL DEFAULT '0',
  PRIMARY KEY (`email`),
  UNIQUE KEY `email_UNIQUE` (`email`),
  UNIQUE KEY `verify_token_UNIQUE` (`verify_token`),
  UNIQUE KEY `unsub_token_UNIQUE` (`unsub_token`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `event_registrations`
--

DROP TABLE IF EXISTS `event_registrations`;
CREATE TABLE `event_registrations` (
  `registration_id` int NOT NULL AUTO_INCREMENT,
  `ip_address` varchar(50) DEFAULT NULL,
  `timestamp` datetime DEFAULT NULL,
  `idempotency` varchar(100) NOT NULL,
  `email_address` varchar(500) NOT NULL,
  `gc_username` varchar(1000) NOT NULL,
  `phone_number` varchar(50) NOT NULL,
  `livemode` tinyint NOT NULL,
  `real_names_adults` varchar(500) DEFAULT NULL,
  `real_names_children` varchar(500) DEFAULT NULL,
  `number_adults` int NOT NULL DEFAULT '1',
  `number_children` int NOT NULL DEFAULT '0',
  `past_jlwe` tinyint NOT NULL DEFAULT '0',
  `have_lanyard` tinyint NOT NULL DEFAULT '0',
  `camping` varchar(10) NOT NULL DEFAULT 'no',
  `dinner` varchar(10) NOT NULL DEFAULT 'no',
  `payment_type` varchar(10) NOT NULL DEFAULT 'cash',
  `stripe_session_id` varchar(200) DEFAULT NULL,
  `status` char(1) NOT NULL DEFAULT 'P',
  PRIMARY KEY (`registration_id`),
  UNIQUE KEY `idempotency_UNIQUE` (`idempotency`),
  UNIQUE KEY `registration_id_UNIQUE` (`registration_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `file_downloads`
--

DROP TABLE IF EXISTS `file_downloads`;
CREATE TABLE `file_downloads` (
  `filename` text NOT NULL,
  `timestamp` datetime NOT NULL,
  `user_ip` varchar(50) NOT NULL,
  `user_agent` text NOT NULL,
  `response_code` int DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `files`
--

DROP TABLE IF EXISTS `files`;
CREATE TABLE `files` (
  `filename` varchar(500) NOT NULL,
  `directory` varchar(500) NOT NULL DEFAULT '/',
  `size` int unsigned NOT NULL DEFAULT '0',
  `owner` varchar(100) DEFAULT NULL,
  `year` int NOT NULL,
  `date_uploaded` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `public` tinyint NOT NULL DEFAULT '0',
  PRIMARY KEY (`filename`),
  UNIQUE KEY `filename_UNIQUE` (`filename`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `game_find_points_extras`
--

DROP TABLE IF EXISTS `game_find_points_extras`;
CREATE TABLE `game_find_points_extras` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` text NOT NULL,
  `point_value` int NOT NULL DEFAULT '1',
  `enabled` int NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `game_find_points_trads`
--

DROP TABLE IF EXISTS `game_find_points_trads`;
CREATE TABLE `game_find_points_trads` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` text NOT NULL,
  `enabled` int NOT NULL DEFAULT '1',
  `hide_or_find` char(1) NOT NULL DEFAULT 'F',
  `config` text,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `game_find_points_trads`
--

LOCK TABLES `game_find_points_trads` WRITE;
INSERT INTO `game_find_points_trads` VALUES (1,'1 point per cache', 1, 'F', '[]'),(2,'Walking points', 1, 'F', '{\"distance\":100,\"max_points\":4}'),(3, 'Zone points', 1, 'H', NULL),(4, 'Creative hide points', 0, 'H', '{\"points\":1}');
UNLOCK TABLES;

--
-- Table structure for table `game_teams`
--

DROP TABLE IF EXISTS `game_teams`;
CREATE TABLE `game_teams` (
  `team_id` int NOT NULL AUTO_INCREMENT,
  `team_name` varchar(100) NOT NULL,
  `team_members` varchar(500) DEFAULT NULL,
  `competing` tinyint NOT NULL DEFAULT '1',
  `final_score` int DEFAULT NULL,
  PRIMARY KEY (`team_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `login_attempts`
--

DROP TABLE IF EXISTS `login_attempts`;
CREATE TABLE `login_attempts` (
  `attempt_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `ip_address` varchar(100) NOT NULL,
  `username` varchar(100) NOT NULL,
  `correct` int NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `merch_groups`
--

DROP TABLE IF EXISTS `merch_groups`;
CREATE TABLE `merch_groups` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(1000) NOT NULL,
  `description` longtext NOT NULL,
  `images` longtext NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `merch_item_options`
--

DROP TABLE IF EXISTS `merch_item_options`;
CREATE TABLE `merch_item_options` (
  `id` int NOT NULL AUTO_INCREMENT,
  `merch_item_id` int NOT NULL,
  `option_name` varchar(100) DEFAULT NULL,
  `option_question` varchar(1000) DEFAULT NULL,
  `option_type` varchar(100) NOT NULL,
  `option_values` varchar(1000) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `merch_items`
--

DROP TABLE IF EXISTS `merch_items`;
CREATE TABLE `merch_items` (
  `id` int NOT NULL AUTO_INCREMENT,
  `group_id` int NOT NULL,
  `name` varchar(1000) NOT NULL,
  `name_plural` varchar(1000) NOT NULL,
  `description` longtext NOT NULL,
  `images` longtext NOT NULL,
  `logo_url` varchar(500) NOT NULL,
  `cost` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `merch_order_items`
--

DROP TABLE IF EXISTS `merch_order_items`;
CREATE TABLE `merch_order_items` (
  `order_item_id` int NOT NULL AUTO_INCREMENT,
  `order_id` int NOT NULL,
  `item_id` int NOT NULL,
  `item_options_str` varchar(1000) NOT NULL,
  PRIMARY KEY (`order_item_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `merch_orders`
--

DROP TABLE IF EXISTS `merch_orders`;
CREATE TABLE `merch_orders` (
  `order_id` int NOT NULL AUTO_INCREMENT,
  `ip_address` varchar(50) DEFAULT NULL,
  `timestamp` datetime DEFAULT NULL,
  `idempotency` varchar(100) NOT NULL,
  `email_address` varchar(500) NOT NULL,
  `gc_username` varchar(1000) NOT NULL,
  `phone_number` varchar(50) NOT NULL,
  `livemode` tinyint NOT NULL,
  `payment_type` varchar(10) NOT NULL DEFAULT 'cash',
  `stripe_session_id` varchar(200) DEFAULT NULL,
  `status` char(1) NOT NULL DEFAULT 'P',
  PRIMARY KEY (`order_id`),
  UNIQUE KEY `idempotency_UNIQUE` (`idempotency`),
  UNIQUE KEY `registration_id_UNIQUE` (`order_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `payment_log`
--

DROP TABLE IF EXISTS `payment_log`;
CREATE TABLE `payment_log` (
  `id` int NOT NULL AUTO_INCREMENT,
  `user_key` varchar(100) NOT NULL,
  `timestamp` int NOT NULL,
  `amount_received` int NOT NULL DEFAULT '0',
  `payment_type` varchar(10) DEFAULT NULL,
  `source_user` int DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id_UNIQUE` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `permission_list`
--

DROP TABLE IF EXISTS `permission_list`;
CREATE TABLE `permission_list` (
  `permission_id` varchar(100) NOT NULL,
  `permission_name` varchar(100) NOT NULL,
  PRIMARY KEY (`permission_id`),
  UNIQUE KEY `permission_id_UNIQUE` (`permission_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `permission_list`
--

LOCK TABLES `permission_list` WRITE;
INSERT INTO `permission_list` VALUES ('perm_admin','Admin'),('perm_email','Send Emails'),('perm_email_forward','Setup Email Forwarding'),('perm_file','Upload Files'),('perm_gpxbuilder','GPX Builder'),('perm_merch','Merchandise Orders'),('perm_pptbuilder','PPT Builder'),('perm_registrations','Event Registrations'),('perm_website_edit','Edit Website');
UNLOCK TABLES;

--
-- Table structure for table `powerpoint_slides`
--

DROP TABLE IF EXISTS `powerpoint_slides`;
CREATE TABLE `powerpoint_slides` (
  `id` int NOT NULL,
  `slide_order` int DEFAULT NULL,
  `type` varchar(50) DEFAULT NULL,
  `title` varchar(100) DEFAULT NULL,
  `enabled` int DEFAULT '1',
  `content` text,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `powerpoint_slides`
--

LOCK TABLES `powerpoint_slides` WRITE;
INSERT INTO `powerpoint_slides` VALUES (1,1,'welcome','Welcome slide',1,NULL),(2,2,'naga','NAGA',1,NULL),(3,5,'disqualified','Disqualified',1,NULL),(4,21,'winner','Winner',1,NULL),(5,22,'runnerup','Runner-up',1,NULL),(6,23,'leaderboard','Final Leaderboard',1,NULL),(7,19,'scores','Places 3-5',1,'3-5'),(8,16,'scores','Places 6-10',1,'6-10'),(9,13,'scores','Places 11-15',1,'11-15'),(10,11,'scores','Places 16-20',1,'16-20'),(11,8,'scores','Places 21-25',1,'21-25'),(12,6,'scores','Places 26-30',1,'26-30'),(13,4,'scores','Places 31-35',1,'31-35'),(14,3,'scores','Places 36-40',1,'36-40'),(15,17,'best_caches','Best Caches',1,NULL),(16,14,'rising_star','Rising Star Award',1,NULL),(17,20,'generic','Good Samaritans',1,'Person 1\n - What they did\nPerson 2\n - What they did'),(18,9,'generic','Freddo\'s cache',1,''),(19,7,'generic','Scavenger Hunt',1,'Winner 1...\nWinner 2...'),(20,12,'generic','Everyone Stand Up...',1,''),(21,18,'generic','Other Prizes',1,'Best costume\nBest table\nAny other prizes'),(22,10,'generic','Closest to GZ',1,''),(23,15,'generic','Junior Achiever Award',1,'');
UNLOCK TABLES;

--
-- Table structure for table `public_file_upload`
--

DROP TABLE IF EXISTS `public_file_upload`;
CREATE TABLE `public_file_upload` (
  `id` int NOT NULL AUTO_INCREMENT,
  `guid` text NOT NULL,
  `cache_number` int NOT NULL DEFAULT '0',
  `public_filename` text,
  `server_filename` text NOT NULL,
  `timestamp` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `file_size` int NOT NULL DEFAULT '0',
  `user_ip` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id_UNIQUE` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `sat_dinner`
--

DROP TABLE IF EXISTS `sat_dinner`;
CREATE TABLE `sat_dinner` (
  `registration_id` int NOT NULL AUTO_INCREMENT,
  `ip_address` varchar(50) DEFAULT NULL,
  `timestamp` datetime DEFAULT NULL,
  `idempotency` varchar(100) NOT NULL,
  `email_address` varchar(500) NOT NULL,
  `gc_username` varchar(1000) NOT NULL,
  `phone_number` varchar(50) NOT NULL,
  `livemode` tinyint NOT NULL,
  `number_adults` int NOT NULL DEFAULT '1',
  `number_children` int NOT NULL DEFAULT '0',
  `dinner_comment` varchar(1000) NOT NULL DEFAULT '',
  `payment_type` varchar(10) NOT NULL DEFAULT 'cash',
  `stripe_session_id` varchar(200) DEFAULT NULL,
  `status` char(1) NOT NULL DEFAULT 'P',
  `dinner_options_adults` text,
  `dinner_options_children` text,
  PRIMARY KEY (`registration_id`),
  UNIQUE KEY `idempotency_UNIQUE` (`idempotency`),
  UNIQUE KEY `registration_id_UNIQUE` (`registration_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `stripe_card_fees`
--

DROP TABLE IF EXISTS `stripe_card_fees`;
CREATE TABLE `stripe_card_fees` (
  `idempotency` varchar(100) NOT NULL,
  `fee` int NOT NULL,
  PRIMARY KEY (`idempotency`),
  UNIQUE KEY `idempotency_UNIQUE` (`idempotency`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `stripe_event_log`
--

DROP TABLE IF EXISTS `stripe_event_log`;
CREATE TABLE `stripe_event_log` (
  `id` varchar(100) NOT NULL,
  `timestamp` int NOT NULL,
  `livemode` tinyint NOT NULL,
  `type` varchar(100) NOT NULL,
  `api_version` varchar(100) NOT NULL,
  `payment_intent` varchar(100) DEFAULT NULL,
  `cs_id` varchar(100) DEFAULT NULL,
  `amount` int DEFAULT NULL,
  `amount_received` int DEFAULT NULL,
  `message` varchar(1000) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id_UNIQUE` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `user_hidden_caches`
--

DROP TABLE IF EXISTS `user_hidden_caches`;
CREATE TABLE `user_hidden_caches` (
  `id_number` int unsigned NOT NULL AUTO_INCREMENT,
  `team_name` varchar(100) NOT NULL DEFAULT '',
  `phone_number` varchar(50) DEFAULT NULL,
  `cache_number` int unsigned NOT NULL,
  `cache_name` varchar(100) NOT NULL DEFAULT '',
  `latitude` double NOT NULL DEFAULT '0',
  `longitude` double NOT NULL DEFAULT '0',
  `public_hint` varchar(1000) NOT NULL DEFAULT '',
  `detailed_hint` varchar(1000) NOT NULL DEFAULT '',
  `camo` tinyint NOT NULL DEFAULT '0',
  `permanent` tinyint NOT NULL DEFAULT '0',
  `private_property` tinyint NOT NULL DEFAULT '0',
  `zone_bonus` int NOT NULL DEFAULT '0',
  `osm_distance` int NOT NULL DEFAULT '0',
  `actual_distance` int NOT NULL DEFAULT '-1',
  `status` char(1) NOT NULL DEFAULT 'R',
  `timestamp` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `IP_address` varchar(50) NOT NULL,
  PRIMARY KEY (`id_number`),
  UNIQUE KEY `id_number_UNIQUE` (`id_number`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `user_log`
--

DROP TABLE IF EXISTS `user_log`;
CREATE TABLE `user_log` (
  `timestamp` int NOT NULL,
  `userIP` varchar(50) NOT NULL,
  `username` varchar(50) NOT NULL DEFAULT '',
  `action` text NOT NULL
) ENGINE=CSV DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `user_permissions`
--

DROP TABLE IF EXISTS `user_permissions`;
CREATE TABLE `user_permissions` (
  `user` varchar(100) NOT NULL,
  `permission` varchar(100) NOT NULL,
  `value` tinyint NOT NULL,
  PRIMARY KEY (`user`,`permission`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `user_permissions`
--

LOCK TABLES `user_permissions` WRITE;
INSERT INTO `user_permissions` VALUES ('admin','perm_admin',1),('admin','perm_email',1),('admin','perm_email_forward',1),('admin','perm_file',1),('admin','perm_gpxbuilder',1),('admin','perm_merch',1),('admin','perm_pptbuilder',1),('admin','perm_registrations',1),('admin','perm_website_edit',1);
UNLOCK TABLES;

--
-- Table structure for table `user_preferences`
--

DROP TABLE IF EXISTS `user_preferences`;
CREATE TABLE `user_preferences` (
  `user_id` int NOT NULL,
  `email_reg_daily` char(1) NOT NULL DEFAULT 'N',
  `email_reg_every` int NOT NULL DEFAULT '0',
  `email_merch_daily` char(1) NOT NULL DEFAULT 'N',
  `email_merch_every` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  UNIQUE KEY `user_id_UNIQUE` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `user_preferences`
--

LOCK TABLES `user_preferences` WRITE;
INSERT INTO `user_preferences` VALUES (1,'N',0,'N',0);
UNLOCK TABLES;

--
-- Table structure for table `user_tokens`
--

DROP TABLE IF EXISTS `user_tokens`;
CREATE TABLE `user_tokens` (
  `token` varchar(150) NOT NULL,
  `username` varchar(100) NOT NULL,
  `ip_address` varchar(45) NOT NULL,
  `expire_time` datetime NOT NULL,
  PRIMARY KEY (`token`),
  UNIQUE KEY `token_UNIQUE` (`token`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
CREATE TABLE `users` (
  `user_id` int NOT NULL,
  `username` varchar(100) NOT NULL,
  `email` varchar(200) DEFAULT NULL,
  `pass_hash` varchar(100) NOT NULL,
  `active` tinyint DEFAULT '1',
  `reset_token` varchar(150) NOT NULL,
  PRIMARY KEY (`user_id`),
  UNIQUE KEY `username_UNIQUE` (`username`),
  UNIQUE KEY `user_id_UNIQUE` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;


--
-- Dumping data for table `users`
--

LOCK TABLES `users` WRITE;
INSERT INTO `users` VALUES (1,'admin','','$2a$12$zA8RevMuBNDzKxuCrC4r7eFWDcrfP5RwjhjnLTBhFEHskM0fssKEy',1,'');
UNLOCK TABLES;

--
-- Table structure for table `vars`
--

DROP TABLE IF EXISTS `vars`;
CREATE TABLE `vars` (
  `name` varchar(100) NOT NULL,
  `value` varchar(1000) NOT NULL DEFAULT '',
  `editable` tinyint NOT NULL,
  `comment` varchar(500) DEFAULT NULL,
  PRIMARY KEY (`name`),
  UNIQUE KEY `name_UNIQUE` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `vars`
--

LOCK TABLES `vars` WRITE;
INSERT INTO `vars` VALUES
  ('admin_index_md','',0,NULL),
  ('bank_details','',0,'The bank details that attendees are asked to make payment to. These details will be shown on the website and on invoice emails when attendees select to pay by bank transfer.'),
  ('camping_cutoff_date','0',1,'The time/date that camping bookings close on. Users will not be able to book campsites after this date has passed.'),
  ('contact_email','',1,'When anyone fills out the contact page on the website, this is where the emails will go. About 90% of these are spam.'),
  ('default_coordinates','S00° 00.000 E000° 00.000',1,NULL),
  ('dinner_cutoff_date','0',1,'The time/date that dinner bookings close on. Users will not be able to order dinner after this date has passed.'),
  ('event_caches_gpx','',1,'A GPX file with the events and attended logs from every year. This is used to generate the attendance statistics.'),
  ('gpx_code_prefix','',1,'The prefix to use on the cache codes in the game GPX file (the prefix GC is used on real caches). This needs to be unique for each year, and don’t use GC, GA or TP.'),
  ('gpx_country','Australia',1,'This is what the country field in the game GPX file is set to.'),
  ('gpx_state','South Australia',1,'This is what the state field in the game GPX file is set to.'),
  ('jlwe_date','0',1,'The date of the June LWE (either the Saturday or Sunday).'),
  ('map_type','google',1,'The map provider to use on the website. The options are google or leaflet.'),
  ('merch_cutoff_date','0',0,'The time/date that merchandise orders close on. Users will not be able to order merchandise after this date has passed.'),
  ('number_game_caches','0',0,'The maximum number of caches in the game. Changing this value requires the cache handout table to be cleared.'),
  ('osm_roads_kml','',1,'This is the KML file of road data that is used for calculating the distance each cache is from the road. We usually download the road data from Open Street Map (OSM).'),
  ('playing_field_kml','',1,'This is a KML that contains the outline of the playing area. It is shown on maps and used to check that caches are within the playing area.'),
  ('powered_camping_sites','10',1,NULL),
  ('ppt_town','',1,'The town name shown on the PowerPoint presentation.'),
  ('registration_enabled','0',1,'When this is set to \"No\" anyone trying to register will be told that registrations are closed. Use this to turn off the registration form after the event is over.'),
  ('registration_open_date','0',1,'The date/time that registrations open on. The registration form will only be visible to admins before this date.');
UNLOCK TABLES;

--
-- Table structure for table `webpage_history`
--

DROP TABLE IF EXISTS `webpage_history`;
CREATE TABLE `webpage_history` (
  `page_id` int NOT NULL,
  `timestamp` datetime NOT NULL,
  `page_name` varchar(100) NOT NULL,
  `html` longtext NOT NULL,
  PRIMARY KEY (`page_id`,`timestamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `webpage_images`
--

DROP TABLE IF EXISTS `webpage_images`;
CREATE TABLE `webpage_images` (
  `filename` varchar(100) NOT NULL,
  `file_size` int unsigned NOT NULL DEFAULT '0',
  `owner` varchar(100) NOT NULL,
  `date_uploaded` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`filename`),
  UNIQUE KEY `filename_UNIQUE` (`filename`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Table structure for table `webpage_menu`
--

DROP TABLE IF EXISTS `webpage_menu`;
CREATE TABLE `webpage_menu` (
  `link_text` varchar(50) NOT NULL,
  `link_url` varchar(100) NOT NULL DEFAULT '',
  `parent` varchar(50) NOT NULL DEFAULT '',
  `menu_order` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`link_text`),
  UNIQUE KEY `link_text_UNIQUE` (`link_text`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `webpage_menu`
--

LOCK TABLES `webpage_menu` WRITE;
INSERT INTO `webpage_menu` VALUES ('About','','*root*',4),('Contact','/contact.html','About',4),('Event Details','','*root*',2),('Event Registration','/register','Event Details',2),('Event Schedule','/schedule.html','Event Details',1),('Game Day GPX File','/gpx','Game Details',6),('Game Details','','*root*',3),('Guidelines for game caches','/guidelines.html','Game Details',2),('June LWE Home','/index.html','*root*',1),('Mailing List Signup','/email','Game Details',5),('Merchandise','','Event Details',5),('Photo Gallery','/gallery.html','About',2),('Playing Field Map','/game_map.html','Game Details',4),('Sponsors','/sponsors.html','About',5),('Who we are','/committee.html','About',3);
UNLOCK TABLES;

--
-- Table structure for table `webpages`
--

DROP TABLE IF EXISTS `webpages`;
CREATE TABLE `webpages` (
  `page_id` int NOT NULL AUTO_INCREMENT,
  `path` varchar(100) NOT NULL,
  `page_name` varchar(100) NOT NULL,
  `html` longtext NOT NULL,
  `login_only` tinyint NOT NULL DEFAULT '0',
  `editable` tinyint NOT NULL DEFAULT '1',
  `special_page` tinyint NOT NULL,
  `draft_page` tinyint NOT NULL DEFAULT '0',
  PRIMARY KEY (`page_id`),
  UNIQUE KEY `page_id_UNIQUE` (`page_id`)
) ENGINE=InnoDB AUTO_INCREMENT=21 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `webpages`
--

LOCK TABLES `webpages` WRITE;
INSERT INTO `webpages` VALUES
  (1,'/about.html','About','<h1>About the June LWE Event</h1>',0,1,0),
  (2,'/committee.html','Who we are','<h1>Who we are</h1>',0,1,0),
  (3,'/contact.html','Contact Us','<h1>Contact us</h1>\n        <form id=\"contact_form\" action=\"/cgi-bin/contact.cgi\" method=\"POST\">\n            <table border=\"0\" style=\"margin-left:auto;margin-right:auto;\">\n                    <tr>\n                      <td><span style=\"float:right\">Your Name:</span></td>\n                      <td><input type=\"text\" name=\"name\"></td>\n                    </tr>\n                    <tr>\n                      <td><span style=\"float:right\">Your Email address:</span></td>\n                      <td><input type=\"text\" name=\"email\"></td>\n                    </tr>\n                    <tr>\n                      <td><span style=\"float:right\">Message:</span></td>\n                      <td><textarea name=\"message\" rows=\"6\" cols=\"30\"></textarea></td>\n                    </tr>\n                    <tr>\n                      <td colspan=\"2\" align=\"center\"><div class=\"g-recaptcha\" data-sitekey=\"6LcSsbsUAAAAAJSnbvPpdUVmE-vwEYPVwjNSpIDA\"></div></td>\n                    </tr>\n                    <tr>\n                      <td colspan=\"2\" align=\"center\"><input type=\"submit\" value=\"Submit\"></td>\n                    </tr>\n            </table>\n        </form>\n\n<script src=\"https://www.google.com/recaptcha/api.js\" async defer></script>\n<script>\nvar form = document.getElementById(\'contact_form\');\nform.addEventListener(\"submit\", function(event){\n    if (grecaptcha.getResponse() === \'\') {                            \n      event.preventDefault();\n      alert(\'Please check the recaptcha\');\n    }\n  }\n, false);\n</script>',0,1,0),
  (4,'/gallery.html','JLWE Photo Gallery','<h1 style=\"text-align:center;\">June LWE Gallery</h1>',0,1,0),
  (5,'/game_map.html','Playing Field Map','<h1>Playing Field Map</h1>',0,1,0),
  (6,'/guidelines.html','Guidelines for game caches','<h1>Guidelines for June LWE Cache Placements</h1>',0,1,0),
  (7,'/index.html','June LWE Home','<h1>SA Geocaching June Long Weekend Event</h1>',0,1,0),
  (8,'/login.html','Admin Login','      <h1>Admin login</h1>\n        <form action=\"/cgi-bin/login.cgi\" method=\"POST\" style=\"overflow:auto;\">\n            <table border=\"0\" align=\"left\" style=\"border:0\">\n                    <tr style=\"border:0\">\n                      <td style=\"border:0\"><span style=\"float:right\">Username:</span></td>\n                      <td style=\"border:0\"><input type=\"text\" name=\"username\"></td>\n                    </tr>\n                    <tr style=\"border:0\">\n                      <td style=\"border:0\"><span style=\"float:right\">Password:</span></td>\n                      <td style=\"border:0\"><input type=\"password\" name=\"password\"></td>\n                    </tr>\n                    <tr style=\"border:0\">\n                      <td colspan=\"2\" align=\"center\" style=\"border:0\"><input type=\"submit\" value=\"Submit\"></td>\n                    </tr>\n            </table>\n        </form>\n\n<p style=\"text-align:right;\"><a href=\"/password_reset.html\">Forget your password? Click here to reset it</a></p>',0,0,0),
  (9,'/merchandise.html','JLWE Merchandise','<h1>Merchandise</h1>',0,1,0),
  (10,'/schedule.html','Event Schedule','<h1 style=\"text-align:center;\">Event Schedule</h1>',0,1,0),
  (11,'/cache_list.html','Download Cache List','      <h1>Download June LWE cache lists</h1>\n        <p><button onclick=\"location.href=\'/cgi-bin/gpx_builder/gpx_builder.cgi\'\" type=\"button\">Return to cache list</button></p>\n        <form action=\"/cgi-bin/gpx_builder/download_cache_list.cgi\" style=\"float:left; margin:10px;\">\n            <input name=\"type\" value=\"coord\" hidden />\n            <table border=\"0\" align=\"left\" style=\"border:0\">\n              <tr><th colspan=\"2\">Names and Coordinates</th></tr>\n                    <tr style=\"border:0\">\n                      <td><span style=\"float:right\">Format:</span></td>\n                      <td><select name=\"format\">\n                        <option value=\"word\">MS Word (.docx)</option>\n                      </select></td>\n                    </tr>\n                    <tr>\n                      <td><span style=\"float:right\">Options:</span></td>\n                      <td>\n                      <input type=\"checkbox\" name=\"all_caches\" value=\"true\" checked>Include unallocated caches<br/>\n                      <input type=\"checkbox\" name=\"rot13\" value=\"true\" checked>Encode hints (ROT13)<br/>\n                      Page size:\n                      <select name=\"page_size\">\n                        <option value=\"a4\" selected>A4</option>\n                        <option value=\"a3_portrait\">A3 (Portrait)</option>\n                        <option value=\"a3_landscape\">A3 (Landscape)</option>\n                      </select>\n                      </td>\n                    </tr>\n                    <tr>\n                      <td><span style=\"float:right\">Columns:</span></td>\n                      <td>\n                      <input type=\"checkbox\" name=\"cache_code\" value=\"true\" checked />Cache code/number<br/>\n                      <input type=\"checkbox\" name=\"cache_name\" value=\"true\" checked />Cache name<br/>\n                      <input type=\"checkbox\" name=\"team_name\" value=\"true\" checked />Team name<br/>\n                      <input type=\"checkbox\" name=\"location\" value=\"true\" checked />Coordinates<br/>\n                      <input type=\"checkbox\" name=\"public_hint\" value=\"true\" checked />Public hint<br/>\n                      <input type=\"checkbox\" name=\"detailed_hint\" value=\"true\" />Detailed hint<br/>\n                      <input type=\"checkbox\" name=\"camo\" value=\"true\" />Creative/camouflaged hide<br/>\n                      <input type=\"checkbox\" name=\"perm\" value=\"true\" />Will become a permanent cache<br/>\n                      <input type=\"checkbox\" name=\"zone_bonus\" value=\"true\" />Zone bonus points<br/>\n                      <input type=\"checkbox\" name=\"walking\" value=\"true\" />Walking distance<br/>\n                      </td>\n                    </tr>\n                    <tr>\n                      <td colspan=\"2\" align=\"center\"><input type=\"submit\" value=\"Download\"></td>\n                    </tr>\n            </table>\n        </form>\n        <form action=\"/cgi-bin/gpx_builder/download_cache_list.cgi\" style=\"float:left; margin:10px;\">\n            <input name=\"type\" value=\"owner\" hidden />\n            <table border=\"0\" align=\"left\" style=\"border:0\">\n              <tr><th colspan=\"2\">Owner List</th></tr>\n                    <tr style=\"border:0\">\n                      <td><span style=\"float:right\">Format:</span></td>\n                      <td><select name=\"format\">\n                        <option value=\"word\">MS Word (.docx)</option>\n                      </select></td>\n                    </tr>\n                    <tr>\n                      <td><span style=\"float:right\">Options:</span></td>\n                      <td>\n                      <input type=\"checkbox\" name=\"sort_by_name\" value=\"true\" />Sort caches by owner name<br/>\n                      Page size:\n                      <select name=\"page_size\" disabled>\n                        <option value=\"a4\" selected>A4</option>\n                        <option value=\"a3_portrait\">A3 (Portrait)</option>\n                        <option value=\"a3_landscape\">A3 (Landscape)</option>\n                      </select>\n                      </td>\n                    </tr>\n                    <tr>\n                      <td colspan=\"2\" align=\"center\"><input type=\"submit\" value=\"Download\"></td>\n                    </tr>\n            </table>\n        </form>',1,0,0),
  (12,'/email_signup.html','Email Signup','      <h1>June LWE email list signup</h1>\n\n      <p>If you would like to receive the June LWE game day GPX file via email, please enter your email address below. A confirmation email will be sent to the address you provide. If you wish to also receive the GPX file via Memory card or USB transfer, please fill out one of the paper forms provided on the day.</p>\n\n        <form id=\"email_form\" action=\"/cgi-bin/mailing_list/email_signup.cgi\" method=\"POST\">\n        <p>Email Address:</p>\n        <div class=\"margin\"><input type=\"email\" name=\"email\" /></div>\n        <div class=\"margin\"><div class=\"g-recaptcha\" data-sitekey=\"6LcSsbsUAAAAAJSnbvPpdUVmE-vwEYPVwjNSpIDA\"></div></div>\n        <div class=\"margin\"><input type=\"submit\" value=\"Submit\" style=\"float:left;width:100px;\" /></div>\n        </form>\n\n<script src=\"https://www.google.com/recaptcha/api.js\" async defer></script>\n<script>\nvar form = document.getElementById(\'email_form\');\nform.addEventListener(\"submit\", function(event){\n    if (grecaptcha.getResponse() === \'\') {                            \n      event.preventDefault();\n      alert(\'Please check the recaptcha\');\n    }\n  }\n, false);\n</script>\n',0,0,0),
  (13,'*camping_registration','','',0,1,1),
  (14,'*event_registration','','',0,1,1),
  (15,'*dinner_registration','','',0,1,1),
  (16,'*merchandise','','',0,1,1),
  (17,'/change_password.html','Change Password','<h1>Change Password</h1>\n<form action=\"/cgi-bin/password/change.cgi\" method=\"POST\">\n    <table border=\"0\" align=\"left\" style=\"border:0\">\n            <tr style=\"border:0\">\n              <td style=\"border:0\"><span style=\"float:right\">Old Password:</span></td>\n              <td style=\"border:0\"><input name=\"old_password\" type=\"password\"></td>\n            </tr>\n            <tr style=\"border:0\">\n              <td style=\"border:0\"><span style=\"float:right\">New Password:</span></td>\n              <td style=\"border:0\"><input name=\"new_password\" type=\"password\"></td>\n            </tr>\n            <tr style=\"border:0\">\n              <td colspan=\"2\" align=\"center\" style=\"border:0\"><input type=\"submit\" value=\"Submit\"></td>\n            </tr>\n    </table>\n</form>\n',1,0,0),
  (18,'/password_reset.html','Reset Password','      <h1>Reset Password</h1>\n<p>Enter your email below and a password reset link will be sent to your inbox.</p>\n        <form action=\"/cgi-bin/password/send_reset_email.cgi\" method=\"POST\">\n            <table border=\"0\" align=\"left\" style=\"border:0\">\n                    <tr style=\"border:0\">\n                      <td style=\"border:0\"><span style=\"float:right\">Email:</span></td>\n                      <td style=\"border:0\"><input type=\"email\" name=\"email\"></td>\n                    </tr>\n                    <tr style=\"border:0\">\n                      <td colspan=\"2\" align=\"center\" style=\"border:0\"><input type=\"submit\" value=\"Submit\"></td>\n                    </tr>\n            </table>\n        </form>',0,0,0),
  (19,'/sponsors.html','Sponsors','<h1>June LWE Sponsors</h1>',0,1,0),
  (20,'/upload','Send us your photos','<h1>Send us your photos</h1>\n<p>Upload photos of your cache hides, or any other photos you take during the event!</p>\n<p>Accepted formats: JPEG, PNG, GIF, HEIC (size limit is 10MB)</p>\n<form id=\"upload_form\" action=\"/cgi-bin/files/public_upload.cgi\" method=\"POST\" enctype=\"multipart/form-data\">\n<table border=\"0\" align=\"left\" style=\"border:0;\">\n<tr><td style=\"border:0;padding:10px;\"><label for=\"cache_number\">Cache number:</label>\n<input type=\"number\" id=\"cache_number\" name=\"cache_number\" min=\"0\" max=\"100\" value=\"0\" /><br />(leave as zero if your photo isn\'t of a game cache)</td></tr>\n<tr><td style=\"border:0\"><input type=\"file\" name=\"files\" multiple=\"true\" required=\"true\" style=\"font-size:20px;padding:10px;\" /></td></tr>\n<tr><td style=\"border:0\" align=\"center\"><div class=\"g-recaptcha\" data-sitekey=\"6LcSsbsUAAAAAJSnbvPpdUVmE-vwEYPVwjNSpIDA\"></div></td></tr>\n<tr><td style=\"border:0\"><input type=\"submit\" value=\"Submit\" style=\"width:100%\"></td></tr>\n</table>\n</form>\n\n<script src=\"https://www.google.com/recaptcha/api.js\" async defer></script>\n<script>\nvar form = document.getElementById(\'upload_form\');\nform.addEventListener(\"submit\", function(event){\n    if (grecaptcha.getResponse() === \'\') {                            \n      event.preventDefault();\n      alert(\'Please check the recaptcha\');\n    }\n  }\n, false);\n</script>',0,1,0);
UNLOCK TABLES;

--
-- Table structure for table `zones`
--

DROP TABLE IF EXISTS `zones`;
CREATE TABLE `zones` (
  `id` int NOT NULL AUTO_INCREMENT,
  `kml_file` varchar(100) NOT NULL,
  `name` varchar(100) NOT NULL DEFAULT '',
  `points` int NOT NULL DEFAULT '0',
  `enabled` int NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

---
--- End of Tables
---
