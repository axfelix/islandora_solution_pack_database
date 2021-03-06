Database Solution Pack
======================

Introduction
------------

Loads all required Fedora Objects, sets up docker, and creates an empty collection object to accept ingested SQL dumps as databases.

Requirements
------------

This module requires the following modules/libraries:

-   [Islandora](<https://github.com/islandora/islandora>)

-   [Tuque](<https://github.com/islandora/tuque>)

-   [docker](<https://www.docker.com/>)

-   [git](<http://git-scm.com/>) - Debian/Ubuntu `sudo apt-get install git`

Installation
------------

Run docker\_setup.sh as your webserver user before installation, then install as usual, see [this](<https://drupal.org/documentation/install/modules-themes/modules-7>) for further information.

Configuration
-------------

There are currently no configuration parameters.

Known Issues
------------

-   Database names can conflict, though this is fairly rare due to the way most DB tools handle import/export.

-   This module does a probably-appalling amount of shelling out to grep and sed and whatnot. Significant parts would have to be rewritten for it to be "cleaner" / more pure PHP / theoretically work on Windows. I will never do this.

-   Non-public objects are not really supported since the Adminer instances running in the docker container don't know anything about the Fedora auth restrictions.


Maintainers/Sponsors
--------------------

Current maintainers:

-   [Alex Garnett](<https://github.com/axfelix>)

License
-------

[GPLv3](<http://www.gnu.org/licenses/gpl-3.0.txt>)
