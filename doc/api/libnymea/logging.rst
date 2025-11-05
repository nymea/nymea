Logging
=======

``libnymea/logging`` exposes client side helpers around the nymea logging
infrastructure.

LogEntry and LogEntries
-----------------------

``LogEntry`` encapsulates a single log record.  It stores the timestamp,
logging category, message, severity, optional ``ThingId`` reference and the
raw JSON payload.  ``LogEntries`` is the ``QList`` wrapper returned by the log
engine.

LogEngine
---------

``LogEngine`` manages streaming and fetching logs from ``nymead``.  Use
``fetch()`` to create a ``LogFetchJob`` that downloads a slice of the history
and ``startStreaming()`` / ``stopStreaming()`` to subscribe to live updates.
The engine emits ``entryAdded`` whenever a new ``LogEntry`` arrives and tracks
per-category enable flags.

``LogFetchJob`` wraps an in-progress fetch request, exposes the ``entries()``
it retrieved and notifies listeners through ``finished``/``failed`` signals.

Logger
------

``Logger`` is a facade for storing new log entries.  It forwards messages to a
``LogEngine`` instance and ensures the shared buffering rules are respected.
Plugins typically rely on the logging categories defined in
``loggingcategories.h``.

