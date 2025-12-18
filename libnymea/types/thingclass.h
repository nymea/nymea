// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICECLASS_H
#define DEVICECLASS_H

#include "libnymea.h"
#include "types/actiontype.h"
#include "types/eventtype.h"
#include "types/paramtype.h"
#include "types/statetype.h"
#include "types/vendor.h"
#include "typeutils.h"

#include <qobjectdefs.h>
#include <QDebug>
#include <QList>
#include <QUuid>

class ThingClass
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QUuid vendorId READ vendorId)
    Q_PROPERTY(QUuid pluginId READ pluginId)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString displayName READ displayName)
    Q_PROPERTY(QStringList interfaces READ interfaces)
    Q_PROPERTY(QStringList providedInterfaces READ providedInterfaces)
    Q_PROPERTY(bool browsable READ browsable)
    Q_PROPERTY(SetupMethod setupMethod READ setupMethod)
    Q_PROPERTY(CreateMethods createMethods READ createMethods)
    Q_PROPERTY(DiscoveryType discoveryType READ discoveryType)
    Q_PROPERTY(StateTypes stateTypes READ stateTypes)
    Q_PROPERTY(EventTypes eventTypes READ eventTypes)
    Q_PROPERTY(ActionTypes actionTypes READ actionTypes)
    Q_PROPERTY(ActionTypes browserItemActionTypes READ browserItemActionTypes)
    Q_PROPERTY(ParamTypes paramTypes READ paramTypes)
    Q_PROPERTY(ParamTypes settingsTypes READ settingsTypes)
    Q_PROPERTY(ParamTypes discoveryParamTypes READ discoveryParamTypes)

public:
    enum CreateMethod { CreateMethodUser = 0x01, CreateMethodAuto = 0x02, CreateMethodDiscovery = 0x04 };
    Q_ENUM(CreateMethod) // Note: required even if Q_DECLARE_FLAGS should do it

    Q_DECLARE_FLAGS(CreateMethods, CreateMethod)
    Q_FLAG(CreateMethods)

    enum SetupMethod { SetupMethodJustAdd, SetupMethodDisplayPin, SetupMethodEnterPin, SetupMethodPushButton, SetupMethodUserAndPassword, SetupMethodOAuth };
    Q_ENUM(SetupMethod)

    enum DiscoveryType { DiscoveryTypePrecise, DiscoveryTypeWeak };
    Q_ENUM(DiscoveryType)

    ThingClass(const PluginId &pluginId = PluginId(), const VendorId &vendorId = VendorId(), const ThingClassId &id = ThingClassId());

    ThingClassId id() const;
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
    bool hasStateType(const StateTypeId &stateTypeId) const;
    bool hasStateType(const QString &stateTypeName) const;

    EventTypes eventTypes() const;
    void setEventTypes(const EventTypes &eventTypes);
    bool hasEventType(const EventTypeId &eventTypeId) const;
    bool hasEventType(const QString &eventTypeName) const;

    ActionTypes actionTypes() const;
    void setActionTypes(const ActionTypes &actionTypes);
    bool hasActionType(const ActionTypeId &actionTypeId) const;
    bool hasActionType(const QString &actionTypeName) const;

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

    DiscoveryType discoveryType() const;
    void setDiscoveryType(DiscoveryType discoveryType);

    QStringList interfaces() const;
    void setInterfaces(const QStringList &interfaces);

    QStringList providedInterfaces() const;
    void setProvidedInterfaces(const QStringList &providedInterfaces);

    bool operator==(const ThingClass &device) const;

private:
    ThingClassId m_id;
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
    CreateMethods m_createMethods = CreateMethodUser;
    SetupMethod m_setupMethod = SetupMethodJustAdd;
    DiscoveryType m_discoveryType = DiscoveryTypePrecise;
    QStringList m_interfaces;
    QStringList m_providedInterfaces;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ThingClass::CreateMethods)

QDebug operator<<(QDebug dbg, const ThingClass &thingClass);

class LIBNYMEA_EXPORT ThingClasses : public QList<ThingClass>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    ThingClasses();
    ThingClasses(const QList<ThingClass> &other);
    ThingClass findById(const ThingClassId &id) const;
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};

#endif
