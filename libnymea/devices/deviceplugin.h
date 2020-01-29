/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
#include <QMetaType>

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
Q_DECLARE_METATYPE(DevicePlugin*)


class LIBNYMEA_EXPORT DevicePlugins: public QList<DevicePlugin*>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    DevicePlugins();
    DevicePlugins(const QList<DevicePlugin*> &other);
    DevicePlugin* findById(const PluginId &id) const;
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
Q_DECLARE_METATYPE(DevicePlugins)

#endif // DEVICEPLUGIN_H
