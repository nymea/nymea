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

#ifndef ZWAVENODE_H
#define ZWAVENODE_H

#include <QObject>
#include <QUuid>

#include "zwavevalue.h"

class ZWaveNode : public QObject
{
    Q_OBJECT

public:
    enum ZWaveNodeType {
        ZWaveNodeTypeUnknown = 0x00,
        ZWaveNodeTypeController = 0x01,
        ZWaveNodeTypeStaticController = 0x02,
        ZWaveNodeTypeSlave = 0x03,
        ZWaveNodeTypeRoutingSlave = 0x04,
    };
    Q_ENUM(ZWaveNodeType)

    enum ZWaveNodeRole {
        ZWaveNodeRoleUnknown = -0x01,
        ZWaveNodeRoleCentralController = 0x00,
        ZWaveNodeRoleSubController = 0x01,
        ZWaveNodeRolePortableController = 0x02,
        ZWaveNodeRolePortableReportingController = 0x03,
        ZWaveNodeRolePortableSlave = 0x04,
        ZWaveNodeRoleAlwaysOnSlabe = 0x05,
        ZWaveNodeRoleReportingSleepingSlave = 0x06,
        ZWaveNodeRoleListeningSleepingSlave = 0x07
    };
    Q_ENUM(ZWaveNodeRole)

    enum ZWaveDeviceType {
        ZWaveDeviceTypeUnknown = 0x0000,
        ZWaveDeviceTypeCentralController = 0x0100,
        ZWaveDeviceTypeDisplaySimple = 0x0200,
        ZWaveDeviceTypeDoorLockKeypad = 0x0300,
        ZWaveDeviceTypeFanSwitch = 0x0400,
        ZWaveDeviceTypeGateway = 0x0500,
        ZWaveDeviceTypeLightDimmerSwitch = 0x0600,
        ZWaveDeviceTypeOnOffPowerSwitch = 0x0700,
        ZWaveDeviceTypePowerStrip = 0x0800,
        ZWaveDeviceTypeRemoteControlAV = 0x0900,
        ZWaveDeviceTypeRemoteControlMultiPurpose = 0x0a00,
        ZWaveDeviceTypeRemoteControlSimple = 0x0b00,
        ZWaveDeviceTypeKeyFob = 0x0b01,
        ZWaveDeviceTypeSensorNotification = 0x0c00,
        ZWaveDeviceTypeSmokeAlarmSensor = 0x0c01,
        ZWaveDeviceTypeCOAlarmSensor = 0x0c02,
        ZWaveDeviceTypeCO2AlarmSensor = 0x0c03,
        ZWaveDeviceTypeHeatAlarmSensor = 0x0c04,
        ZWaveDeviceTypeWaterAlarmSensor = 0x0c05,
        ZWaveDeviceTypeAccessControlSensor = 0x0c06,
        ZWaveDeviceTypeHomeSecuritySensor = 0x0c07,
        ZWaveDeviceTypePowerManagementSensor = 0x0c08,
        ZWaveDeviceTypeSystemSensor = 0x0c09,
        ZWaveDeviceTypeEmergencyAlarmSensor = 0x0c0a,
        ZWaveDeviceTypeClockSensor = 0x0c0b,
        ZWaveDeviceTypeMultiDeviceAlarmSensor = 0x0cff,
        ZWaveDeviceTypeMultilevelSensor = 0x0d00,
        ZWaveDeviceTypeAirTemperatureSensor = 0x0d01,
        ZWaveDeviceTypeGeneralPurposeSensor = 0x0d02,
        ZWaveDeviceTypeLuminanceSensor = 0x0d03,
        ZWaveDeviceTypePowerSensor = 0x0d04,
        ZWaveDeviceTypeHumiditySensor = 0x0d05,
        ZWaveDeviceTypeVelocitySensor = 0x0d06,
        ZWaveDeviceTypeDirectionSensor = 0x0d07,
        ZWaveDeviceTypeAtmosphericPressureSensor = 0x0d08,
        ZWaveDeviceTypeBarometricPressureSensor = 0x0d09,
        ZWaveDeviceTypeSolarRadiationSensor = 0x0d0a,
        ZWaveDeviceTypeDewPointSensor = 0x0d0b,
        ZWaveDeviceTypeRainRateSensor = 0x0d0c,
        ZWaveDeviceTypeTideLevelSensor = 0x0d0d,
        ZWaveDeviceTypeWeightSensor = 0x0d0e,
        ZWaveDeviceTypeVoltageSensor = 0x0d0f,
        ZWaveDeviceTypeCurrentSensor = 0x0d10,
        ZWaveDeviceTypeCO2LevelSensor = 0x0d11,
        ZWaveDeviceTypeAirFlowSensor = 0x0d12,
        ZWaveDeviceTypeTankCapacitySensor = 0x0d13,
        ZWaveDeviceTypeDistanceSensor = 0x0d14,
        ZWaveDeviceTypeAnglePositionSensor = 0x0d15,
        ZWaveDeviceTypeRotationSensor = 0x0d16,
        ZWaveDeviceTypeWaterTemperatureSensor = 0x0d17,
        ZWaveDeviceTypeSoilTemperatureSensor = 0x0d18,
        ZWaveDeviceTypeSeismicIntensitySensor = 0x0d19,
        ZWaveDeviceTypeSeismicMagnitudeSensor = 0x0d1a,
        ZWaveDeviceTypeUltraVioletSensor = 0x0d1b,
        ZWaveDeviceTypeElectricalResistivitySensor = 0x0d1c,
        ZWaveDeviceTypeElectricalConductivitySensor = 0x0d1d,
        ZWaveDeviceTypeLoudnessSensor = 0x0d1e,
        ZWaveDeviceTypeMoistureSensor = 0x0d1f,
        ZWaveDeviceTypeFrequencySensor = 0x0d20,
        ZWaveDeviceTypeTimeSensor = 0x0d21,
        ZWaveDeviceTypeTargetTemperatureSensor = 0x0d22,
        ZWaveDeviceTypeMultiDeviceSensor = 0x0dff,
        ZWaveDeviceTypeSetTopBox = 0x0e00,
        ZWaveDeviceTypeSiren = 0x0f00,
        ZWaveDeviceTypeSubEnergyMeter = 0x1000,
        ZWaveDeviceTypeSubSystemController = 0x1100,
        ZWaveDeviceTypeThermostatHVAC = 0x1200,
        ZWaveDeviceTypeThermostatSetback = 0x1300,
        ZWaveDeviceTypeTV = 0x1400,
        ZWaveDeviceTypeValveOpenClose = 0x1500,
        ZWaveDeviceTypeWallController = 0x1600,
        ZWaveDeviceTypeWholeHomeMeterSimple = 0x1700,
        ZWaveDeviceTypeWindowCoveringNoPosEndpoint = 0x1800,
        ZWaveDeviceTypeWindowCoveringEndpointAware = 0x1900,
        ZWaveDeviceTypeWindowCoveringPositionEndpointAware = 0x1a00,
    };
    Q_ENUM(ZWaveDeviceType)

