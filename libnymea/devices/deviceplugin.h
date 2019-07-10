/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2018 Simon Stürz <simon.stuerz@guh.io>              *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#ifndef DEVICEPLUGIN_H
#define DEVICEPLUGIN_H

#include "libnymea.h"
#include "typeutils.h"

#include "device.h"
#include "devicedescriptor.h"
#include "pluginmetadata.h"

#include "types/deviceclass.h"
#include "types/event.h"
#include "types/action.h"
#include "types/vendor.h"
#include "types/param.h"
#include "types/interface.h"
#include "types/browseraction.h"
#include "types/browseritemaction.h"

#include "hardwaremanager.h"

#include <QObject>
#include <QTranslator>
#include <QPair>

class Device;
class DeviceManager;

class LIBNYMEA_EXPORT DevicePlugin: public QObject
{
    Q_OBJECT

    friend class DeviceManager;
    friend class DeviceManagerImplementation;

public:
    DevicePlugin(QObject *parent = nullptr);
    virtual ~DevicePlugin();

    virtual void init() {}

    PluginId pluginId() const;
    QString pluginName() const;
    QString pluginDisplayName() const;
    Vendors supportedVendors() const;
    DeviceClasses supportedDevices() const;

    virtual void startMonitoringAutoDevices();
    virtual Device::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params);

    virtual Device::DeviceSetupStatus setupDevice(Device *device);
    virtual void postSetupDevice(Device *device);
    virtual void deviceRemoved(Device *device);

    virtual Device::DeviceError displayPin(const PairingTransactionId &pairingTransactionId, const DeviceDescriptor &deviceDescriptor);
    virtual Device::DeviceSetupStatus confirmPairing(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params, const QString &secret);

    virtual Device::DeviceError executeAction(Device *device, const Action &action);

    virtual Device::BrowseResult browseDevice(Device *device, Device::BrowseResult result, const QString &nodeId = QString());
    virtual Device::DeviceError executeBrowserItem(Device *device, const BrowserAction &browserAction);
    virtual Device::DeviceError executeBrowserItemAction(Device *device, const BrowserItemAction &browserItemAction);

    // Configuration
    ParamTypes configurationDescription() const;
    Device::DeviceError setConfiguration(const ParamList &configuration);
    ParamList configuration() const;
    QVariant configValue(const ParamTypeId &paramTypeId) const;
    Device::DeviceError setConfigValue(const ParamTypeId &paramTypeId, const QVariant &value);

    bool isBuiltIn() const;

signals:
    void emitEvent(const Event &event);
    void devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors);
    void deviceSetupFinished(Device *device, Device::DeviceSetupStatus status);
    void pairingFinished(const PairingTransactionId &pairingTransactionId, Device::DeviceSetupStatus status);
    void actionExecutionFinished(const ActionId &id, Device::DeviceError status);
    void configValueChanged(const ParamTypeId &paramTypeId, const QVariant &value);
    void autoDevicesAppeared(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors);
    void autoDeviceDisappeared(const DeviceId &deviceId);
    void browseRequestFinished(const Device::BrowseResult &result);
    void browserItemExecutionFinished(const ActionId &actionid, Device::DeviceError status);
    void browserItemActionExecutionFinished(const ActionId &actionid, Device::DeviceError status);

protected:
    Devices myDevices() const;
    HardwareManager *hardwareManager() const;

private:
    void setMetaData(const PluginMetadata &metaData);
    void initPlugin(const PluginMetadata &metadata, DeviceManager *deviceManager, HardwareManager *hardwareManager);

    QPair<bool, QList<ParamType> > parseParamTypes(const QJsonArray &array) const;

    // Returns <missingFields, unknownFields>
    QPair<QStringList, QStringList> verifyFields(const QStringList &possibleFields, const QStringList &mandatoryFields, const QJsonObject &value) const;

    // load and verify enum values
    QPair<bool, Types::Unit> loadAndVerifyUnit(const QString &unitString) const;
    QPair<bool, Types::InputType> loadAndVerifyInputType(const QString &inputType) const;

    DeviceManager *m_deviceManager = nullptr;
    HardwareManager *m_hardwareManager = nullptr;

    PluginMetadata m_metaData;
    ParamList m_config;
};

Q_DECLARE_INTERFACE(DevicePlugin, "io.nymea.DevicePlugin")


class LIBNYMEA_EXPORT DevicePlugins: public QList<DevicePlugin*>
{
public:
    DevicePlugins();
    DevicePlugins(const QList<DevicePlugin*> &other);
    DevicePlugin* findById(const PluginId &id) const;
};

#endif // DEVICEPLUGIN_H
