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

#include "devicediscoveryinfo.h"
#include "devicepairinginfo.h"
#include "devicesetupinfo.h"
#include "deviceactioninfo.h"
#include "browseresult.h"
#include "browseritemresult.h"
#include "browseractioninfo.h"
#include "browseritemactioninfo.h"

#include <QObject>
#include <QTranslator>
#include <QPair>
#include <QSettings>

class DeviceManager;

class LIBNYMEA_EXPORT DevicePlugin: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QUuid id READ pluginId)
    Q_PROPERTY(QString name READ pluginName)
    Q_PROPERTY(QString displayName READ pluginDisplayName)
    Q_PROPERTY(ParamTypes paramTypes READ configurationDescription)

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
    virtual void discoverDevices(DeviceDiscoveryInfo *info);

    virtual void setupDevice(DeviceSetupInfo *info);
    virtual void postSetupDevice(Device *device);
    virtual void deviceRemoved(Device *device);

    virtual void startPairing(DevicePairingInfo *info);
    virtual void confirmPairing(DevicePairingInfo *info, const QString &username, const QString &secret);

    virtual void executeAction(DeviceActionInfo *info);

    virtual void browseDevice(BrowseResult *result);
    virtual void browserItem(BrowserItemResult *result);
    virtual void executeBrowserItem(BrowserActionInfo *info);
    virtual void executeBrowserItemAction(BrowserItemActionInfo *info);

    // Configuration
    ParamTypes configurationDescription() const;
    Device::DeviceError setConfiguration(const ParamList &configuration);
    ParamList configuration() const;
    QVariant configValue(const ParamTypeId &paramTypeId) const;
    Device::DeviceError setConfigValue(const ParamTypeId &paramTypeId, const QVariant &value);

    bool isBuiltIn() const;

signals:
    void emitEvent(const Event &event);
    void configValueChanged(const ParamTypeId &paramTypeId, const QVariant &value);
    void autoDevicesAppeared(const DeviceDescriptors &deviceDescriptors);
    void autoDeviceDisappeared(const DeviceId &deviceId);

protected:
    Devices myDevices() const;
    HardwareManager *hardwareManager() const;
    QSettings *pluginStorage() const;

private:
    friend class DeviceManager;
    friend class DeviceManagerImplementation;


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
    QSettings *m_storage = nullptr;

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
