/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
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
#include "deviceclass.h"

#include "libguh.h"
#include "typeutils.h"

#include "types/event.h"
#include "types/action.h"
#include "types/vendor.h"
#include "types/param.h"

#ifdef BLUETOOTH_LE
#include <QBluetoothDeviceInfo>
#endif

#include <QObject>
#include <QMetaEnum>
#include <QJsonObject>
#include <QMetaObject>
#include <QTranslator>
#include <QPair>

class DeviceManager;
class Device;

class LIBGUH_EXPORT DevicePlugin: public QObject
{
    Q_OBJECT
public:
    DevicePlugin(QObject *parent = 0);
    virtual ~DevicePlugin();

    virtual void init() {}

    QString pluginName() const;
    PluginId pluginId() const;
    QList<Vendor> supportedVendors() const;
    QList<DeviceClass> supportedDevices() const;

    QTranslator *translator();
    bool setLocale(const QLocale &locale);

    virtual DeviceManager::HardwareResources requiredHardware() const = 0;

    virtual void startMonitoringAutoDevices();
    virtual DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params);

    virtual DeviceManager::DeviceSetupStatus setupDevice(Device *device);
    virtual void postSetupDevice(Device *device);
    virtual void deviceRemoved(Device *device);

    virtual DeviceManager::DeviceError displayPin(const PairingTransactionId &pairingTransactionId, const DeviceDescriptor &deviceDescriptor);
    virtual DeviceManager::DeviceSetupStatus confirmPairing(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params, const QString &secret);

    virtual DeviceManager::DeviceError executeAction(Device *device, const Action &action);

    // Hardware input
    virtual void radioData(const QList<int> &rawData) {Q_UNUSED(rawData)}
    virtual void guhTimer() {}
    virtual void upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList) { Q_UNUSED(upnpDeviceDescriptorList) }
    virtual void upnpNotifyReceived(const QByteArray &notifyData) {Q_UNUSED(notifyData)}

    virtual void networkManagerReplyReady(QNetworkReply *reply) {Q_UNUSED(reply)}

    #ifdef BLUETOOTH_LE
    virtual void bluetoothDiscoveryFinished(const QList<QBluetoothDeviceInfo> &deviceInfos) { Q_UNUSED(deviceInfos) }
    #endif


    // Configuration
    QList<ParamType> configurationDescription() const;
    DeviceManager::DeviceError setConfiguration(const ParamList &configuration);
    ParamList configuration() const;
    QVariant configValue(const ParamTypeId &paramTypeId) const;
    DeviceManager::DeviceError setConfigValue(const ParamTypeId &paramTypeId, const QVariant &value);

signals:
    void emitEvent(const Event &event);
    void devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors);
    void deviceSetupFinished(Device *device, DeviceManager::DeviceSetupStatus status);
    void pairingFinished(const PairingTransactionId &pairingTransactionId, DeviceManager::DeviceSetupStatus status);
    void actionExecutionFinished(const ActionId &id, DeviceManager::DeviceError status);
    void configValueChanged(const ParamTypeId &paramTypeId, const QVariant &value);
    void autoDevicesAppeared(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors);

protected:
    DeviceManager *deviceManager() const;
    QList<Device*> myDevices() const;
    Device* findDeviceByParams(const ParamList &params) const;

    // Radio 433
    bool transmitData(int delay, QList<int> rawData, int repetitions = 10);

    // UPnP dicovery
    void upnpDiscover(QString searchTarget = "ssdp:all", QString userAgent = QString());

    // Avahi browse services
    QtAvahiServiceBrowser *avahiServiceBrowser() const;

    // Bluetooth LE discovery
    #ifdef BLUETOOTH_LE
    bool discoverBluetooth();
    #endif

    // Network manager
    QNetworkReply *networkManagerGet(const QNetworkRequest &request);
    QNetworkReply *networkManagerPost(const QNetworkRequest &request, const QByteArray &data);
    QNetworkReply *networkManagerPut(const QNetworkRequest &request, const QByteArray &data);

private:
    void setMetaData(const QJsonObject &metaData);
    void initPlugin(DeviceManager *deviceManager);

    QPair<bool, QList<ParamType> > parseParamTypes(const QJsonArray &array) const;

    QStringList verifyFields(const QStringList &fields, const QJsonObject &value) const;

    QString translateValue(const QString &context, const QString &string) const;

    // load and verify enum values
    QPair<bool, Types::Unit> loadAndVerifyUnit(const QString &unitString) const;
    QPair<bool, Types::InputType> loadAndVerifyInputType(const QString &inputType) const;
    QPair<bool, DeviceClass::BasicTag> loadAndVerifyBasicTag(const QString &basicTag) const;
    QPair<bool, DeviceClass::DeviceIcon> loadAndVerifyDeviceIcon(const QString &deviceIcon) const;

    static QVariantMap loadInterface(const QString &name);

    QTranslator *m_translator;
    DeviceManager *m_deviceManager;

    QList<ParamType> m_configurationDescription;
    ParamList m_config;

    QJsonObject m_metaData;

    friend class DeviceManager;
};

Q_DECLARE_INTERFACE(DevicePlugin, "guru.guh.DevicePlugin")

#endif // DEVICEPLUGIN_H
