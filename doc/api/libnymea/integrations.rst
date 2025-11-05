Integrations framework
======================

.. _libnymea-integrations-plugin:

IntegrationPlugin
-----------------

``IntegrationPlugin`` is the base class for every nymea integration.  Plugins
inherit from it, declare their :ref:`PluginMetadata
<libnymea-integrations-plugin-metadata>` and implement the discovery, pairing
and runtime hooks that ``ThingManager`` invokes.

**Lifecycle hooks**
  * ``init()`` — optional setup after the plugin object is constructed.
  * ``startMonitoringAutoThings()`` — begin background scans for auto-created
    things.
  * ``discoverThings(ThingDiscoveryInfo *info)`` — populate
    ``ThingDescriptor`` instances for the requested ``ThingClass``.
  * ``setupThing(ThingSetupInfo *info)`` and ``postSetupThing(Thing *thing)`` —
    configure the actual device.
  * ``thingRemoved(Thing *thing)`` — release any resources tied to the thing.

**Pairing and configuration**
  * ``startPairing()`` and ``confirmPairing()`` coordinate secure workflows
    for devices that need out-of-band confirmation.
  * ``configurationDescription()``, ``configuration()``, ``configValue()`` and
    ``setConfigValue()`` expose persistent plugin configuration.

**Runtime interactions**
  * ``executeAction()``, ``browseThing()``, ``browserItem()`` and their
    companion methods handle user initiated actions.
  * ``serviceInformation()`` returns auxiliary services such as REST or MQTT
    endpoints described by :ref:`ServiceData
    <libnymea-integrations-service-data>`.

Plugins emit ``emitEvent`` for event propagation, ``configValueChanged`` when
configuration mutates, and ``autoThingsAppeared`` / ``autoThingDisappeared``
when passive discovery detects devices.

``IntegrationPlugins`` is a thin ``QList`` wrapper that provides ``findById``
and QML friendly ``get``/``put`` helpers.

.. _libnymea-integrations-thing-manager:

ThingManager
------------

``ThingManager`` is the runtime interface offered by ``nymead``.  Plugin code
never instantiates it directly; ``nymead`` injects a pointer during plugin
initialisation.  The manager is responsible for:

* Listing available plugins, vendors, interfaces and thing classes.
* Tracking configured things and looking them up by id, class or interface.
* Starting discovery, setup, reconfiguration and removal transactions through
  ``ThingDiscoveryInfo``, ``ThingSetupInfo`` and ``ThingPairingInfo``.
* Executing actions, browsing media hierarchies and managing IO connections.
* Translating metadata into a user facing locale.

``ThingManager`` emits signals for every state change (thing added/removed,
state changes, ioConnection updates, executed actions) and forwards events from
``Thing`` instances via ``eventTriggered``.

.. _libnymea-integrations-thing:

Thing and Things
----------------

A ``Thing`` represents a configured device.  It exposes its identifier,
``ThingClass``, user facing name, static ``ParamList`` data, mutable settings,
current ``States`` and logging configuration.  Key helpers include
``stateValue()``, ``setStateValue()``, ``setting()``, ``setSettingValue()`` and
``emitEvent()``.  ``ThingSetupStatus`` keeps track of onboarding progress and
``ThingError`` enumerates the full error space shared across the integrations
stack.

``Things`` is a ``QList<Thing*>`` façade with ``get``/``put`` helpers for QML
views.

.. _libnymea-integrations-descriptors:

Descriptors and transactions
----------------------------

``ThingDescriptor`` describes a concrete device candidate discovered during
scanning.  It bundles the ``ThingClass`` metadata, unique ids, optional parent
relationships and default parameters.  ``ThingDescriptors`` is the list type
returned from discovery jobs.

``ThingDiscoveryInfo`` provides context to ``discoverThings()`` calls.  Plugins
populate it with descriptors, use ``finish()`` / ``setError()`` to signal
completion and read configuration hints through ``params()``.

``ThingSetupInfo`` and ``ThingPairingInfo`` behave similarly for setup and
pairing transactions.  Both expose ``setThing()`` to hand back the final
``Thing`` or ``setError()`` to abort.  ``ThingPairingInfo`` additionally tracks
``PairingTransactionId`` values so plugins can resume multi step workflows.

``ThingActionInfo`` mirrors the structure for action execution.  Plugins call
``setFinished()``/``setError()`` or stream progress via ``emitEvent()`` while
accessing the requested ``Action`` through ``action()``.

.. _libnymea-integrations-browsing:

Browsing helpers
----------------

Media capable integrations use a trio of helper classes:

* ``BrowseResult`` describes the initial browse request for a ``Thing``.  Fill
  it with ``BrowserItems`` via ``addItem()`` and call ``setStatus()`` when the
  listing completes.
* ``BrowserItemResult`` returns the details for a single ``BrowserItem``
  including its ``BrowserItemActions``.
* ``BrowserActionInfo`` and ``BrowserItemActionInfo`` wrap the execution of
  generic and item specific actions respectively.  Use ``setFinished()`` or
  ``setError()`` to update the caller.

.. _libnymea-integrations-io:

IO connections
--------------

``IOConnection`` links the state of one thing to another thing's action or
state.  It stores the input/output thing ids, the referenced state/action type
ids and whether the connection is inverted.  ``IOConnections`` is a list
wrapper that ``ThingManager::ioConnections()`` returns.

``ThingUtils`` offers helper functions for common workflows such as finding
child things, resolving params and filtering IO connections.

.. _libnymea-integrations-service-data:

ServiceData and metadata
------------------------

.. _libnymea-integrations-plugin-metadata:

``ServiceData`` describes auxiliary network services exposed by a plugin.  It
provides the service ``type()``, ``name()``, ``host()``, ``port()`` and a map
of ``txtRecords()``.

``PluginMetadata`` mirrors the plugin's ``metadata.json`` file: it stores the
unique ``pluginId()``, ``name()``, ``displayName()``, supported
``Vendor``/``ThingClass`` identifiers, version information, ``deviceClasses``
and translation domains.  The metadata also contains capability flags such as
``autoThings()`` and ``createMethods()``.

.. _libnymea-integrations-state-value-filter:

State value filters
-------------------

``StateValueFilter`` is an abstract base class for smoothing noisy state
updates.  Plugins attach filters to thing states through
``ThingManager::setStateFilter()``.  ``StateValueFilterAdaptive`` implements an
adaptive moving average with a configurable window size and deviation bounds.

