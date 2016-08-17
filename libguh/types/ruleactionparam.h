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

#ifndef RULEACTIONPARAM_H
#define RULEACTIONPARAM_H

#include <QDebug>
#include <QString>
#include <QVariant>

#include "param.h"
#include "libguh.h"
#include "typeutils.h"

class LIBGUH_EXPORT RuleActionParam
{
public:
    RuleActionParam(const Param &param = Param());
    RuleActionParam(const ParamTypeId &paramTypeId, const QVariant &value = QVariant(), const EventTypeId &eventTypeId = EventTypeId(), const QString &eventParamName = QString());

    ParamTypeId paramTypeId() const;

    QString eventParamName() const;
    void setEventParamName(const QString &eventParamName);

    QVariant value() const;
    void setValue(const QVariant &value);

    bool isValid() const;

    EventTypeId eventTypeId() const;
    void setEventTypeId(const EventTypeId &eventTypeId);

private:
    ParamTypeId m_paramTypeId;
    QVariant m_value;
    EventTypeId m_eventTypeId;
    QString m_eventParamName;
};

Q_DECLARE_METATYPE(RuleActionParam)
QDebug operator<<(QDebug dbg, const RuleActionParam &ruleActionParam);

class LIBGUH_EXPORT RuleActionParamList: public QList<RuleActionParam>
{
public:
    bool hasParam(const ParamTypeId &ruleActionParamTypeId) const;
    QVariant paramValue(const ParamTypeId &ruleActionParamName) const;
    void setParamValue(const ParamTypeId &ruleActionParamTypeId, const QVariant &value);
    RuleActionParamList operator<<(const RuleActionParam &ruleActionParam);

private:
    QList<ParamTypeId> m_ids;
};
QDebug operator<<(QDebug dbg, const RuleActionParamList &ruleActionParams);


#endif // RULEACTIONPARAM_H
