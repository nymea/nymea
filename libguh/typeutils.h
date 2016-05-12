/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#ifndef TYPEUTILS_H
#define TYPEUTILS_H

#include <QMetaType>
#include <QUuid>

#include "libguh.h"

#define DECLARE_TYPE_ID(type) class type##Id: public QUuid \
{ \
public: \
    type##Id(const QString &uuid): QUuid(uuid) {} \
    type##Id(): QUuid() {} \
    static type##Id create##type##Id() { return type##Id(QUuid::createUuid().toString()); } \
    static type##Id fromUuid(const QUuid &uuid) { return type##Id(uuid.toString()); } \
    bool operator==(const type##Id &other) const { \
        return toString() == other.toString(); \
    } \
}; \
Q_DECLARE_METATYPE(type##Id);

DECLARE_TYPE_ID(Vendor)
DECLARE_TYPE_ID(DeviceClass)
DECLARE_TYPE_ID(Device)
DECLARE_TYPE_ID(DeviceDescriptor)

DECLARE_TYPE_ID(EventType)
DECLARE_TYPE_ID(Event)
DECLARE_TYPE_ID(StateType)
DECLARE_TYPE_ID(State)
DECLARE_TYPE_ID(ActionType)
DECLARE_TYPE_ID(Action)
DECLARE_TYPE_ID(Plugin)
DECLARE_TYPE_ID(Rule)

DECLARE_TYPE_ID(PairingTransaction)

class LIBGUH_EXPORT Types
{
    Q_GADGET
    Q_ENUMS(InputType)
    Q_ENUMS(Unit)
    Q_ENUMS(StateOperator)
    Q_ENUMS(ValueOperator)

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
        UnitHerz,
        UnitAmpere,
        UnitMilliAmpere,
        UnitVolt,
        UnitMilliVolt,
        UnitVoltAmpere,
        UnitVoltAmpereReactive,
        UnitAmpereHour
    };

    enum ValueOperator {
        ValueOperatorEquals,
        ValueOperatorNotEquals,
        ValueOperatorLess,
        ValueOperatorGreater,
        ValueOperatorLessOrEqual,
        ValueOperatorGreaterOrEqual
    };
    enum StateOperator {
        StateOperatorAnd,
        StateOperatorOr
    };

    Types(QObject *parent = 0);
};

Q_DECLARE_METATYPE(Types::InputType)
Q_DECLARE_METATYPE(Types::Unit)
Q_DECLARE_METATYPE(Types::ValueOperator)
Q_DECLARE_METATYPE(Types::StateOperator)

#endif // TYPEUTILS_H