    enum ZWavePlusDeviceType {
        ZWavePlusDeviceTypeUnknown = 0x00
    };
    Q_ENUM(ZWavePlusDeviceType)

    explicit ZWaveNode(QObject *parent = nullptr);
    virtual ~ZWaveNode() = default;

    virtual QUuid networkUuid() const = 0;
    virtual quint8 nodeId() const = 0;

    virtual bool initialized() const = 0;
    virtual bool reachable() const = 0;
    virtual bool failed() const = 0;
    virtual bool sleeping() const = 0;
    virtual quint8 linkQuality() const = 0;
    virtual quint8 securityMode() const = 0;

    virtual ZWaveNodeType nodeType() const = 0;
    virtual ZWaveNodeRole role() const = 0;
    virtual ZWaveDeviceType deviceType() const = 0;
    virtual quint16 manufacturerId() const = 0;
    virtual QString manufacturerName() const = 0;
    virtual QString name() const = 0;
    virtual quint16 productId() const = 0;
    virtual QString productName() const = 0;
    virtual quint16 productType() const = 0;
    virtual quint8 version() const = 0;

    virtual bool isZWavePlusDevice() const = 0;
    virtual bool isSecurityDevice() const = 0;
    virtual bool isBeamingDevice() const = 0;

    virtual ZWavePlusDeviceType plusDeviceType() const = 0;

    virtual QList<ZWaveValue> values() const = 0;

    virtual ZWaveValue value(quint64 valueId) const = 0;
    virtual ZWaveValue value(ZWaveValue::Genre genre, ZWaveValue::CommandClass commandClass, quint8 instance, quint16 index, ZWaveValue::Type type) const = 0;

    virtual void setValue(const ZWaveValue &value) = 0;

signals:
    void initializedChanged(bool initialized);
    void reachableChanged(bool reachable);
    void failedChanged(bool failed);
    void sleepingChanged(bool failed);
    void linkQualityChanged(quint8 linkQuality);
    void securityModeChanged(quint8 securityMode);

    void nodeTypeChanged();
    void roleChanged();
    void deviceTypeChanged();
    void plusDeviceTypeChanged();
    void manufacturerIdChanged();
    void manufacturerNameChanged();
    void nameChanged();
    void productIdChanged();
    void productNameChanged();
    void productTypeChanged();
    void versionChanged();
    void isZWavePlusDeviceChanged();
    void isSecurityDeviceChanged();
    void isBeamingDeviceChanged();

    void valueAdded(const ZWaveValue &value);
    void valueChanged(const ZWaveValue &value);
    void valueRemoved(const ZWaveValue &value);
};

class ZWaveNodes: public QList<ZWaveNode*>
{
public:
    ZWaveNodes() = default;
    ZWaveNodes(const ZWaveNodes &other): QList<ZWaveNode*>(other) {}
    ZWaveNodes(const QList<ZWaveNode*> &other): QList<ZWaveNode*>(other) {}
};

QDebug operator<<(QDebug debug, ZWaveNode *node);

#endif // ZWAVENODE_H
