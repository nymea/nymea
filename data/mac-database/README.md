# MAC address dabase

The MAC address database has been created using the dowloaded JSON data from [https://macaddress.io](https://macaddress.io).

## Build database

Before you can start the `build-database.py` script, please make sure you downloaded the the online database in JSON format.

Once the `macaddress.io-db.json` file has been downloaded and placed into this folder, the python tool can be started in order to generate a read performance optimized and minimal database for searching mac address OUIs (Organizationally Unique Identifiers).

    $ python3 build-database.py

The final database will be named `mac-addresses.db`.

In nymea the `MacAddressDatabase` will search for this database file in `/usr/share/nymea/mac-addresses.db` and provides an asynch mechanism to provide the manufacturer name for a given mac address.

