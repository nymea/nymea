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

#ifndef RULEACTIONPARAM_H
#define RULEACTIONPARAM_H

#include <QDebug>
#include <QString>
#include <QVariant>

#include "types/param.h"
#include "libnymea.h"
#include "typeutils.h"

class LIBNYMEA_EXPORT RuleActionParam
{
    Q_GADGET
    Q_PROPERTY(QUuid paramTypeId READ paramTypeId WRITE setParamTypeId USER true)
    Q_PROPERTY(QString paramName READ paramName WRITE setParamName USER true)
    Q_PROPERTY(QVariant value READ value WRITE setValue USER true)
    Q_PROPERTY(QUuid eventTypeId READ eventTypeId WRITE setEventTypeId USER true)
    Q_PROPERTY(QUuid eventParamTypeId READ eventParamTypeId WRITE setEventParamTypeId USER true)
    Q_PROPERTY(QUuid stateDeviceId READ stateDeviceId WRITE setStateDeviceId USER true)
    Q_PROPERTY(QUuid stateTypeId READ stateTypeId WRITE setStateTypeId USER true)

public:
    RuleActionParam(const Param &param = Param());
    RuleActionParam(const ParamTypeId &paramTypeId, const QVariant &value = QVariant());
    RuleActionParam(const ParamTypeId &paramTypeId, const EventTypeId &eventTypeId, const ParamTypeId &eventParamTypeId);
    RuleActionParam(const ParamTypeId &paramTypeId, const DeviceId &stateDeviceId, const StateTypeId &stateTypeId);
    RuleActionParam(const QString &paramName, const QVariant &value = QVariant());
    RuleActionParam(const QString &paramName, const EventTypeId &eventTypeId, const ParamTypeId &eventParamTypeId);
    RuleActionParam(const QString &paramName, const DeviceId &stateDeviceId, const StateTypeId &stateTypeId);

    ParamTypeId paramTypeId() const;
    void setParamTypeId(const ParamTypeId &paramTypeId);

    QString paramName() const;
    void setParamName(const QString &paramName);

    bool isValid() const;
    bool isValueBased() const;
    bool isEventBased() const;
    bool isStateBased() const;

    QVariant value() const;
    void setValue(const QVariant &value);

    EventTypeId eventTypeId() const;
    void setEventTypeId(const EventTypeId &eventTypeId);

    ParamTypeId eventParamTypeId() const;
    void setEventParamTypeId(const ParamTypeId &eventParamTypeId);

    DeviceId stateDeviceId() const;
    void setStateDeviceId(const DeviceId &deviceId);

    StateTypeId stateTypeId() const;
    void setStateTypeId(const StateTypeId &stateTypeId);

private:
    ParamTypeId m_paramTypeId;
    QString m_paramName;
    QVariant m_value;

    EventTypeId m_eventTypeId;
    ParamTypeId m_eventParamTypeId;

    DeviceId m_stateDeviceId;
    StateTypeId m_stateTypeId;
};

Q_DECLARE_METATYPE(RuleActionParam)
QDebug operator<<(QDebug dbg, const RuleActionParam &ruleActionParam);

class LIBNYMEA_EXPORT RuleActionParams: public QList<RuleActionParam>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    bool hasParam(const ParamTypeId &ruleActionParamTypeId) const;
    bool hasParam(const QString &ruleActionParamName) const;
    Q_INVOKABLE QVariant get(int index) const;
    QVariant paramValue(const ParamTypeId &ruleActionParamName) const;
    bool setParamValue(const ParamTypeId &ruleActionParamTypeId, const QVariant &value);
    RuleActionParams operator<<(const RuleActionParam &ruleActionParam);

private:
    QList<ParamTypeId> m_ids;
};
QDebug operator<<(QDebug dbg, const RuleActionParams &ruleActionParams);


#endif // RULEACTIONPARAM_H
