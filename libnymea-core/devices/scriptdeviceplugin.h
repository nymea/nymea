/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SCRIPTDEVICEPLUGIN_H
#define SCRIPTDEVICEPLUGIN_H

#include "devices/deviceplugin.h"

#include <QQmlEngine>
#include <QJsonObject>

class ScriptDeviceDiscoveryInfo: public QObject
{
    Q_OBJECT
public:
    ScriptDeviceDiscoveryInfo(DeviceDiscoveryInfo *info): QObject(info), m_info(info) {
        connect(info, &DeviceDiscoveryInfo::aborted, this, &ScriptDeviceDiscoveryInfo::aborted);
        connect(info, &DeviceDiscoveryInfo::finished, this, &ScriptDeviceDiscoveryInfo::finished);
    }
    Q_INVOKABLE void addDeviceDescriptor(const QUuid &deviceClassId, const QString &title, const QString &description = QString(), const QVariantList &params = QVariantList(), const QUuid &parentDeviceId = QUuid()) {
        ParamList paramList;
        for (int i = 0; i < params.count(); i++) {
            paramList << Param(params.at(i).toMap().value("paramTypeId").toUuid(), params.at(i).toMap().value("value"));
        }
        DeviceDescriptor d(deviceClassId, title, description, parentDeviceId);
        d.setParams(paramList);
        m_info->addDeviceDescriptor(d);
    }
    Q_INVOKABLE void finish(Device::DeviceError status = Device::DeviceErrorNoError, const QString &displayMessage = QString()) {
        m_info->finish(status, displayMessage);
    }
signals:
    void aborted();
    void finished();
private:
    DeviceDiscoveryInfo *m_info = nullptr;
};

class ScriptDevice: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
public:
    ScriptDevice(Device *device): QObject(device), m_device(device) {
        connect(device, &Device::nameChanged, this, &ScriptDevice::nameChanged);
    }

    QString name() const { return m_device->name(); }
    void setName(const QString &name) { m_device->setName(name); }

    Q_INVOKABLE QVariant paramValue(const QUuid &paramTypeId) { return m_device->paramValue(paramTypeId); }
    Q_INVOKABLE void setParamValue(const QUuid &paramTypeId, const QVariant &value) { m_device->setParamValue(paramTypeId, value); }

    Q_INVOKABLE QVariant stateValue(const QUuid &stateTypeId) { return m_device->stateValue(stateTypeId); }
    Q_INVOKABLE void setStateValue(const QUuid &stateTypeId, const QVariant &value) { m_device->setStateValue(stateTypeId, value); }

signals:
    void nameChanged();

private:
    Device *m_device = nullptr;
};

class ScriptDeviceSetupInfo: public QObject
{
    Q_OBJECT
    Q_PROPERTY(ScriptDevice* device READ device CONSTANT)
public:
    ScriptDeviceSetupInfo(DeviceSetupInfo *info, ScriptDevice *scriptDevice): QObject(info), m_info(info), m_device(scriptDevice) {
        connect(info, &DeviceSetupInfo::aborted, this, &ScriptDeviceSetupInfo::aborted);
        connect(info, &DeviceSetupInfo::finished, this, &ScriptDeviceSetupInfo::finished);
    }
    Q_INVOKABLE void finish(Device::DeviceError status = Device::DeviceErrorNoError, const QString &displayMessage = QString()) {
        m_info->finish(status, displayMessage);
    }
    ScriptDevice* device() const { return m_device; }
signals:
    void aborted();
    void finished();
private:
    DeviceSetupInfo *m_info = nullptr;
    ScriptDevice *m_device = nullptr;
};

class ScriptDevicePairingInfo: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUuid deviceClassId READ deviceClassId CONSTANT)
    Q_PROPERTY(QUuid deviceId READ deviceId CONSTANT)
    Q_PROPERTY(QString deviceName READ deviceName CONSTANT)
    Q_PROPERTY(QUuid parentDeviceId READ parentDeviceId CONSTANT)
    Q_PROPERTY(QUrl oAuthUrl READ oAuthUrl WRITE setOAuthUrl)
public:
    ScriptDevicePairingInfo(DevicePairingInfo* info): QObject(info), m_info(info) {
        connect(info, &DevicePairingInfo::aborted, this, &ScriptDevicePairingInfo::aborted);
        connect(info, &DevicePairingInfo::finished, this, &ScriptDevicePairingInfo::finished);
    }
    Q_INVOKABLE QVariant paramValue(const QUuid &paramTypeId) { return m_info->params().paramValue(paramTypeId); }
    Q_INVOKABLE void finish(Device::DeviceError status = Device::DeviceErrorNoError, const QString &displayMessage = QString()) {
        m_info->finish(status, displayMessage);
    }
    QUuid deviceClassId() const { return m_info->deviceClassId(); }
    QUuid deviceId() const { return m_info->deviceId(); }
    QString deviceName() const { return m_info->deviceName(); }
    QUuid parentDeviceId() const { return m_info->parentDeviceId(); }
    QUrl oAuthUrl() const { return m_info->oAuthUrl(); }
    void setOAuthUrl(const QUrl &oAuthUrl) { m_info->setOAuthUrl(oAuthUrl); }
signals:
    void aborted();
    void finished();
private:
    DevicePairingInfo *m_info = nullptr;
};

class ScriptDeviceActionInfo: public QObject
{
    Q_OBJECT
    Q_PROPERTY(ScriptDevice* device READ device CONSTANT)
    Q_PROPERTY(QUuid actionTypeId READ actionTypeId CONSTANT)
public:
    ScriptDeviceActionInfo(DeviceActionInfo* info, ScriptDevice* scriptDevice): QObject(info), m_info(info), m_device(scriptDevice) {
        connect(info, &DeviceActionInfo::finished, this, &ScriptDeviceActionInfo::finished);
        connect(info, &DeviceActionInfo::aborted, this, &ScriptDeviceActionInfo::aborted);
    }
    ScriptDevice* device() const { return m_device; }
    QUuid actionTypeId() const { return m_info->action().actionTypeId(); }
    Q_INVOKABLE QVariant paramValue(const QUuid &paramTypeId) { return m_info->action().params().paramValue(paramTypeId); }
    Q_INVOKABLE void finish(Device::DeviceError status = Device::DeviceErrorNoError, const QString &displayMessage = QString()) {
        m_info->finish(status, displayMessage);
    }

signals:
    void aborted();
    void finished();
private:
    DeviceActionInfo* m_info = nullptr;
    ScriptDevice* m_device = nullptr;
};

class ScriptDevicePlugin : public DevicePlugin
{
    Q_OBJECT
public:
    explicit ScriptDevicePlugin(QObject *parent = nullptr);

    bool loadScript(const QString &fileName);
    QJsonObject metaData() const;

    void init() override;
    void startMonitoringAutoDevices() override;
    void discoverDevices(DeviceDiscoveryInfo *info) override;
    void startPairing(DevicePairingInfo *info) override;
    void confirmPairing(DevicePairingInfo *info, const QString &username, const QString &secret) override;
    void setupDevice(DeviceSetupInfo *info) override;
    void postSetupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;
    void executeAction(DeviceActionInfo *info) override;

private:
    QQmlEngine *m_engine = nullptr;
    QJsonObject m_metaData;
    QJSValue m_pluginImport;
    QHash<Device*, ScriptDevice*> m_devices;
};

#endif // SCRIPTDEVICEPLUGIN_H
