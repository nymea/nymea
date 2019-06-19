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

#include "devicemanager.h"

#include "libnymea.h"
#include "typeutils.h"

#include "types/deviceclass.h"
#include "types/event.h"
#include "types/action.h"
#include "types/vendor.h"
#include "types/param.h"

#include <QObject>
#include <QMetaEnum>
#include <QJsonObject>
#include <QMetaObject>
#include <QTranslator>
#include <QPair>

class Device;
class DeviceManager;

class LIBNYMEA_EXPORT DevicePlugin: public QObject
{
    Q_OBJECT

    friend class DeviceManager;

public:
    DevicePlugin(QObject *parent = nullptr);
    virtual ~DevicePlugin();

    virtual void init() {}

    PluginId pluginId() const;
    QString pluginName() const;
    QString pluginDisplayName() const;
    QList<Vendor> supportedVendors() const;
    QList<DeviceClass> supportedDevices() const;

    virtual void startMonitoringAutoDevices();
    virtual DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params);

    virtual DeviceManager::DeviceSetupStatus setupDevice(Device *device);
    virtual void postSetupDevice(Device *device);
    virtual void deviceRemoved(Device *device);

    virtual DeviceManager::DeviceError displayPin(const PairingTransactionId &pairingTransactionId, const DeviceDescriptor &deviceDescriptor);
    virtual DeviceManager::DeviceSetupStatus confirmPairing(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params, const QString &secret);

    virtual DeviceManager::DeviceError executeAction(Device *device, const Action &action);

    // Configuration
    ParamTypes configurationDescription() const;
    DeviceManager::DeviceError setConfiguration(const ParamList &configuration);
    ParamList configuration() const;
    QVariant configValue(const ParamTypeId &paramTypeId) const;
    DeviceManager::DeviceError setConfigValue(const ParamTypeId &paramTypeId, const QVariant &value);

    bool isBuiltIn() const;

signals:
    void emitEvent(const Event &event);
    void devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors);
    void deviceSetupFinished(Device *device, DeviceManager::DeviceSetupStatus status);
    void pairingFinished(const PairingTransactionId &pairingTransactionId, DeviceManager::DeviceSetupStatus status);
    void actionExecutionFinished(const ActionId &id, DeviceManager::DeviceError status);
    void configValueChanged(const ParamTypeId &paramTypeId, const QVariant &value);
    void autoDevicesAppeared(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors);
    void autoDeviceDisappeared(const DeviceId &deviceId);

protected:
    DeviceManager *deviceManager() const;
    Devices myDevices() const;
    HardwareManager *hardwareManager() const;
    Device* findDeviceByParams(const ParamList &params) const;

private:
    void setMetaData(const QJsonObject &metaData);
    void loadMetaData();
    void initPlugin(DeviceManager *deviceManager);

    QPair<bool, QList<ParamType> > parseParamTypes(const QJsonArray &array) const;

    // Returns <missingFields, unknownFields>
    QPair<QStringList, QStringList> verifyFields(const QStringList &possibleFields, const QStringList &mandatoryFields, const QJsonObject &value) const;

    // load and verify enum values
    QPair<bool, Types::Unit> loadAndVerifyUnit(const QString &unitString) const;
    QPair<bool, Types::InputType> loadAndVerifyInputType(const QString &inputType) const;

    // FIXME: This is expensive because it will open all the files.
    // Once DeviceManager is in libnymea-core this should probably be there too.
    // I didn't want to add even more dependencies on the devicemanager into here, so reading the list here for now.
    static Interfaces allInterfaces();
    static Interface loadInterface(const QString &name);
    static Interface mergeInterfaces(const Interface &iface1, const Interface &iface2);
    static QStringList generateInterfaceParentList(const QString &interface);

    DeviceManager *m_deviceManager = nullptr;

    QList<ParamType> m_configurationDescription;
    ParamList m_config;

    QJsonObject m_metaData;

    mutable QList<DeviceClass> m_supportedDevices;

};

Q_DECLARE_INTERFACE(DevicePlugin, "io.nymea.DevicePlugin")

#endif // DEVICEPLUGIN_H
