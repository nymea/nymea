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
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QUuid deviceClassId READ deviceClassId)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged USER true)
    Q_PROPERTY(ParamList params READ params)
    Q_PROPERTY(ParamList settings READ settings WRITE setSettings USER true)
    Q_PROPERTY(States states READ states)
    Q_PROPERTY(bool setupComplete READ setupComplete NOTIFY setupStatusChanged REVISION 1)
    Q_PROPERTY(DeviceSetupStatus setupStatus READ setupStatus NOTIFY setupStatusChanged)
    Q_PROPERTY(QString setupDisplayMessage READ setupDisplayMessage NOTIFY setupStatusChanged USER true)
    Q_PROPERTY(DeviceError setupError READ setupError NOTIFY setupStatusChanged)
    Q_PROPERTY(QUuid parentId READ parentId USER true)

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
        DeviceErrorTimeout,
    };
    Q_ENUM(DeviceError)

    enum DeviceSetupStatus {
        DeviceSetupStatusNone,
        DeviceSetupStatusInProgress,
        DeviceSetupStatusComplete,
        DeviceSetupStatusFailed,
    };
    Q_ENUM(DeviceSetupStatus)

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

    States states() const;
    bool hasState(const StateTypeId &stateTypeId) const;
    void setStates(const States &states);

    QVariant stateValue(const StateTypeId &stateTypeId) const;
    void setStateValue(const StateTypeId &stateTypeId, const QVariant &value);

    State state(const StateTypeId &stateTypeId) const;

    DeviceId parentId() const;
    void setParentId(const DeviceId &parentId);

    // Deprecated
    bool setupComplete() const;
    bool autoCreated() const;

    DeviceSetupStatus setupStatus() const;
    DeviceError setupError() const;
    QString setupDisplayMessage() const;

signals:
    void stateValueChanged(const StateTypeId &stateTypeId, const QVariant &value);
    void settingChanged(const ParamTypeId &paramTypeId, const QVariant &value);
    void nameChanged();
    void setupStatusChanged();

private:
    friend class DeviceManager;
    friend class DeviceManagerImplementation;
    Device(DevicePlugin *plugin, const DeviceClass &deviceClass, const DeviceId &id, QObject *parent = nullptr);
    Device(DevicePlugin *plugin, const DeviceClass &deviceClass, QObject *parent = nullptr);

    void setSetupStatus(Device::DeviceSetupStatus status, Device::DeviceError setupError, const QString &displayMessage = QString());

private:
    DeviceClass m_deviceClass;
    DevicePlugin* m_plugin = nullptr;

    DeviceId m_id;
    DeviceId m_parentId;
    QString m_name;
    ParamList m_params;
    ParamList m_settings;
    States m_states;
    bool m_autoCreated = false;

    DeviceSetupStatus m_setupStatus = DeviceSetupStatusNone;
    DeviceError m_setupError = DeviceErrorNoError;
    QString m_setupDisplayMessage;
};

QDebug operator<<(QDebug dbg, Device *device);

class LIBNYMEA_EXPORT Devices: public QList<Device*>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    Devices() = default;
    Devices(const QList<Device *> &other);
    Device* findById(const DeviceId &id);
    Device* findByParams(const ParamList &params) const;
    Devices filterByParam(const ParamTypeId &paramTypeId, const QVariant &value = QVariant());
    Devices filterByDeviceClassId(const DeviceClassId &deviceClassId);
    Devices filterByParentDeviceId(const DeviceId &deviceId);
    Devices filterByInterface(const QString &interface);
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};

Q_DECLARE_METATYPE(Device::DeviceError)

#endif
