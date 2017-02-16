/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "libguh.h"

#include "plugin/deviceclass.h"
#include "plugin/device.h"
#include "plugin/devicedescriptor.h"

#include "types/event.h"
#include "types/action.h"
#include "types/vendor.h"

#include "network/networkaccessmanager.h"
#include "network/upnp/upnpdiscovery.h"
#include "network/upnp/upnpdevicedescriptor.h"
#include "network/avahi/qtavahiservicebrowser.h"

#ifdef BLUETOOTH_LE
#include "bluetooth/bluetoothscanner.h"
#endif

#include <QObject>
#include <QTimer>
#include <QLocale>
#include <QPluginLoader>

class Device;
class DevicePlugin;
class DevicePairingInfo;
class Radio433;
class UpnpDiscovery;

class LIBGUH_EXPORT DeviceManager : public QObject
{
    Q_OBJECT
    Q_ENUMS(DeviceError)

    friend class DevicePlugin;

public:
    enum HardwareResource {
        HardwareResourceNone = 0,
        HardwareResourceRadio433 = 1,
        HardwareResourceTimer = 2,
        HardwareResourceNetworkManager = 4,
        HardwareResourceUpnpDisovery = 8,
        HardwareResourceBluetoothLE = 16
    };
    Q_DECLARE_FLAGS(HardwareResources, HardwareResource)

    enum DeviceError {
        DeviceErrorNoError,
        DeviceErrorPluginNotFound,
        DeviceErrorVendorNotFound,
        DeviceErrorDeviceNotFound,
        DeviceErrorDeviceClassNotFound,
        DeviceErrorActionTypeNotFound,
        DeviceErrorStateTypeNotFound,
        DeviceErrorEventTypeNotFound,
        DeviceErrorDeviceDescriptorNotFound,
        DeviceErrorMissingParameter,
        DeviceErrorInvalidParameter,
        DeviceErrorSetupFailed,
        DeviceErrorDuplicateUuid,
        DeviceErrorCreationMethodNotSupported,
        DeviceErrorSetupMethodNotSupported,
        DeviceErrorHardwareNotAvailable,
        DeviceErrorHardwareFailure,
        DeviceErrorAuthentificationFailure,
        DeviceErrorAsync,
        DeviceErrorDeviceInUse,
        DeviceErrorDeviceInRule,
        DeviceErrorDeviceIsChild,
        DeviceErrorPairingTransactionIdNotFound,
        DeviceErrorParameterNotWritable
    };

    enum DeviceSetupStatus {
        DeviceSetupStatusSuccess,
        DeviceSetupStatusFailure,
        DeviceSetupStatusAsync
    };

    explicit DeviceManager(const QLocale &locale, QObject *parent = 0);
    ~DeviceManager();

    static QStringList pluginSearchDirs();
    static QList<QJsonObject> pluginsMetadata();

    void setLocale(const QLocale &locale);

    QList<DevicePlugin*> plugins() const;
    DevicePlugin* plugin(const PluginId &id) const;
    DeviceError setPluginConfig(const PluginId &pluginId, const ParamList &pluginConfig);

    QList<Vendor> supportedVendors() const;
    QList<DeviceClass> supportedDevices(const VendorId &vendorId = VendorId()) const;
    DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params);

    QList<Device*> configuredDevices() const;
    DeviceError addConfiguredDevice(const DeviceClassId &deviceClassId, const QString &name, const ParamList &params, const DeviceId id = DeviceId::createDeviceId());
    DeviceError addConfiguredDevice(const DeviceClassId &deviceClassId, const QString &name, const DeviceDescriptorId &deviceDescriptorId, const DeviceId &id = DeviceId::createDeviceId());

    DeviceError reconfigureDevice(const DeviceId &deviceId, const ParamList &params, bool fromDiscovery = false);
    DeviceError reconfigureDevice(const DeviceId &deviceId, const DeviceDescriptorId &deviceDescriptorId);

    DeviceError editDevice(const DeviceId &deviceId, const QString &name);

    DeviceError pairDevice(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const QString &name, const ParamList &params);
    DeviceError pairDevice(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const QString &name, const DeviceDescriptorId &deviceDescriptorId);
    DeviceError confirmPairing(const PairingTransactionId &pairingTransactionId, const QString &secret = QString());

    DeviceError removeConfiguredDevice(const DeviceId &deviceId);

    Device* findConfiguredDevice(const DeviceId &id) const;
    QList<Device *> findConfiguredDevices(const DeviceClassId &deviceClassId) const;
    QList<Device *> findChildDevices(Device *device) const;
    DeviceClass findDeviceClass(const DeviceClassId &deviceClassId) const;

    DeviceError verifyParams(const QList<ParamType> paramTypes, ParamList &params, bool requireAll = true);
    DeviceError verifyParam(const QList<ParamType> paramTypes, const Param &param);
    DeviceError verifyParam(const ParamType &paramType, const Param &param);

