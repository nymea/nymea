/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2021, nymea GmbH
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

#ifndef TYPEUTILS_H
#define TYPEUTILS_H

#include <QMetaType>
#include <QUuid>

#include "libnymea.h"


#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)

#define DECLARE_TYPE_ID(type) class type##Id: public QUuid \
{ \
public: \
    type##Id(const QUuid &uuid): QUuid(uuid) {} \
    type##Id(): QUuid() {} \
    type##Id(const QString &uuidString): QUuid(QAnyStringView(uuidString)) {} \
    static type##Id create##type##Id() { return type##Id(QUuid::createUuid()); } \
    bool operator==(const type##Id &other) const { \
        return toString() == other.toString(); \
    } \
}; \
Q_DECLARE_METATYPE(type##Id);
#else
#define DECLARE_TYPE_ID(type) class type##Id: public QUuid \
{ \
    public: \
    type##Id(const QUuid &uuid): QUuid(uuid) {} \
    type##Id(): QUuid() {} \
    static type##Id create##type##Id() { return type##Id(QUuid::createUuid()); } \
    bool operator==(const type##Id &other) const { \
        return toString() == other.toString(); \
    } \
}; \
    Q_DECLARE_METATYPE(type##Id);
#endif

DECLARE_TYPE_ID(Vendor)
DECLARE_TYPE_ID(ThingClass)
DECLARE_TYPE_ID(Thing)
DECLARE_TYPE_ID(ThingDescriptor)

DECLARE_TYPE_ID(ParamType)
DECLARE_TYPE_ID(Param)
DECLARE_TYPE_ID(EventType)
DECLARE_TYPE_ID(StateType)
DECLARE_TYPE_ID(ActionType)
DECLARE_TYPE_ID(Plugin)
DECLARE_TYPE_ID(Rule)
DECLARE_TYPE_ID(Browser)
DECLARE_TYPE_ID(IOConnection)

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
        UnitMilliSeconds,
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
        UnitPartsPerBillion,
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
        UnitOhm,
        UnitMicroSiemensPerCentimeter,
        UnitDuration,
        UnitNewton,
        UnitNewtonMeter,
        UnitRpm,
        UnitMilligramPerLiter,
        UnitLiter,
        UnitMicroGrammPerCubicalMeter,
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

    enum IOType {
        IOTypeNone,
        IOTypeDigitalInput,
        IOTypeDigitalOutput,
        IOTypeAnalogInput,
        IOTypeAnalogOutput
    };
    Q_ENUM(IOType)

    enum StateValueFilter {
        StateValueFilterNone,
        StateValueFilterAdaptive
    };
    Q_ENUM(StateValueFilter)

    enum PermissionScope {
        PermissionScopeNone             = 0x0000,
        PermissionScopeControlThings    = 0x0001,
        PermissionScopeConfigureThings  = 0x0003,
        PermissionScopeExecuteRules     = 0x0010,
        PermissionScopeConfigureRules   = 0x0030,
        PermissionScopeAdmin            = 0xFFFF,
    };
    Q_ENUM(PermissionScope)
    Q_DECLARE_FLAGS(PermissionScopes, PermissionScope)
    Q_FLAG(PermissionScopes)

    static PermissionScopes scopesFromStringList(const QStringList &scopeList);
    static PermissionScope scopeFromString(const QString &scopeString);
    static QStringList scopesToStringList(PermissionScopes scopes);
    static QString scopeToString(PermissionScope scope);

    enum LoggingType {
        LoggingTypeDiscrete,
        LoggingTypeSampled,
    };
    Q_ENUM(LoggingType)

    enum SampleRate {
        SampleRateAny = 0,
        SampleRate1Min = 1,
        SampleRate15Mins = 15,
        SampleRate1Hour = 60,
        SampleRate3Hours = 180,
        SampleRate1Day = 1440,
        SampleRate1Week = 10080,
        SampleRate1Month = 43200,
        SampleRate1Year = 525600
    };
    Q_ENUM(SampleRate)

};

Q_DECLARE_METATYPE(Types::InputType)
Q_DECLARE_METATYPE(Types::Unit)
Q_DECLARE_METATYPE(Types::ValueOperator)
Q_DECLARE_METATYPE(Types::StateOperator)

#endif // TYPEUTILS_H
