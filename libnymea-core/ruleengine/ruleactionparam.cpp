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

#include "ruleactionparam.h"

RuleActionParam::RuleActionParam(const Param &param)
    : m_paramTypeId(param.paramTypeId())
    , m_value(param.value())
{}

RuleActionParam::RuleActionParam(const ParamTypeId &paramTypeId, const QVariant &value)
    : m_paramTypeId(paramTypeId)
    , m_value(value)
{}

RuleActionParam::RuleActionParam(const ParamTypeId &paramTypeId, const EventTypeId &eventTypeId, const ParamTypeId &eventParamTypeId)
    : m_paramTypeId(paramTypeId)
    , m_eventTypeId(eventTypeId)
    , m_eventParamTypeId(eventParamTypeId)
{}

RuleActionParam::RuleActionParam(const ParamTypeId &paramTypeId, const ThingId &stateThingId, const StateTypeId &stateTypeId)
    : m_paramTypeId(paramTypeId)
    , m_stateThingId(stateThingId)
    , m_stateTypeId(stateTypeId)
{}

RuleActionParam::RuleActionParam(const QString &paramName, const QVariant &value)
    : m_paramName(paramName)
    , m_value(value)
{}

RuleActionParam::RuleActionParam(const QString &paramName, const EventTypeId &eventTypeId, const ParamTypeId &eventParamTypeId)
    : m_paramName(paramName)
    , m_eventTypeId(eventTypeId)
    , m_eventParamTypeId(eventParamTypeId)
{}

RuleActionParam::RuleActionParam(const QString &paramName, const ThingId &stateThingId, const StateTypeId &stateTypeId)
    : m_paramName(paramName)
    , m_stateThingId(stateThingId)
    , m_stateTypeId(stateTypeId)
{}

ParamTypeId RuleActionParam::paramTypeId() const
{
    return m_paramTypeId;
}

void RuleActionParam::setParamTypeId(const ParamTypeId &paramTypeId)
{
    m_paramTypeId = paramTypeId;
}

QString RuleActionParam::paramName() const
{
    return m_paramName;
}

void RuleActionParam::setParamName(const QString &paramName)
{
    m_paramName = paramName;
}

QVariant RuleActionParam::value() const
{
    return m_value;
}

void RuleActionParam::setValue(const QVariant &value)
{
    m_value = value;
}

EventTypeId RuleActionParam::eventTypeId() const
{
    return m_eventTypeId;
}

void RuleActionParam::setEventTypeId(const EventTypeId &eventTypeId)
{
    m_eventTypeId = eventTypeId;
}

ParamTypeId RuleActionParam::eventParamTypeId() const
{
    return m_eventParamTypeId;
}

void RuleActionParam::setEventParamTypeId(const ParamTypeId &eventParamTypeId)
{
    m_eventParamTypeId = eventParamTypeId;
}

ThingId RuleActionParam::stateThingId() const
{
    return m_stateThingId;
}

void RuleActionParam::setStateThingId(const ThingId &thingId)
{
    m_stateThingId = thingId;
}

StateTypeId RuleActionParam::stateTypeId() const
{
    return m_stateTypeId;
}

void RuleActionParam::setStateTypeId(const StateTypeId &stateTypeId)
{
    m_stateTypeId = stateTypeId;
}

bool RuleActionParam::isValid() const
{
    if (m_paramTypeId.isNull() && m_paramName.isNull()) {
        return false;
    }
    return isValueBased() ^ isEventBased() ^ isStateBased();
}

bool RuleActionParam::isValueBased() const
{
    return !m_value.isNull();
}

bool RuleActionParam::isEventBased() const
{
    return !m_eventTypeId.isNull() && !m_eventParamTypeId.isNull();
}

bool RuleActionParam::isStateBased() const
{
    return !m_stateThingId.isNull() && !m_stateTypeId.isNull();
}

QDebug operator<<(QDebug dbg, const RuleActionParam &ruleActionParam)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "RuleActionParam(ParamTypeId: " << ruleActionParam.paramTypeId().toString() << ", Name:" << ruleActionParam.paramName()
                  << ", Value:" << ruleActionParam.value();
    if (ruleActionParam.eventTypeId() != EventTypeId()) {
        dbg.nospace() << ", EventTypeId:" << ruleActionParam.eventTypeId().toString() << ", EventParamTypeId:" << ruleActionParam.eventParamTypeId().toString() << ")";
    } else {
        dbg.nospace() << ")";
    }
    return dbg;
}

bool RuleActionParams::hasParam(const ParamTypeId &ruleActionParamTypeId) const
{
    return m_ids.contains(ruleActionParamTypeId);
}

bool RuleActionParams::hasParam(const QString &ruleActionParamName) const
{
    foreach (const RuleActionParam &param, *this) {
        if (param.paramName() == ruleActionParamName) {
            return true;
        }
    }
    return false;
}

QVariant RuleActionParams::paramValue(const ParamTypeId &ruleActionParamTypeId) const
{
    foreach (const RuleActionParam &param, *this) {
        if (param.paramTypeId() == ruleActionParamTypeId) {
            return param.value();
        }
    }

    return QVariant();
}

bool RuleActionParams::setParamValue(const ParamTypeId &ruleActionParamTypeId, const QVariant &value)
{
    for (int i = 0; i < count(); i++) {
        if (this->operator[](i).paramTypeId() == ruleActionParamTypeId) {
            this->operator[](i).setValue(value);
            return true;
        }
    }

    return false;
}

RuleActionParams RuleActionParams::operator<<(const RuleActionParam &ruleActionParam)
{
    this->append(ruleActionParam);
    m_ids.append(ruleActionParam.paramTypeId());
    return *this;
}

QDebug operator<<(QDebug dbg, const RuleActionParams &ruleActionParams)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "RuleActionParamList (count:" << ruleActionParams.count() << ")" << '\n';
    for (int i = 0; i < ruleActionParams.count(); i++) {
        dbg.nospace() << "     " << i << ": " << ruleActionParams.at(i) << '\n';
    }

    return dbg;
}

QVariant RuleActionParams::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void RuleActionParams::put(const QVariant &variant)
{
    append(variant.value<RuleActionParam>());
}