signals:
    void loaded();
    void languageUpdated();
    void eventTriggered(const Event &event);
    void deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value);
    void deviceRemoved(const DeviceId &deviceId);
    void deviceAdded(Device *device);
    void deviceChanged(Device *device);
    void devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &devices);
    void deviceSetupFinished(Device *device, DeviceError status);
    void deviceReconfigurationFinished(Device *device, DeviceError status);
    void pairingFinished(const PairingTransactionId &pairingTransactionId, DeviceError status, const DeviceId &deviceId = DeviceId());
    void actionExecutionFinished(const ActionId &actionId, DeviceManager::DeviceError status);

public slots:
    DeviceError executeAction(const Action &action);
    void timeTick();

private slots:
    void loadPlugins();
    void loadConfiguredDevices();
    void storeConfiguredDevices();
    void startMonitoringAutoDevices();
    void slotDevicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> deviceDescriptors);
    void slotDeviceSetupFinished(Device *device, DeviceManager::DeviceSetupStatus status);
    void slotPairingFinished(const PairingTransactionId &pairingTransactionId, DeviceManager::DeviceSetupStatus status);
    void autoDevicesAppeared(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors);

    // Only connect this to Devices. It will query the sender()
    void slotDeviceStateValueChanged(const QUuid &stateTypeId, const QVariant &value);

    void radio433SignalReceived(QList<int> rawData);
    void replyReady(const PluginId &pluginId, QNetworkReply *reply);

    void upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &deviceDescriptorList, const PluginId &pluginId);
    void upnpNotifyReceived(const QByteArray &notifyData);

    #ifdef BLUETOOTH_LE
    void bluetoothDiscoveryFinished(const PluginId &pluginId, const QList<QBluetoothDeviceInfo> &deviceInfos);
    #endif

    void timerEvent();

private:
    bool verifyPluginMetadata(const QJsonObject &data);
    DeviceError addConfiguredDeviceInternal(const DeviceClassId &deviceClassId, const QString &name, const ParamList &params, const DeviceId id = DeviceId::createDeviceId());
    DeviceSetupStatus setupDevice(Device *device);
    void postSetupDevice(Device *device);

private:
    QLocale m_locale;
    QHash<VendorId, Vendor> m_supportedVendors;
    QHash<VendorId, QList<DeviceClassId> > m_vendorDeviceMap;
    QHash<DeviceClassId, DeviceClass> m_supportedDevices;
    QList<Device *> m_configuredDevices;
    QHash<DeviceDescriptorId, DeviceDescriptor> m_discoveredDevices;

    QHash<PluginId, DevicePlugin*> m_devicePlugins;

    // Hardware Resources
    Radio433* m_radio433;
    QTimer m_pluginTimer;
    QList<DevicePlugin *> m_pluginTimerUsers;
    NetworkAccessManager *m_networkManager;
    UpnpDiscovery* m_upnpDiscovery;
    QtAvahiServiceBrowser *m_avahiBrowser;

    #ifdef BLUETOOTH_LE
    BluetoothScanner *m_bluetoothScanner;
    #endif

    QHash<QUuid, DevicePairingInfo> m_pairingsJustAdd;
    QHash<QUuid, DevicePairingInfo> m_pairingsDiscovery;

    QList<Device *> m_asyncDeviceReconfiguration;
    QList<DevicePlugin *> m_discoveringPlugins;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DeviceManager::HardwareResources)
Q_DECLARE_METATYPE(DeviceManager::DeviceError)

#endif // DEVICEMANAGER_H
