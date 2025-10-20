JSON-RPC helpers
================

``libnymea/jsonrpc`` contains reusable building blocks for serving the
nymea JSON-RPC API from plugins or auxiliary services.

JsonHandler
-----------

``JsonHandler`` is the core registration facility.  Subclasses override
``name()`` and call ``registerEnum()``, ``registerObject()`` and
``registerMethod()`` to describe their namespace.  The handler keeps track of
registered enums, flags, objects, methods and notifications so clients can
introspect the API via ``jsonEnums()``, ``jsonMethods()`` and related helpers.

``translateNotification()`` gives handlers a chance to localise notifications
based on the target ``QLocale``.  ``createReply()`` and ``createAsyncReply()``
produce ``JsonReply`` instances with the correct bookkeeping information.

JsonReply
---------

``JsonReply`` wraps a method invocation.  It stores the request ``id()``,
``method()`` name, the ``params()`` map and emits ``finished`` / ``error`` once
processing completes.  ``setResult()`` and ``setError()`` allow handlers to
signal success or failure.

JsonRPCServer
-------------

``JsonRPCServer`` dispatches incoming requests to registered ``JsonHandler``
instances.  Use ``registerHandler()`` to expose a handler, ``requestReceived``
to hook into the transport layer and ``sendNotification()`` to push events to
connected clients.  The server keeps a ``JsonContext`` describing the caller's
identity and permissions.

JsonContext
-----------

``JsonContext`` represents the execution context (authentication state, user
id, locale, permissions) for a request.  Handlers can read it to enforce access
checks or personalise responses.

