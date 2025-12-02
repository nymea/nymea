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

#ifndef ZWAVEVALUE_H
#define ZWAVEVALUE_H

#include <QObject>
#include <QVariant>

class ZWaveValue
{
    Q_GADGET

public:
    enum Genre {
        GenreUnknown = -1,
        GenreBasic = 0,
        GenreUser,
        GenreConfig,
        GenreSystem,
    };
    Q_ENUM(Genre)

    enum CommandClass {
        CommandClassNoOperation = 0x00,
        CommandClassBasic = 0x20,
        CommandClassApplicationStatus = 0x22,
        CommandClassSwitchBinary = 0x25,
        CommandClassSwitchMultilevel = 0x26,
        CommandClassSwitchAll = 0x27,
        CommandClassSceneActuatorConf = 0x2c,
        CommandClassSceneActivation = 0x2b,
        CommandClassSceneControllerConf = 0x2d,
        CommandClassSensorBinary = 0x30,
        CommandClassSensorMultilevel = 0x31,
        CommandClassMeter = 0x32,
        CommandClassSwitchColor = 0x33,
        CommandClassMeterPulse = 0x35,
        CommandClassMeterTableMonitor = 0x3d,
        CommandClassThermostatMode = 0x40,
        CommandClassThermostatOperatingState = 0x42,
        CommandClassThermostatSetPoint = 0x43,
        CommandClassThermostatFanMode = 0x44,
        CommandClassThermostatFanState = 0x45,
        CommandClassClimateControlSchedule = 0x46,
        CommandClassDoorLockLogging = 0x4C,
        CommandClassScheduleEntryLock = 0x4e,
        CommandClassBasicWindowCovering = 0x50,
        CommandClassCRC16 = 0x56,
        CommandClassAssociationGroupInformation = 0x59,
        CommandClassDeviceResetLocally = 0x5a,
        CommandClassCentralScene = 0x5b,
        CommandClassZWavePlusInfo = 0x5e,
        CommandClassMultiChannel = 0x60,
        CommandClassDoorLock = 0x62,
        CommandClassUserCode = 0x63,
        CommandClassBarrierOperator = 0x66,
        CommandClassSupervision = 0x6c,
        CommandClassEntryControl = 0x6f,
        CommandClassConfiguration = 0x70,
        CommandClassAlarm = 0x71,
        CommandClassManufacturerSpecific = 0x72,
        CommandClassPowerLevel = 0x73,
        CommandClassProtection = 0x75,
        CommandClassNodeNaming = 0x77,
        CommandClassSoundSwitch = 0x79,
        CommandClassFirmwareUpdate = 0x7a,
        CommandClassBattery = 0x80,
        CommandClassClock = 0x81,
        CommandClassWakeup = 0x84,
        CommandClassAssociation = 0x85,
        CommandClassVersion = 0x86,
        CommandClassIndicator = 0x87,
        CommandClassProprietary = 0x88,
        CommandClassTime = 0x8a,
        CommandClassTimeParameters = 0x8b,
        CommandClassMultiChannelAssociation = 0x8e,
        CommandClassMultiCmd = 0x8f,
        CommandClassManufacturerProprietary = 0x91,
        CommandClassSimpleAV = 0x94,
        CommandClassSecurity = 0x98,
        CommandClassAlarmSensor = 0x9c,
        CommandClassSensorConfiguration = 0x9e,
        CommandClassSecurityS2 = 0x9f,
    };
    Q_ENUM(CommandClass)

    enum Type {
        TypeUnknown = -1,
        TypeBool = 0,
        TypeByte,
        TypeDecimal,
        TypeInt,
        TypeList,
        TypeSchedule,
        TypeShort,
        TypeString,
        TypeButton,
        TypeRaw,
        TypeBitSet,
    };
    Q_ENUM(Type)

    ZWaveValue();
    ZWaveValue(quint64 id, Genre genre, ZWaveValue::CommandClass commandClass, quint8 instance, quint16 index, Type type, const QString &description);

    quint64 id() const;
    Genre genre() const;
    CommandClass commandClass() const;
    quint8 instance() const;
    quint16 index() const;
    Type type() const;

    QVariant value() const;
    void setValue(const QVariant &value, int listSelection = -1);
    int valueListSelection() const;
    void selectListValue(int selection);

    QString description() const;

    bool isValid() const;

private:
    quint64 m_id = 0;
    Genre m_genre = GenreUnknown;
    CommandClass m_commandClass = CommandClassNoOperation;
    quint8 m_instance = 0;
    quint8 m_index = 0;
    Type m_type = TypeUnknown;
    QVariant m_value;
    int m_listSelection = -1;
    QString m_description;
};

Q_DECLARE_METATYPE(ZWaveValue::Genre)
Q_DECLARE_METATYPE(ZWaveValue::Type)

QDebug operator<<(QDebug debug, ZWaveValue value);

#endif // ZWAVEVALUE_H
