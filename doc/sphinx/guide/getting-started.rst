Getting started with plugin development
=======================================

Plugins extend the nymea server with support for new devices, services or
integrations. A plugin is a shared library that the server loads during start-up.
Each plugin exposes vendors, device classes and the corresponding runtime
interfaces that describe how devices behave.

Devices
-------

Plugins can represent hardware devices, gateways or online services. When
planning a plugin consider the following concepts:

* :ref:`Param types <jsonrpc>` describe configuration data that is provided during
  setup (for example IP addresses or identifiers). Params are immutable after the
  initial configuration.
* Settings types define runtime configuration options that can be changed while a
  device is active. A typical example is the polling interval for a web service.
* State types describe live values such as the current temperature or whether a
  device is on or off.
* Event types represent signals emitted by a device. For instance a button press
  or a motion detection event.
* Action types expose executable commands on the device. Actions can accept
  parameters to change how they are executed.

Hardware resources
------------------

``libnymea`` provides a range of reusable :doc:`reference/interfaces` that
encapsulate access to hardware resources such as GPIOs, Bluetooth adapters and
network services. Select the resources your plugin depends on and interact with
them through the provided interfaces rather than implementing the low level
communication yourself.

Next steps
----------

Continue with the :doc:`plugin-json` guide to learn how plugins are described via
JSON manifests, then read :doc:`create-setup-methods` for a deeper look at the
setup workflow.
