.. _jsonrpc:

JSON-RPC API
============

The JSON-RPC interface represents a persistent socket connection that is used by
clients to communicate with the nymea server. Messages are exchanged as UTF-8
encoded JSON objects terminated by a newline character (``\n``). The transport
channel is not HTTP based; instead, the socket offers an internal RPC mechanism
between the daemon and controller front-ends. Exposing the socket directly to
untrusted networks is discouraged because it allows issuing privileged commands.

Communicating with the server
-----------------------------

Each message is either a request, a response or a notification. The sections
below describe their structure in detail.

Request message
^^^^^^^^^^^^^^^

.. code-block:: json

   {
       "id": 1,
       "method": "Namespace.Method",
       "o:token": "string",
       "o:params": { }
   }

``id`` values should be unique per connection. The ``method`` string contains
both the namespace and the method name separated by a dot. Parameters prefixed
with ``o:`` are optional.

.. list-table:: Request message fields
   :widths: 12 10 10 68
   :header-rows: 1

   * - Field
     - Required
     - Type
     - Description
   * - ``id``
     - Mandatory
     - Integer
     - Unique identifier that is echoed in the response.
   * - ``method``
     - Mandatory
     - String
     - Namespaced method to invoke, e.g. ``JSONRPC.Introspect``.
   * - ``token``
     - Optional
     - String
     - Authentication token acquired through the authentication workflow.
   * - ``params``
     - Optional
     - Object
     - Method specific payload.

Response message
^^^^^^^^^^^^^^^^

.. code-block:: json

   {
       "id": 1,
       "status": "success",
       "o:params": { },
       "o:error": "string"
   }

.. list-table:: Response message fields
   :widths: 12 10 10 68
   :header-rows: 1

   * - Field
     - Required
     - Type
     - Description
   * - ``id``
     - Mandatory
     - Integer
     - Matches the originating request ``id``.
   * - ``status``
     - Mandatory
     - String
     - ``"success"``, ``"error"`` or ``"unauthorized"``.
   * - ``params``
     - Optional
     - Object
     - Method specific response data.
   * - ``error``
     - Optional
     - String
     - Transport level error description when ``status`` is ``"error"`` or
       ``"unauthorized"``.

Handshake
^^^^^^^^^

After establishing the socket the client must perform a handshake using
``JSONRPC.Hello``. The call returns core metadata about the server and the
connection. A typical interaction looks as follows:

.. code-block:: json

   {
       "id": 0,
       "method": "JSONRPC.Hello",
       "params": {
           "locale": "de_DE"
       }
   }

.. code-block:: json

   {
       "id": 0,
       "status": "success",
       "params": {
           "name": "My nymea",
           "protocol version": "2.0",
           "authenticationRequired": false,
           "initialSetupRequired": false,
           "pushButtonAuthAvailable": false,
           "server": "nymea",
           "locale": "de_DE",
           "uuid": "{42842b0f-a7bb-4a94-b624-a55f31c5603e}",
           "version": "0.12.1"
       }
   }

.. list-table:: Handshake result fields
   :widths: 22 12 66
   :header-rows: 1

   * - Field
     - Type
     - Description
   * - ``name``
     - String
     - Friendly name of the server instance.
   * - ``protocol version``
     - String
     - Major/minor API version, e.g. ``2.0``.
   * - ``authenticationRequired``
     - Boolean
     - ``True`` when authentication must be performed before other commands.
   * - ``initialSetupRequired``
     - Boolean
     - ``True`` if the server has not completed its initial setup.
   * - ``pushButtonAuthAvailable``
     - Boolean
     - ``True`` if the push-button authentication agent is available.
   * - ``server``
     - String
     - Identifier for the server type.
   * - ``locale``
     - String
     - Locale applied to the connection.
   * - ``uuid``
     - String
     - Unique identifier of the server instance.
   * - ``version``
     - String
     - Build version of the running daemon.

The handshake can be repeated at any time to refresh server properties or to
change the connection locale.

Sending a request
^^^^^^^^^^^^^^^^^

After the handshake, subsequent RPC calls follow the same request/response
pattern. JSON payloads should be transmitted without whitespace and must be
terminated by ``\n``. For example:

.. code-block:: json

   {"id":122,"method":"JSONRPC.KeepAlive","params":{"sessionId":"my-session"}}

The server replies with a compact JSON object, also terminated by ``\n``:

.. code-block:: json

   {"id":122,"params":{"success": true, "sessionId": "my-session"}}

Notifications
^^^^^^^^^^^^^

Notifications are disabled by default. Clients can enable them with
``JSONRPC.SetNotificationStatus``. Once enabled, the server sends asynchronous
messages that follow this structure:

.. code-block:: json

   {
       "id": 42,
       "notification": "Namespace.Notification",
       "o:params": { }
   }

.. list-table:: Notification fields
   :widths: 18 12 12 58
   :header-rows: 1

   * - Field
     - Required
     - Type
     - Description
   * - ``id``
     - Mandatory
     - Integer
     - Monotonically increasing identifier per connection.
   * - ``notification``
     - Mandatory
     - String
     - Namespaced notification that was emitted.
   * - ``params``
     - Optional
     - Object
     - Payload specific to the notification.

Authentication
--------------

The API supports authentication to prevent unauthorized access. Tokens are
attached to requests via the optional ``token`` property. Authentication should
be performed over an encrypted channel.

A small number of methods can be called without a token:

* ``JSONRPC.Hello``
* ``JSONRPC.Introspect``
* ``JSONRPC.CreateUser`` (when the system has not been set up)
* ``JSONRPC.RequestPushButtonAuth``
* ``JSONRPC.Authenticate``

Two authentication flows are available: username/password and push-button.

Username and password
^^^^^^^^^^^^^^^^^^^^^^

If ``initialSetupRequired`` is ``true`` the server has no user account. Create
one by calling ``JSONRPC.CreateUser``. Afterwards obtain a token via
``JSONRPC.Authenticate`` using the same credentials. Tokens should be stored
alongside the server UUID and can be reused until revoked by either the client or
server.

Push-button authentication
^^^^^^^^^^^^^^^^^^^^^^^^^^

When available, push-button authentication restricts token issuance to clients
with physical access to the device.

1. Request a session using ``JSONRPC.RequestPushButtonAuth`` and provide a
   descriptive client name.
2. Wait for the user to press the physical button and handle the
   ``JSONRPC.PushButtonAuthFinished`` notification. The notification contains the
   transaction identifier and the issued token.
3. Reuse the token for subsequent requests.

Removing a token
^^^^^^^^^^^^^^^^

To invalidate a client token call ``JSONRPC.RemoveToken``. The associated client
must re-authenticate before issuing further requests.

Machine readable API description
--------------------------------

The JSON-RPC API is self-describing. Call ``JSONRPC.Introspect`` at runtime to
obtain the same data that is published in :doc:`reference/generated/jsonrpc-api`.
