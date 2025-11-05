Hardware resources
==================

Bluetooth Low Energy
--------------------

* ``BluetoothLowEnergyManager`` extends :ref:`HardwareResource
  <libnymea-core-hardware-resource>` and exposes device discovery and
  management for BLE adapters.  ``discoverDevices()`` returns a
  ``BluetoothDiscoveryReply`` while ``pairDevice()``, ``unpairDevice()``,
  ``registerDevice()`` and ``unregisterDevice()`` control connections.
* ``BluetoothPairingJob`` encapsulates interactive pairing flows.  It keeps the
  target ``QBluetoothAddress`` and emits ``finished``/``passKeyRequested``/
  ``displayPinCode`` signals.
* ``BluetoothLowEnergyDevice`` represents a connected GATT device and offers
  access to services, characteristics and controller state.

Zigbee
------

* ``ZigbeeHardwareResource`` coordinates Zigbee networks.  Plugins register a
  ``ZigbeeHandler`` for vendor specific behaviour, claim nodes via
  ``claimNode()`` and can remove devices with ``removeNodeFromNetwork()``.  The
  resource exposes coordinator information and emits ``networkStateChanged``.
* ``ZigbeeHandler`` is the pluggable strategy that decides if a discovered
  ``ZigbeeNode`` belongs to the plugin and handles node removal.

Z-Wave
------

* ``ZWaveHardwareResource`` provides access to the Z-Wave stack and publishes
  higher level helpers such as ``ZWaveBackend`` and ``ZWaveHandler``.
* ``ZWave`` exposes controller level helpers (network initialisation,
  inclusion/exclusion entry points) and cooperates with ``ZWaveBackend``.
* ``ZWaveNode`` models an individual node, including endpoints, command
  classes and association information.  ``ZWaveNodes`` is the list wrapper.
* ``ZWaveValue`` stores individual command class values and exposes the
  metadata required to render them.
* ``ZWaveReply`` encapsulates asynchronous operations (inclusion/exclusion,
  interviews, configuration writes).

Modbus RTU
----------

* ``ModbusRtuHardwareResource`` manages the pool of available
  ``ModbusRtuMaster`` instances and emits signals when masters are added,
  removed or changed.
* ``ModbusRtuMaster`` exposes connection parameters (serial port, baudrate,
  parity, retries) and offers read/write helpers for coils, discrete inputs and
  holding/input registers.
* ``ModbusRtuReply`` represents the asynchronous result of a request and
  provides access to the returned ``QVector<quint16>`` payload.

I²C
---

* ``I2CManager`` tracks available ports, performs ``scanRegisters()`` and
  controls read/write operations.  It manages ``I2CDevice`` lifetimes and
  orchestrates polling via ``startReading()``/``stopReading()``.
* ``I2CDevice`` stores the port name and slave address and emits
  ``readingAvailable``/``dataWritten`` signals when asynchronous operations
  complete.

.. _libnymea-hardware-radio433:

Radio 433 MHz
-------------

* ``Radio433`` is the base class for 433 MHz transmitters.  The asynchronous
  ``sendData()`` slot takes raw timings and repetition hints.
* ``Radio433Receiver`` (defined alongside the transmitter implementation)
  provides threaded reception helpers for decoding remote controls.

Electrical helpers
------------------

* ``Electricity`` groups helpers for three-phase awareness.  It exposes the
  ``Phase`` enum, conversion helpers between bit flags and strings and a
  ``getPhaseCount()`` utility.
* ``Pwm`` is a convenience wrapper around Linux sysfs PWM controllers.  It
  handles exporting channels, enabling/disabling them and configuring period,
  duty cycle, frequency and polarity.

Other helpers
-------------

* ``ZWaveBackend`` and ``ZWaveHandler`` expose internal coordination points for
  the Z-Wave integration layer.
* ``ModbusRtuHardwareResource`` interops with ``HardwareManager`` so plugins
  can request Modbus masters without caring about the underlying serial setup.

