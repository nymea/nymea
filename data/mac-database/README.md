# Building the MAC address database

The MAC address database can be created using the `build-database.py` script.

There are 2 online services supported:

* [https://maclookup.app](https://maclookup.app) default
* [https://macaddress.io](https://macaddress.io) not free any more, but still supported

The script will download the latest registered MAC address block information
from [https://maclookup.app](https://maclookup.app) and creates a size and access optimized
SQLITE database file.

The generated database is read performance optimized and tried to keep as small as possible for
searching MAC address OUIs (Organizationally Unique Identifiers) blocks and returning the registered company name.

    $ python3 build-database.py

The final database will be named `mac-addresses.db`.

In nymea the `MacAddressDatabase` class will provide access to this generated database and provides an asynch threaded mechanism to get the company name for a given MAC address.

The database will be searched in the system default data location `${XDG_DATA_DIRS}/nymead/`.

On debian package based system the database file will be installed in `/usr/share/nymea/nymead/mac-addresses.db`.

