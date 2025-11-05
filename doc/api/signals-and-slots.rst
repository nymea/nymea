Signals and slots
=================

Many parts of the nymea APIs rely on Qt's signals and slots for asynchronous
communication. When using libnymea or writing plugins, ensure your classes
follow these best practices:

* Derive from ``QObject`` when emitting signals.
* Use the ``Q_OBJECT`` macro in class definitions.
* Prefer the function pointer syntax when connecting signals and slots.

Example:

.. code-block:: cpp

   connect(device, &nymea::Device::stateChanged,
           this, &MyController::handleStateChanged);

Signals mirror the events emitted by nymead, giving you real-time updates without
polling the server.
