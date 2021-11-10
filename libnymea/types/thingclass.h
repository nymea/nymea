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

class LIBNYMEA_EXPORT ThingClass
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
    Q_PROPERTY(StateTypes stateTypes READ stateTypes)
    Q_PROPERTY(EventTypes eventTypes READ eventTypes)
    Q_PROPERTY(ActionTypes actionTypes READ actionTypes)
    Q_PROPERTY(ActionTypes browserItemActionTypes READ browserItemActionTypes)
    Q_PROPERTY(ParamTypes paramTypes READ paramTypes)
    Q_PROPERTY(ParamTypes settingsTypes READ settingsTypes)
    Q_PROPERTY(ParamTypes discoveryParamTypes READ discoveryParamTypes)

public:
    enum CreateMethod {
        CreateMethodUser = 0x01,
        CreateMethodAuto = 0x02,
        CreateMethodDiscovery = 0x04
    };
    Q_ENUM(CreateMethod)
    Q_DECLARE_FLAGS(CreateMethods, CreateMethod)
    Q_FLAG(CreateMethods)

    enum SetupMethod {
        SetupMethodJustAdd,
        SetupMethodDisplayPin,
        SetupMethodEnterPin,
        SetupMethodPushButton,
        SetupMethodUserAndPassword,
        SetupMethodOAuth
    };
    Q_ENUM(SetupMethod)

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
    CreateMethods m_createMethods;
    SetupMethod m_setupMethod;
    QStringList m_interfaces;
    QStringList m_providedInterfaces;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ThingClass::CreateMethods)

QDebug operator<<(QDebug &dbg, const ThingClass &deviceClass);

class LIBNYMEA_EXPORT ThingClasses: public QList<ThingClass>
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
