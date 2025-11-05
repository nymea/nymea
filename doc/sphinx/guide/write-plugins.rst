Write your own plugin
=====================

Developing a nymea device plugin unlocks the full power of the rule engine,
logging infrastructure and user interfaces that ship with the platform. Focus on
the integration logic and let nymea take care of client communication, logging
and persistence.

If you are new to plugin development follow the :doc:`setup-environment` and
:doc:`getting-started` guides first. Afterwards use the following references while
building your plugin:

* :doc:`plugin-json` – how to describe your plugin, vendors and device classes in
  the JSON manifest.
* :doc:`create-setup-methods` – how discovery and setup workflows are expressed
  through create and setup methods.
* :doc:`reference/interfaces` – reusable runtime interfaces for accessing
  hardware resources.
* :doc:`reference/jsonrpc` – communication interface used by nymea clients.

Community support is available on the `nymea forum <https://forum.nymea.io>`_.
