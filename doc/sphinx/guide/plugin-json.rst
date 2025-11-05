Plugin JSON manifest
====================

Each device plugin ships with a JSON manifest that describes the plugin, the
vendors it supports and all exposed device classes. The manifest is processed by
the ``nymea-plugininfocompiler`` tool, which generates ``plugininfo.h`` and
``extern-plugininfo.h`` headers. These headers contain strongly typed UUIDs for
plugins, vendors, device classes and types as well as the logging category used
by the plugin implementation.

Basic layout
------------

The manifest file follows the ``deviceplugin<PluginName>.json`` naming scheme.
A minimal structure looks like this:

.. code-block:: json

   {
       "name": "PluginName",
       "displayName": "Name of the plugin",
       "id": "uuid",
       "vendors": [
           {
               "name": "VendorName",
               "displayName": "Name of the vendor",
               "id": "uuid",
               "thingClasses": [
                   {
                       "name": "ThingClassName",
                       "displayName": "Device class",
                       "id": "uuid"
                   }
               ]
           }
       ]
   }

Use ``uuidgen`` to create UUIDs for every ``id`` field in the document.

Plugin definition
-----------------

The root object describes the plugin itself:

.. list-table:: Plugin properties
   :header-rows: 1
   :widths: 20 15 65

   * - Key
     - Required
     - Description
   * - ``id``
     - Yes
     - UUID of the plugin (``DevicePlugin::pluginId()``).
   * - ``name``
     - Yes
     - Identifier used to generate strongly typed IDs and the logging category
       (``dc<Name>``).
   * - ``displayName``
     - Yes
     - Localisable, user facing name of the plugin.
   * - ``paramTypes``
     - No
     - Global :ref:`Param types <jsonrpc>` provided by the plugin.
   * - ``vendors``
     - Yes
     - Array of vendor definitions (see below).

Vendor definition
-----------------

Each vendor entry defines a ``Vendor`` object and references the device classes
it owns:

.. list-table:: Vendor properties
   :header-rows: 1
   :widths: 20 15 65

   * - Key
     - Required
     - Description
   * - ``id``
     - Yes
     - UUID of the vendor (``Vendor::id()``).
   * - ``name``
     - Yes
     - Identifier used by the generated headers (``<name>VendorId``).
   * - ``displayName``
     - Yes
     - Localisable label presented to users.
   * - ``thingClasses``
     - Yes
     - Array of device class definitions.

Device class definition
-----------------------

A device class describes how nymea should create, configure and interact with a
particular type of device. The following properties are commonly used:

.. list-table:: Device class properties
   :header-rows: 1
   :widths: 24 15 61

   * - Key
     - Required
     - Description
   * - ``id``
     - Yes
     - UUID of the device class (``ThingClass::id()``).
   * - ``name``
     - Yes
     - Identifier used to generate ``<name>DeviceClassId``.
   * - ``displayName``
     - Yes
     - Localisable name for the device class.
   * - ``createMethods``
     - No
     - Array of creation methods offered by the class.
   * - ``setupMethod``
     - No
     - Name of the default setup method.
   * - ``interfaces``
     - No
     - List of interfaces implemented by the device class.
   * - ``pairingInfo``
     - No
     - Human readable instructions shown during pairing.
   * - ``discoveryParamTypes`` / ``paramTypes``
     - No
     - Parameters required during discovery or configuration.
   * - ``stateTypes`` / ``actionTypes`` / ``eventTypes``
     - No
     - Behavioural types exposed by the device class.

For an exhaustive description of all keys inspect the generated manifest schema
in :doc:`reference/generated/jsonrpc-api` or review the existing plugin
manifests in ``libnymea``.
