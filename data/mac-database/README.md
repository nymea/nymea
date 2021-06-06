# Building the MAC address database

The MAC address database can be created using the `build-database.py` script. The script will download the latest registered MAC address block information from [https://macaddress.io](https://macaddress.io) and creates a SQLITE database file.

The generated database is read performance optimized and tried to keep as small as possible for searching MAC address OUIs (Organizationally Unique Identifiers) blocks and returning the registered company name.

    $ python3 build-database.py

The final database will be named `mac-addresses.db`.

In nymea the `MacAddressDatabase` will search by default for this database file in `/usr/share/nymea/mac-addresses.db` and provides an asynch threaded mechanism to get the company name for a given MAC address.

