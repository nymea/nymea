/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
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

#include "param.h"
#include "libnymea.h"
#include "typeutils.h"

class LIBNYMEA_EXPORT RuleActionParam
{
public:
    RuleActionParam(const Param &param = Param());
    RuleActionParam(const ParamTypeId &paramTypeId, const QVariant &value = QVariant(), const EventTypeId &eventTypeId = EventTypeId(), const ParamTypeId &eventParamTypeId = ParamTypeId());
    RuleActionParam(const QString &paramName, const QVariant &value = QVariant(), const EventTypeId &eventTypeId = EventTypeId(), const ParamTypeId &eventParamTypeId = ParamTypeId());

    ParamTypeId paramTypeId() const;
    QString paramName() const;

    ParamTypeId eventParamTypeId() const;
    void setEventParamTypeId(const ParamTypeId &eventParamTypeId);

    QVariant value() const;
    void setValue(const QVariant &value);

    bool isValid() const;

    EventTypeId eventTypeId() const;
    void setEventTypeId(const EventTypeId &eventTypeId);

private:
    ParamTypeId m_paramTypeId;
    QString m_paramName;
    QVariant m_value;
    EventTypeId m_eventTypeId;
    ParamTypeId m_eventParamTypeId;
};

Q_DECLARE_METATYPE(RuleActionParam)
QDebug operator<<(QDebug dbg, const RuleActionParam &ruleActionParam);

class LIBNYMEA_EXPORT RuleActionParamList: public QList<RuleActionParam>
{
public:
    bool hasParam(const ParamTypeId &ruleActionParamTypeId) const;
    bool hasParam(const QString &ruleActionParamName) const;
    QVariant paramValue(const ParamTypeId &ruleActionParamName) const;
    bool setParamValue(const ParamTypeId &ruleActionParamTypeId, const QVariant &value);
    RuleActionParamList operator<<(const RuleActionParam &ruleActionParam);

private:
    QList<ParamTypeId> m_ids;
};
QDebug operator<<(QDebug dbg, const RuleActionParamList &ruleActionParams);


#endif // RULEACTIONPARAM_H
