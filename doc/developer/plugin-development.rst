Plugin development
==================

Plugins extend nymea with support for new devices or services. Each plugin is a
Qt plugin that implements one or more device classes.

Project layout
--------------

A typical plugin repository has the following structure:

.. code-block:: text

   myplugin/
     CMakeLists.txt
     plugin.json
     myplugin.h/.cpp
     translations/

Use the :command:`nymea-plugin-template` tool to bootstrap a new plugin if you
prefer starting from a working baseline.

Device and action types
-----------------------

nymea defines device, action, and event types centrally. Declare them in
``plugin.json`` with unique identifiers to avoid collisions. The :doc:`../api/libnymea`
section covers how to register types programmatically.

Testing plugins
---------------

* Implement unit tests using Qt Test and add them to the ``tests`` directory.
* Run the plugin inside a development nymead instance before shipping.

Publishing
----------

Open a merge request in the nymea repository or publish the plugin separately
and submit it to the nymea plugin catalogue.
