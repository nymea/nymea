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

#ifndef DEVICE_H
#define DEVICE_H

#include "typeutils.h"
#include "libnymea.h"

#include "types/deviceclass.h"
#include "types/state.h"
#include "types/param.h"
#include "types/browseritem.h"

#include <QObject>
#include <QUuid>
#include <QVariant>

class DevicePlugin;

class LIBNYMEA_EXPORT Device: public QObject
{
    Q_OBJECT

    friend class DeviceManager;
    friend class DeviceManagerImplementation;

public:
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
        DeviceErrorAuthenticationFailure,
        DeviceErrorDeviceInUse,
        DeviceErrorDeviceInRule,
        DeviceErrorDeviceIsChild,
        DeviceErrorPairingTransactionIdNotFound,
        DeviceErrorParameterNotWritable,
        DeviceErrorItemNotFound,
        DeviceErrorItemNotExecutable,
        DeviceErrorUnsupportedFeature,
    };
    Q_ENUM(DeviceError)

    DeviceId id() const;
    DeviceClassId deviceClassId() const;
    PluginId pluginId() const;

    DeviceClass deviceClass() const;
    DevicePlugin* plugin() const;

    QString name() const;
    void setName(const QString &name);

    ParamList params() const;
    bool hasParam(const ParamTypeId &paramTypeId) const;
    void setParams(const ParamList &params);

    QVariant paramValue(const ParamTypeId &paramTypeId) const;
    void setParamValue(const ParamTypeId &paramName, const QVariant &value);

    ParamList settings() const;
    bool hasSetting(const ParamTypeId &paramTypeId) const;
    void setSettings(const ParamList &settings);

    QVariant setting(const ParamTypeId &paramTypeId) const;
    void setSettingValue(const ParamTypeId &paramTypeId, const QVariant &value);

    QList<State> states() const;
    bool hasState(const StateTypeId &stateTypeId) const;
    void setStates(const QList<State> &states);

    QVariant stateValue(const StateTypeId &stateTypeId) const;
    void setStateValue(const StateTypeId &stateTypeId, const QVariant &value);

    State state(const StateTypeId &stateTypeId) const;

    DeviceId parentId() const;
    void setParentId(const DeviceId &parentId);

    bool setupComplete() const;
    bool autoCreated() const;

signals:
    void stateValueChanged(const StateTypeId &stateTypeId, const QVariant &value);
    void settingChanged(const ParamTypeId &paramTypeId, const QVariant &value);
    void nameChanged();

private:
    Device(DevicePlugin *plugin, const DeviceClass &deviceClass, const DeviceId &id, QObject *parent = nullptr);
    Device(DevicePlugin *plugin, const DeviceClass &deviceClass, QObject *parent = nullptr);

    void setupCompleted();
    void setSetupComplete(bool complete);

private:
    DeviceClass m_deviceClass;
    DevicePlugin* m_plugin = nullptr;

    DeviceId m_id;
    DeviceId m_parentId;
    QString m_name;
    ParamList m_params;
    ParamList m_settings;
    QList<State> m_states;
    bool m_setupComplete = false;
    bool m_autoCreated = false;
};

QDebug operator<<(QDebug dbg, Device *device);

class LIBNYMEA_EXPORT Devices: public QList<Device*>
{
public:
    Devices() = default;
    Devices(const QList<Device *> &other);
    Device* findById(const DeviceId &id);
    Device* findByParams(const ParamList &params) const;
    Devices filterByParam(const ParamTypeId &paramTypeId, const QVariant &value = QVariant());
    Devices filterByDeviceClassId(const DeviceClassId &deviceClassId);
    Devices filterByParentDeviceId(const DeviceId &deviceId);
};

Q_DECLARE_METATYPE(Device::DeviceError)

#endif
