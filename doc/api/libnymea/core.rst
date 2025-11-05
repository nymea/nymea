Core utilities
==============

.. _libnymea-core-lib-header:

Export macros
-------------

``libnymea.h`` exposes the ``LIBNYMEA_EXPORT`` macro that decorates every
public class in this library.  Consumers should include the header before any
other ``libnymea`` type to get the proper import/export semantics when
building plugins on different platforms.

.. _libnymea-core-nymeasettings:

NymeaSettings
-------------

``NymeaSettings`` wraps ``QSettings`` and scopes configuration files to the
server roles that nymea exposes.

**Construction**
  ``NymeaSettings(SettingsRole role = SettingsRoleNone, QObject *parent = nullptr)``

**Roles**
  ``SettingsRole`` distinguishes dedicated settings trees for things, rules,
  plugins, global preferences, tags, MQTT policies, IO connections, and the
  Zigbee, Modbus RTU, and Z-Wave stacks.

**Helpers**
  The class forwards the standard ``QSettings`` API (``beginGroup()``,
  ``setValue()``, ``allKeys()`` …) and adds static helpers for the commonly
  used paths ``settingsPath()``, ``storagePath()``, ``cachePath()`` and
  ``translationsPath()``.  ``isRoot()`` is a convenience for detecting if the
  process has the permissions to manage system level configuration.

.. _libnymea-core-nymeadbus:

NymeaDBusService
----------------

``NymeaDBusService`` simplifies registering a D-Bus service under the
``io.guh.nymead`` interface.  Instantiate it with the object path you want to
export and call ``isValid()`` to verify that the registration succeeded.

``setBusType()`` allows switching between the system and the session bus
before creating the first instance.  Subclasses can access the underlying
``QDBusConnection`` via the protected ``connection()`` accessor.

.. _libnymea-core-hardware-resource:

HardwareResource
----------------

The base class for any pluggable hardware stack.  Each resource exposes a
``name()``, a tri-state ``available()``/``enabled()`` pair, and emits
``enabledChanged``/``availableChanged`` whenever the driver toggles state.

Hardware resources are managed exclusively through
:ref:`HardwareManager <libnymea-core-hardware-manager>` which has the
necessary access to call ``setEnabled()``.

.. _libnymea-core-hardware-manager:

HardwareManager
---------------

``HardwareManager`` is the abstract factory through which plugins obtain
access to physical transports.  Subclasses provided by ``nymead`` return
instances for all built-in resources:

* Radio 433 MHz (:ref:`Radio433 <libnymea-hardware-radio433>`)
* UPnP discovery and descriptor helpers
* HTTP networking via ``NetworkAccessManager``
* Zeroconf, Bluetooth LE, MQTT, I²C, Zigbee, Z-Wave, Modbus RTU and the
  generic :ref:`NetworkDeviceDiscovery <libnymea-network-devices>` service
* The :ref:`PluginTimerManager <libnymea-core-plugin-timer>`

Plugins never instantiate a ``HardwareManager`` themselves.  Instead the
``ThingManager`` injects one during ``IntegrationPlugin`` initialisation.

.. _libnymea-core-plugin-timer:

Plugin timers
-------------

``PluginTimer`` provides a cooperative timer that is safe to expose to QML.
It offers ``start()``, ``stop()``, ``pause()``, ``resume()``, ``reset()`` and
tracks ``interval()``, ``currentTick()`` and ``running()``.  ``timeout``,
``currentTickChanged`` and ``runningChanged`` signals allow reacting to the
schedule.

``PluginTimerManager`` is a :ref:`HardwareResource
<libnymea-core-hardware-resource>` that hands out timer instances via
``registerTimer(int seconds = 60)`` and tears them down with
``unregisterTimer()``.  Plugins should always request timers through the
manager instead of creating their own ``QTimer`` objects so the runtime can
coordinate resource usage.

