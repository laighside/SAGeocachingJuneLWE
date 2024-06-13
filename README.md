# SA Geocaching June LWE website
This repository contains the source code for the SA Geocaching June Long Weekend website - [jlwe.org](https://jlwe.org/)

## Introduction
The JLWE website has a number of purposes. The public side of the website promotes and distributes information about the event (including maps and GPX files), collects registration info (including payment) from attendees, sells event merchandise and provides a mailing list for attendees to subscribe to. The admin side of the website provides a number of tools to assist in running the event and associated games. There is also an archive of content from past events.

It is designed to run on a Linux server with Apache HTTP Server and a MySQL database.

The website components are arranged as follows:
- Dynamic content is located in `src`, is written in C++ and needs compiling before installing in the cgi-bin directory
- Static content is located in `html`, and includes images, CSS and JavaScript used on the website
- MySQL database schema and functions are in `mysql`

## Dependencies
The follow software and libraries are required to make the website work.

These software packages are used on the server:
- Apache HTTP Server
- MySQL Database
- Postfix mail server

These libraries are required for building the software:
- [curl](https://github.com/curl/curl)
- [MySQL Connector C++](https://github.com/mysql/mysql-connector-cpp)
- [MaxMind DB](https://github.com/maxmind/libmaxminddb) (optional)

These 3rd party libraries are included in this repository (in the `src/ext` directory)
- [nlohmann JSON](https://github.com/nlohmann/json)
- [Duthomhas CSPRNG](https://github.com/Duthomhas/CSPRNG)
- [pugixml](https://pugixml.org/)
- [md4c](https://github.com/mity/md4c)
- [Openwall bcrypt](https://www.openwall.com/crypt/)
- [OpenXLSX](https://github.com/troldal/OpenXLSX)
- [Stephan Brumme's C++ Hashing Library](https://github.com/stbrumme/hash-library)

3rd party JavaScript libraries: (included in the `html/js/ext` directory)
- [otto-dev coordinate parser](https://github.com/otto-dev/coordinate-parser)
- [Leaflet](https://leafletjs.com/)
- [Lightbox by Lokesh Dhakar](https://lokeshdhakar.com/projects/lightbox2/)

Command line tools required:
- [Imagemagick](https://www.imagemagick.org/) `convert`
- [zip](https://linux.die.net/man/1/zip)

## Building and configuring
First install and setup Apache HTTP Server and MySQL database server.

Then clone this repo:
```
git clone https://github.com/laighside/SAGeocachingJuneLWE.git
```

### Config file
The default location for the config file is `/etc/jlwe/jlwe.json` (however this can be changed by defining `CONFIG_FILE` when running cmake)
Copy the `config_sample.json` file to this location and edit it as required:
```
cp config_sample.json /etc/jlwe/jlwe.json
```

A directory must be created for storing files uploaded to the file manager. This directory must have read/write access for the Apache user (usually `www-data`). The path for this directory is then entered into the config file in `files -> directory`

#### Templates directory

The `templates` folder contains files used by the CGI scripts for creating Office Open XML files (docx, xlsx, pptx). The `ooxmlTemplatePath` item in the config file must contain the full path to the `templates` folder. The Apache user must have read permissions for this folder.

### Static content
The default location for this is usually `/var/www/html/` however any directory can be used. This location is set by `DocumentRoot` in the Apache config.

Copy the contents of the `html` folder to this directory:
```
cp -R html /var/www/html
```

Within this directory, is a folder called `img/uploads/` which is used for storing images uploaded by users. The `img/uploads/` directory must have read/write access for the Apache user.

### CGI scripts
The default location for this is usually `/usr/lib/cgi-bin/` however any directory can be used. This location is set by `ScriptAlias` in the Apache config, and by defining `CGI_BIN_DIR` when running cmake.

Build the CGI scripts:
```
mkdir build
cd build
cmake ../src/ -DCGI_BIN_DIR=/usr/lib/cgi-bin/ -DCONFIG_FILE=/etc/jlwe/jlwe.json
make
```
(the `CONFIG_FILE` option can be omitted if the default location is used, it is shown above as an example of usage)

### MySQL
1. Create a new database
   - Make sure the charset is `utf8mb4` and the collation is `utf8mb4_0900_ai_ci` (this allows full unicode support)
2. Import the contents of `tables.sql` and `functions.sql` into this new database
3. Create a user in MySQL to be used by Apache to access the database
4. Grant this user `SELECT` and `EXECUTE` privileges for the database
5. Enter the database name, username and password into the `/etc/jlwe/jlwe.json` config file

### Apache config
The Apache config varies depending on how the server is setup. The following things are required for the JLWE website:
- CGI must be enabled (`mod_cgi`)
- `mod_rewrite` must be enabled
- `DocumentRoot` set
- `ScriptAlias` set for /cgi-bin/ (for example: `ScriptAlias /cgi-bin/ /usr/lib/cgi-bin/`)

### Website login
The default username is `admin` and the default password is `password`. This password should be changed immediately. The change password link is in the top right of every webpage (when logged in).

On the first use, you also need to reset the number of game caches to a non-zero value (which also populates the cache handout table). This should be done by going to `/cgi-bin/admin/admin_tools.cgi`

There are also many other variables to configure at `/cgi-bin/settings/settings.cgi`

## Changelog
- 2017 *(First year with website)*
   - Mailing list & file manager
   - Game caches on Google Maps
- 2018
   - Added GPX file download
- 2019
   - Added GPX builder
   - Added Powerpoint builder
- 2020 *(Event not held)*
   - Moved to jlwe.org domain
   - Added event, dinner and camping registrations
   - Made public side of website editable by all admins
- 2021
   - Added cache hide form for people to upload their own cache details
   - Added merchandise store
- 2022
   - Removed merchandise store
   - Added cache handout records
   - Added scoring page (only cache hiding points)
   - Added notes page
- 2023
   - Created public Github repo
- 2024
   - Added scoring spreadsheet download and upload
   - Added public photo upload page
