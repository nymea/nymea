/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICECLASS_H
#define DEVICECLASS_H

#include "libguh.h"
#include "typeutils.h"
#include "types/vendor.h"
#include "types/eventtype.h"
#include "types/actiontype.h"
#include "types/statetype.h"
#include "types/paramtype.h"

#include <QList>
#include <QUuid>

class LIBGUH_EXPORT DeviceClass
{
    Q_GADGET
    Q_ENUMS(CreateMethod)
    Q_ENUMS(SetupMethod)
    Q_ENUMS(BasicTag)
    Q_ENUMS(DeviceIcon)

public:
    enum CreateMethod {
        CreateMethodUser = 0x01,
        CreateMethodAuto = 0x02,
        CreateMethodDiscovery = 0x04
    };
    Q_DECLARE_FLAGS(CreateMethods, CreateMethod)

    enum SetupMethod {
        SetupMethodJustAdd,
        SetupMethodDisplayPin,
        SetupMethodEnterPin,
        SetupMethodPushButton
    };

    enum BasicTag {
        BasicTagService,
        BasicTagDevice,
        BasicTagSensor,
        BasicTagActuator,
        BasicTagLighting,
        BasicTagEnergy,
        BasicTagMultimedia,
        BasicTagWeather,
        BasicTagGateway,
        BasicTagHeating,
        BasicTagCooling,
        BasicTagNotification,
        BasicTagSecurity,
        BasicTagTime,
        BasicTagShading,
        BasicTagAppliance,
        BasicTagCamera,
        BasicTagLock
    };

    enum DeviceIcon {
        DeviceIconNone,
        DeviceIconBed,
        DeviceIconBlinds,
        DeviceIconCeilingLamp,
        DeviceIconCouch,
        DeviceIconDeskLamp,
        DeviceIconDesk,
        DeviceIconHifi,
        DeviceIconPower,
        DeviceIconEnergy,
        DeviceIconRadio,
        DeviceIconSmartPhone,
        DeviceIconSocket,
        DeviceIconStandardLamp,
        DeviceIconSun,
        DeviceIconTablet,
        DeviceIconThermometer,
        DeviceIconTune,
        DeviceIconTv,
        DeviceIconBattery,
        DeviceIconDishwasher,
        DeviceIconWashingMachine,
        DeviceIconLaundryDryer,
        DeviceIconIrHeater,
        DeviceIconRadiator,
        DeviceIconSwitch,
        DeviceIconMotionDetectors,
        DeviceIconWeather,
        DeviceIconTime,
        DeviceIconLightBulb,
        DeviceIconGateway,
        DeviceIconMail,
        DeviceIconNetwork,
        DeviceIconCloud
    };

    DeviceClass(const PluginId &pluginId = PluginId(), const VendorId &vendorId = VendorId(), const DeviceClassId &id = DeviceClassId());

    DeviceClassId id() const;
    VendorId vendorId() const;
    PluginId pluginId() const;
    bool isValid() const;

    QString name() const;
    void setName(const QString &name);

    DeviceIcon deviceIcon() const;
    void setDeviceIcon(const DeviceIcon &deviceIcon);

    QList<BasicTag> basicTags() const;
    void setBasicTags(const QList<BasicTag> &basicTags);

    QList<StateType> stateTypes() const;
    void setStateTypes(const QList<StateType> &stateTypes);
    bool hasStateType(const StateTypeId &stateTypeId);

    QList<EventType> eventTypes() const;
    void setEventTypes(const QList<EventType> &eventTypes);
    bool hasEventType(const EventTypeId &eventTypeId);

    QList<ActionType> actionTypes() const;
    void setActionTypes(const QList<ActionType> &actionTypes);
    bool hasActionType(const ActionTypeId &actionTypeId);

    QList<ParamType> paramTypes() const;
    void setParamTypes(const QList<ParamType> &paramTypes);

    QList<ParamType> discoveryParamTypes() const;
    void setDiscoveryParamTypes(const QList<ParamType> &paramTypes);

    CreateMethods createMethods() const;
    void setCreateMethods(CreateMethods createMethods);

    SetupMethod setupMethod() const;
    void setSetupMethod(SetupMethod setupMethod);

    QString pairingInfo() const;
    void setPairingInfo(const QString &pairingInfo);

    bool operator==(const DeviceClass &device) const;

private:
    DeviceClassId m_id;
    VendorId m_vendorId;
    PluginId m_pluginId;
    QString m_name;
    DeviceIcon m_deviceIcon;
    QList<BasicTag> m_basicTags;
    QList<StateType> m_stateTypes;
    QList<EventType> m_eventTypes;
    QList<EventType> m_allEventTypes;
    QList<ActionType> m_actionTypes;
    QList<ParamType> m_paramTypes;
    QList<ParamType> m_discoveryParamTypes;
    CreateMethods m_createMethods;
    SetupMethod m_setupMethod;
    QString m_pairingInfo;
};

#endif
