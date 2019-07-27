/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014-2018 Michael Zanetti <michael.zanetti@guh.io>       *
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

#ifndef DEVICECLASS_H
#define DEVICECLASS_H

#include "libnymea.h"
#include "typeutils.h"
#include "types/vendor.h"
#include "types/eventtype.h"
#include "types/actiontype.h"
#include "types/statetype.h"
#include "types/paramtype.h"

#include <QList>
#include <QUuid>

class LIBNYMEA_EXPORT DeviceClass
{
    Q_GADGET

public:
    enum CreateMethod {
        CreateMethodUser = 0x01,
        CreateMethodAuto = 0x02,
        CreateMethodDiscovery = 0x04
    };
    Q_ENUM(CreateMethod)
    Q_DECLARE_FLAGS(CreateMethods, CreateMethod)

    enum SetupMethod {
        SetupMethodJustAdd,
        SetupMethodDisplayPin,
        SetupMethodEnterPin,
        SetupMethodPushButton,
        SetupMethodOAuth,
        SetupMethodUserAndPassword,
    };
    Q_ENUM(SetupMethod)

    DeviceClass(const PluginId &pluginId = PluginId(), const VendorId &vendorId = VendorId(), const DeviceClassId &id = DeviceClassId());

    DeviceClassId id() const;
    VendorId vendorId() const;
    PluginId pluginId() const;
    bool isValid() const;

    QString name() const;
    void setName(const QString &name);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    StateTypes stateTypes() const;
    StateType getStateType(const StateTypeId &stateTypeId);
    void setStateTypes(const StateTypes &stateTypes);
    bool hasStateType(const StateTypeId &stateTypeId);

    EventTypes eventTypes() const;
    void setEventTypes(const EventTypes &eventTypes);
    bool hasEventType(const EventTypeId &eventTypeId);

    ActionTypes actionTypes() const;
    void setActionTypes(const ActionTypes &actionTypes);
    bool hasActionType(const ActionTypeId &actionTypeId);

    bool browsable() const;
    void setBrowsable(bool browsable);

    ActionTypes browserItemActionTypes() const;
    void setBrowserItemActionTypes(const ActionTypes &browserItemActionTypes);
    bool hasBrowserItemActionType(const ActionTypeId &actionTypeId);

    ParamTypes paramTypes() const;
    void setParamTypes(const ParamTypes &paramTypes);

    ParamTypes settingsTypes() const;
    void setSettingsTypes(const ParamTypes &settingsTypes);

    ParamTypes discoveryParamTypes() const;
    void setDiscoveryParamTypes(const ParamTypes &paramTypes);

    CreateMethods createMethods() const;
    void setCreateMethods(CreateMethods createMethods);

    SetupMethod setupMethod() const;
    void setSetupMethod(SetupMethod setupMethod);

    QString pairingInfo() const;
    void setPairingInfo(const QString &pairingInfo);

    QStringList interfaces() const;
    void setInterfaces(const QStringList &interfaces);

    bool operator==(const DeviceClass &device) const;

private:
    DeviceClassId m_id;
    VendorId m_vendorId;
    PluginId m_pluginId;
    QString m_name;
    QString m_displayName;
    bool m_browsable = false;
    StateTypes m_stateTypes;
    EventTypes m_eventTypes;
    ActionTypes m_actionTypes;
    ActionTypes m_browserItemActionTypes;
    ParamTypes m_paramTypes;
    ParamTypes m_settingsTypes;
    ParamTypes m_discoveryParamTypes;
    CreateMethods m_createMethods;
    SetupMethod m_setupMethod;
    QString m_pairingInfo;
    QStringList m_interfaces;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DeviceClass::CreateMethods)

QDebug operator<<(QDebug &dbg, const DeviceClass &deviceClass);

class LIBNYMEA_EXPORT DeviceClasses: public QList<DeviceClass>
{
public:
    DeviceClasses();
    DeviceClasses(const QList<DeviceClass> &other);
    DeviceClass findById(const DeviceClassId &id) const;
};

#endif
