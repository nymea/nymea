Troubleshooting
===============

Common issues encountered during installation and the first run are listed
below.

Service does not start
----------------------

Check the system journal for detailed logs:

.. code-block:: bash

   sudo journalctl -u nymead

Missing plugins
---------------

Ensure the nymea plugins package is installed. On Debian-based systems run
``apt install nymea-plugins``.

Connectivity issues
-------------------

* Confirm the firewall allows access to TCP port 4443 and UDP port 5678.
* Verify that client devices trust the nymea TLS certificate or install a
  certificate issued by your organisation.
