Network services
================

HTTP access
-----------

``NetworkAccessManager`` is a :ref:`HardwareResource
<libnymea-core-hardware-resource>` that wraps ``QNetworkAccessManager``.  It
exposes pure virtual methods for all HTTP verbs (``get()``, ``post()``,
``put()``, ``deleteResource()``, ``head()`` and ``sendCustomRequest()``) so the
runtime can provide a secured implementation that automatically injects client
certificates and proxy settings.

.. _libnymea-network-devices:

Device discovery
----------------

``NetworkDeviceDiscovery`` orchestrates whole-network scans.  It returns a
``NetworkDeviceDiscoveryReply`` which emits progress and completion signals and
ultimately yields a ``NetworkDeviceInfos`` cache.  The discovery object can
also register per-thing monitors using ``registerMonitor()``; these return a
``NetworkDeviceMonitor`` that tracks reachability for a specific ``Thing``.

Supporting types include:

* ``NetworkDeviceInfo`` — a snapshot of a device (MAC address, hostname,
  vendor, ping information, reachability).  ``NetworkDeviceInfos`` is the list
  wrapper exposed as a ``QVector``.
* ``MacAddress`` and ``MacAddressInfo``/``MacAddressInfos`` — helpers for MAC
  parsing and vendor lookup.  ``MacAddressDatabaseReply`` represents the
  asynchronous vendor lookup job.
* ``Ping``/``PingReply`` — expose ICMP reachability checks with retry
  configuration.
* ``ArpSocket`` — allows sending ARP requests and receiving replies on the
  local network.
* ``NetworkDeviceDiscoveryReply`` — encapsulates discovery progress, timeout
  handling and translated status messages for user interfaces.

Utilities
---------

* ``NetworkUtils`` bundles helpers such as subnet calculations, broadcast
  address computation and IPv4/IPv6 normalisation.
* ``OAuth2`` implements the stateful OAuth 2.0 device and authorisation code
  flows.  It emits ``authorizationCodeReceived`` and ``tokenReceived`` and
  stores ``clientId()``, ``clientSecret()`` and ``redirectUri()`` information.
* ``ApiKey`` is a lightweight value object holding the provider id and secret.
  ``ApiKeysProvider`` exposes persistent storage backends and ``ApiKeyStorage``
  gives plugins read/write access to the configured keys.

Messaging and discovery
-----------------------

* ``MqttProvider`` is a :ref:`HardwareResource <libnymea-core-hardware-resource>`
  that provisions ``MqttChannel`` instances for publishing or subscribing to
  MQTT topics.
* ``ZeroConfServiceEntry`` models a service advertisement while
  ``ZeroConfServiceBrowser`` and ``ZeroConfServicePublisher`` locate and expose
  zeroconf services on the network.
* ``UpnpDiscovery`` discovers ``UpnpDevice`` objects and emits
  ``UpnpDiscoveryReply`` instances for asynchronous enumeration.  ``UpnpDevice``
  captures the SSDP details and ``UpnpDeviceDescriptor`` parses the XML device
  description document.

