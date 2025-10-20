Platform controllers
====================

``libnymea/platform`` contains helper classes that integrate nymea with the
underlying operating system and package sources.

Repositories and packages
-------------------------

* ``Repository`` describes a software repository entry with its ``id()``,
  ``displayName()``, ``url()`` and enabled state.  ``Repositories`` is the list
  wrapper that adds convenience iterators.
* ``Package`` represents a single package known to the platform.  It stores the
  package ``id()``, ``displayName()``, ``description()``, ``categories()``,
  ``installedVersion()`` and ``availableVersion()`` as well as booleans for
  ``upgradable()``, ``installed()`` and ``restartRequired()``.  ``Packages`` is
  the matching container.

Controllers
-----------

* ``PlatformSystemController`` exposes high level device management features
  such as ``restart()``, ``shutdown()``, ``factoryReset()``, log retrieval and
  network configuration triggers.
* ``PlatformUpdateController`` coordinates system updates.  It provides access
  to the configured repositories, exposes ``checkForUpdates()`` and
  ``installPackages()`` helpers and emits progress signals while operations run.
* ``PlatformZeroConfController`` is a :ref:`HardwareResource
  <libnymea-core-hardware-resource>` that wraps zeroconf discovery/publishing
  for platform level services.  It exposes ``serviceBrowser()`` and
  ``servicePublisher()`` accessors so plugins can register additional entries.

