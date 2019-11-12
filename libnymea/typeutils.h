/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef TYPEUTILS_H
#define TYPEUTILS_H

#include <QMetaType>
#include <QUuid>

#include "libnymea.h"

class GadgetUuid: public QUuid
{
    Q_GADGET
public:
    GadgetUuid() {}
    GadgetUuid(const QUuid &uuid): QUuid(uuid) {}
};

#define DECLARE_TYPE_ID(type) class type##Id: public GadgetUuid \
{ \
public: \
    type##Id(const QUuid &uuid): GadgetUuid(uuid) {} \
    type##Id(): GadgetUuid() {} \
    static type##Id create##type##Id() { return type##Id(QUuid::createUuid()); } \
    bool operator==(const type##Id &other) const { \
        return toString() == other.toString(); \
    } \
}; \
Q_DECLARE_METATYPE(type##Id);

DECLARE_TYPE_ID(Vendor)
DECLARE_TYPE_ID(DeviceClass)
DECLARE_TYPE_ID(Device)
DECLARE_TYPE_ID(DeviceDescriptor)

DECLARE_TYPE_ID(ParamType)
DECLARE_TYPE_ID(Param)
DECLARE_TYPE_ID(EventType)
DECLARE_TYPE_ID(Event)
DECLARE_TYPE_ID(StateType)
DECLARE_TYPE_ID(State)
DECLARE_TYPE_ID(ActionType)
DECLARE_TYPE_ID(Action)
DECLARE_TYPE_ID(Plugin)
DECLARE_TYPE_ID(Rule)
DECLARE_TYPE_ID(Browser)

DECLARE_TYPE_ID(PairingTransaction)

class LIBNYMEA_EXPORT Types
{
    Q_GADGET

public:
    enum InputType {
        InputTypeNone,
        InputTypeTextLine,
        InputTypeTextArea,
        InputTypePassword,
        InputTypeSearch,
        InputTypeMail,
        InputTypeIPv4Address,
        InputTypeIPv6Address,
        InputTypeUrl,
        InputTypeMacAddress
    };
    Q_ENUM(InputType)

    enum Unit {
        UnitNone,
        UnitSeconds,
        UnitMinutes,
        UnitHours,
        UnitUnixTime,
        UnitMeterPerSecond,
        UnitKiloMeterPerHour,
        UnitDegree,
        UnitRadiant,
        UnitDegreeCelsius,
        UnitDegreeKelvin,
        UnitMired,
        UnitMilliBar,
        UnitBar,
        UnitPascal,
        UnitHectoPascal,
        UnitAtmosphere,
        UnitLumen,
        UnitLux,
        UnitCandela,
        UnitMilliMeter,
        UnitCentiMeter,
        UnitMeter,
        UnitKiloMeter,
        UnitGram,
        UnitKiloGram,
        UnitDezibel,
        UnitBpm,
        UnitKiloByte,
        UnitMegaByte,
        UnitGigaByte,
        UnitTeraByte,
        UnitMilliWatt,
        UnitWatt,
        UnitKiloWatt,
        UnitKiloWattHour,
        UnitEuroPerMegaWattHour,
        UnitEuroCentPerKiloWattHour,
        UnitPercentage,
        UnitPartsPerMillion,
        UnitEuro,
        UnitDollar,
        UnitHertz,
        UnitAmpere,
        UnitMilliAmpere,
        UnitVolt,
        UnitMilliVolt,
        UnitVoltAmpere,
        UnitVoltAmpereReactive,
        UnitAmpereHour,
        UnitMicroSiemensPerCentimeter,
        UnitDuration
    };
    Q_ENUM(Unit)

    enum ValueOperator {
        ValueOperatorEquals,
        ValueOperatorNotEquals,
        ValueOperatorLess,
        ValueOperatorGreater,
        ValueOperatorLessOrEqual,
        ValueOperatorGreaterOrEqual
    };
    Q_ENUM(ValueOperator)

    enum StateOperator {
        StateOperatorAnd,
        StateOperatorOr
    };
    Q_ENUM(StateOperator)

    enum BrowserType {
        BrowserTypeGeneric,
    };
    Q_ENUM(BrowserType)
};

Q_DECLARE_METATYPE(Types::InputType)
Q_DECLARE_METATYPE(Types::Unit)
Q_DECLARE_METATYPE(Types::ValueOperator)
Q_DECLARE_METATYPE(Types::StateOperator)

#endif // TYPEUTILS_H
