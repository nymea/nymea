JSON-RPC API
============

The JSON-RPC API is the primary remote interface for nymea clients. It is
accessible over secure WebSockets at ``wss://<host>:4443/jsonrpc``.

Authentication
--------------

Clients authenticate using nymea user credentials or OAuth tokens provided by a
plugin. After authentication a session token is issued for subsequent calls.

Calling methods
---------------

Methods are namespaced, e.g. ``Devices.GetDevices``. The following example lists
all devices:

.. code-block:: json

   {
     "id": 42,
     "jsonrpc": "2.0",
     "method": "Devices.GetDevices"
   }

Notifications
-------------

State changes are broadcast as JSON-RPC notifications. Clients should subscribe
to relevant events immediately after connecting to avoid missing updates.
