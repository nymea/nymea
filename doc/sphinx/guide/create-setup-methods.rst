Create and setup methods
========================

Device classes control how nymea discovers and configures devices. Each class
specifies one or more creation methods and exactly one setup method. Creation
methods describe how devices enter the system (user initiated, via discovery or
automatically), whereas setup methods describe how the pairing and configuration
workflow proceeds.

Creation methods
----------------

``DeviceClass::CreateMethod`` values map to the following scenarios:

* ``User`` – devices are added manually and the user provides all required
  parameters.
* ``Discovery`` – the plugin discovers devices on demand and the user selects
  which ones to add.
* ``Auto`` – discovery runs in the background and devices appear automatically
  without interaction.

Setup methods
-------------

Only a single ``DeviceClass::SetupMethod`` can be assigned to a device class. The
most common choices are:

* ``JustAdd`` – no additional interaction is required during setup.
* ``DisplayPin`` – the device shows a PIN that must be entered in nymea.
* ``EnterPin`` – nymea generates a PIN that has to be entered on the device.
* ``PushButton`` – the user confirms pairing by pressing a hardware button.

Typical workflows
-----------------

The following scenarios summarise how plugins should react when the device
manager performs setup steps. Consult the source tree for concrete examples and
state machines.

User + JustAdd (synchronous)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The user enters all required parameters and triggers the setup.
2. ``DevicePlugin::setupDevice()`` performs the configuration and returns
   ``DeviceSetupStatusSuccess`` or ``DeviceSetupStatusFailure``.
3. When successful, ``DevicePlugin::postSetupDevice()`` is invoked to finish the
   initialisation.

User + JustAdd (asynchronous)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. ``DevicePlugin::setupDevice()`` starts the asynchronous operation and returns
   ``DeviceSetupStatusAsync``.
2. Once the result is known emit ``DevicePlugin::deviceSetupFinished()``.
3. ``DevicePlugin::postSetupDevice()`` finalises the device when setup succeeds.

Auto + JustAdd
^^^^^^^^^^^^^^

1. ``DevicePlugin::startMonitoringAutoDevices()`` is called after all plugins
   have been loaded.
2. Emit ``DevicePlugin::autoDevicesAppeared()`` whenever new devices are found.
3. Nymea calls ``setupDevice()`` with the discovered descriptor and the plugin
   completes setup as in the synchronous case.

Discovery + JustAdd
^^^^^^^^^^^^^^^^^^^

1. ``DevicePlugin::discoverDevices()`` is called when the user starts a discovery
   session.
2. Return ``DeviceErrorAsync`` and emit ``DevicePlugin::devicesDiscovered`` once
   discovery finished.
3. ``setupDevice()`` receives the descriptor and completes configuration.

Discovery + PushButton
^^^^^^^^^^^^^^^^^^^^^^

1. Run the same discovery flow as above.
2. ``DevicePlugin::confirmPairing()`` is called so the plugin can verify that the
   user pressed the button on the device.
3. Report the result via ``DeviceSetupStatus`` or asynchronously by emitting
   ``DevicePlugin::pairingFinished()``.
4. ``setupDevice()`` finalises the configuration and ``postSetupDevice()`` is
   invoked after success.

Discovery + EnterPin / DisplayPin
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

These methods extend the push-button flow with a PIN exchange:

* ``EnterPin`` – nymea collects the PIN from the user, ``confirmPairing()``
  validates it and ``pairingFinished()`` reports the result.
* ``DisplayPin`` – ``DevicePlugin::displayPin()`` instructs the device to display
  the PIN that was passed to ``confirmPairing()``. Once verified, emit
  ``pairingFinished()`` and continue with ``setupDevice()``.
