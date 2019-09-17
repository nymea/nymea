/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>

#include "device.h"
#include "deviceplugin.h"
#include "types/interface.h"
#include "types/vendor.h"
#include "types/browseritem.h"
#include "types/browseraction.h"
#include "types/browseritemaction.h"

// Signals

/*! \fn void DeviceManager::loaded();
    The DeviceManager will emit this signal when all \l{Device}{Devices} are loaded.
*/

/*! \fn void DeviceManager::pluginConfigChanged(const PluginId &id, const ParamList &config);
    The DeviceManager will emit this signal when the \a config \l{ParamList}{Params} of the \l{DevicePlugin}{plugin} with the given \a id has changed.
*/

/*! \fn void DeviceManager::deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value);
    This signal is emitted when the \l{State} of a \a device changed. The \a stateTypeId parameter describes the
    \l{StateType} and the \a value parameter holds the new value.
*/

/*! \fn void DeviceManager::deviceDisappeared(const DeviceId &deviceId);
    This signal is emitted when the automatically created \l{Device} with the given \a deviceId dissapeard. This signal will
    create the Devices.DeviceRemoved notification.
*/

/*! \fn void DeviceManager::deviceRemoved(const DeviceId &deviceId);
    This signal is emitted when the \l{Device} with the given \a deviceId was removed from the system. This signal will
    create the Devices.DeviceRemoved notification.
*/

/*! \fn void DeviceManager::deviceAdded(Device *device);
    This signal is emitted when a \a \device  was added to the system. This signal will
    create the Devices.DeviceAdded notification.
*/

/*! \fn void DeviceManager::deviceChanged(Device *device);
    This signal is emitted when a \a \device  was changed in the system (by edit or rediscover). This signal will
    create the Devices.DeviceParamsChanged notification.
*/

/*! \fn void DeviceManager::eventTriggered(const Event &event)
    The DeviceManager will emit a \l{Event} described in \a event whenever a Device
    creates one. Normally only \l{nymeaserver::NymeaCore} should connect to this and execute actions
    after checking back with the \{nymeaserver::RulesEngine}. Exceptions might be monitoring interfaces
    or similar, but you should never directly react to this in a \l{DevicePlugin}.
*/


class DeviceManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceManager(QObject *parent = nullptr);
    virtual ~DeviceManager() = default;

    virtual DevicePlugins plugins() const = 0;
    virtual DevicePlugin* plugin(const PluginId &pluginId) const = 0;
    virtual Device::DeviceError setPluginConfig(const PluginId &pluginId, const ParamList &pluginConfig) = 0;

    virtual Vendors supportedVendors() const = 0;
    virtual Interfaces supportedInterfaces() const = 0;
    virtual DeviceClasses supportedDevices(const VendorId &vendorId = VendorId()) const = 0;

    virtual DeviceClass findDeviceClass(const DeviceClassId &deviceClassId) const = 0;

    virtual Devices configuredDevices() const = 0;
    virtual Device* findConfiguredDevice(const DeviceId &id) const = 0;
    virtual Devices findConfiguredDevices(const DeviceClassId &deviceClassId) const = 0;
    virtual Devices findConfiguredDevices(const QString &interface) const = 0;
    virtual Devices findChildDevices(const DeviceId &id) const = 0;

    virtual DeviceDiscoveryInfo* discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) = 0;

    virtual DeviceSetupInfo* addConfiguredDevice(const DeviceClassId &deviceClassId, const QString &name, const ParamList &params, const DeviceId id = DeviceId::createDeviceId()) = 0;
    virtual DeviceSetupInfo* addConfiguredDevice(const DeviceClassId &deviceClassId, const QString &name, const DeviceDescriptorId &deviceDescriptorId, const ParamList &params = ParamList(), const DeviceId &deviceId = DeviceId::createDeviceId()) = 0;

    virtual DeviceSetupInfo* reconfigureDevice(const DeviceId &deviceId, const ParamList &params, bool fromDiscoveryOrAuto = false) = 0;
    virtual DeviceSetupInfo* reconfigureDevice(const DeviceId &deviceId, const DeviceDescriptorId &deviceDescriptorId) = 0;

    virtual Device::DeviceError editDevice(const DeviceId &deviceId, const QString &name) = 0;
    virtual Device::DeviceError setDeviceSettings(const DeviceId &deviceId, const ParamList &settings) = 0;

    virtual DevicePairingInfo* pairDevice(const DeviceClassId &deviceClassId, const QString &name, const ParamList &params) = 0;
    virtual DevicePairingInfo* pairDevice(const DeviceClassId &deviceClassId, const QString &name, const DeviceDescriptorId &deviceDescriptorId) = 0;
    virtual DevicePairingInfo* confirmPairing(const PairingTransactionId &pairingTransactionId, const QString &username = QString(), const QString &secret = QString()) = 0;

    virtual Device::DeviceError removeConfiguredDevice(const DeviceId &deviceId) = 0;

    virtual DeviceActionInfo* executeAction(const Action &action) = 0;

    virtual BrowseResult* browseDevice(const DeviceId &deviceId, const QString &itemId, const QLocale &locale) = 0;
    virtual BrowserItemResult* browserItemDetails(const DeviceId &deviceId, const QString &itemId, const QLocale &locale) = 0;
    virtual BrowserActionInfo* executeBrowserItem(const BrowserAction &browserAction) = 0;
    virtual BrowserItemActionInfo* executeBrowserItemAction(const BrowserItemAction &browserItemAction) = 0;

    virtual QString translate(const PluginId &pluginId, const QString &string, const QLocale &locale) = 0;

signals:
    void pluginConfigChanged(const PluginId &id, const ParamList &config);
    void eventTriggered(const Event &event);
    void deviceStateChanged(Device *device, const StateTypeId &stateTypeId, const QVariant &value);
    void deviceRemoved(const DeviceId &deviceId);
    void deviceDisappeared(const DeviceId &deviceId);
    void deviceAdded(Device *device);
    void deviceChanged(Device *device);
    void deviceSettingChanged(const DeviceId deviceId, const ParamTypeId &settingParamTypeId, const QVariant &value);
};

#endif // DEVICEMANAGER_H
