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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
    Q_PROPERTY(QUuid stateThingId READ stateThingId WRITE setStateThingId USER true)
    Q_PROPERTY(QUuid stateTypeId READ stateTypeId WRITE setStateTypeId USER true)

public:
    RuleActionParam(const Param &param = Param());
    RuleActionParam(const ParamTypeId &paramTypeId, const QVariant &value = QVariant());
    RuleActionParam(const ParamTypeId &paramTypeId, const EventTypeId &eventTypeId, const ParamTypeId &eventParamTypeId);
    RuleActionParam(const ParamTypeId &paramTypeId, const ThingId &stateThingId, const StateTypeId &stateTypeId);
    RuleActionParam(const QString &paramName, const QVariant &value = QVariant());
    RuleActionParam(const QString &paramName, const EventTypeId &eventTypeId, const ParamTypeId &eventParamTypeId);
    RuleActionParam(const QString &paramName, const ThingId &stateThingId, const StateTypeId &stateTypeId);

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

    ThingId stateThingId() const;
    void setStateThingId(const ThingId &thingId);

    StateTypeId stateTypeId() const;
    void setStateTypeId(const StateTypeId &stateTypeId);

private:
    ParamTypeId m_paramTypeId;
    QString m_paramName;
    QVariant m_value;

    EventTypeId m_eventTypeId;
    ParamTypeId m_eventParamTypeId;

    ThingId m_stateThingId;
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
    Q_INVOKABLE void put(const QVariant &variant);
    QVariant paramValue(const ParamTypeId &ruleActionParamName) const;
    bool setParamValue(const ParamTypeId &ruleActionParamTypeId, const QVariant &value);
    RuleActionParams operator<<(const RuleActionParam &ruleActionParam);

private:
    QList<ParamTypeId> m_ids;
};
QDebug operator<<(QDebug dbg, const RuleActionParams &ruleActionParams);


#endif // RULEACTIONPARAM_H
