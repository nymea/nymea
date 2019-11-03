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

/*!
    \class RuleActionParam
    \brief Holds the parameters for a \l{RuleAction}.

    \ingroup nymea-types
    \ingroup rules
    \inmodule libnymea

    A RuleActionParam allows rules to take over an \l{Event} parameter into a rule
    \l{RuleAction}.

    RuleActionParams are identified by either paramTypeId or paramName (for interface based actions).

    The parameter value can either be a static \l{value}, a pair of \l{EventTypeId} and \l{ParamTypeId} or a pair of
    \l{DeviceId} and \l{StateTypeId}.

    When composing the actual Param for the executeAction() call the value is generated as follows:
    - Static value params are filled with \l{RuleActionParam::paramTypeId()} and the \l{RuleActionParam::value()}
    - Event based actions are filled with \l{RuleActionParam::paramTypeId()} and the param value of the event that triggered this rule, identified by \l{RuleActionParam::eventTypeId()} and \l{RuleActionParam::eventParamTypeId()}
    - State based actions are filled with \l{RuleActionParam::paramTypeId()} and the current value of the state identified by \l{RuleActionParam::deviceId()} and \l{RuleActionParam::stateTypeId()}

    If the param types are not matching, nymea will do a best effort to cast the values. E.g. a RuleActionParam for
    a param of type "string" and a state of type "int" would cast the integer to a string which would always work.
    However, the other way round, having a parameter requiring an "int" value, and reading the value from a state of type
    "string" might work, if the string does only hold numbers but would fail.

    \sa nymeaserver::Rule, RuleAction,
*/

#include "ruleactionparam.h"

/*! Constructs a \l{RuleActionParam} with the given \a param.
 *  \sa Param, */
RuleActionParam::RuleActionParam(const Param &param) :
    m_paramTypeId(param.paramTypeId()),
    m_value(param.value())
{
}

/*! Constructs a \l{RuleActionParam} with the given \a paramTypeId and \a value.
 *  \sa Param, Event, */
RuleActionParam::RuleActionParam(const ParamTypeId &paramTypeId, const QVariant &value):
    m_paramTypeId(paramTypeId),
    m_value(value)
{

}

/*! Constructs a \l{RuleActionParam} with the given \a paramTypeId, \a eventTypeId and \a eventParamTypeId.
 *  \sa Param, Event, */
RuleActionParam::RuleActionParam(const ParamTypeId &paramTypeId, const EventTypeId &eventTypeId, const ParamTypeId &eventParamTypeId):
    m_paramTypeId(paramTypeId),
    m_eventTypeId(eventTypeId),
    m_eventParamTypeId(eventParamTypeId)
{

}

/*! Constructs a \l{RuleActionParam} with the given \a paramTypeId, \a stateDeviceId and \a stateTypeId.
 *  \sa Param, Event, */
RuleActionParam::RuleActionParam(const ParamTypeId &paramTypeId, const DeviceId &stateDeviceId, const StateTypeId &stateTypeId):
    m_paramTypeId(paramTypeId),
    m_stateDeviceId(stateDeviceId),
    m_stateTypeId(stateTypeId)
{

}

/*! Constructs a \l{RuleActionParam} with the given \a paramName and \a value.
 *  \sa Param, Event, */
RuleActionParam::RuleActionParam(const QString &paramName, const QVariant &value):
    m_paramName(paramName),
    m_value(value)
{

}

/*! Constructs a \l{RuleActionParam} with the given \a paramName, \a eventTypeId and \a eventParamTypeId.
 *  \sa Param, Event, */
RuleActionParam::RuleActionParam(const QString &paramName, const EventTypeId &eventTypeId, const ParamTypeId &eventParamTypeId):
    m_paramName(paramName),
    m_eventTypeId(eventTypeId),
    m_eventParamTypeId(eventParamTypeId)
{

}

/*! Constructs a \l{RuleActionParam} with the given \a paramName, \a stateDeviceId and \a stateTypeId.
 *  \sa Param, Event, */
RuleActionParam::RuleActionParam(const QString &paramName, const DeviceId &stateDeviceId, const StateTypeId &stateTypeId):
    m_paramName(paramName),
    m_stateDeviceId(stateDeviceId),
    m_stateTypeId(stateTypeId)
{

}

/*! Returns the \l ParamTypeId of this \l RuleActionParam. */
ParamTypeId RuleActionParam::paramTypeId() const
{
    return m_paramTypeId;
}

void RuleActionParam::setParamTypeId(const ParamTypeId &paramTypeId)
{
    m_paramTypeId = paramTypeId;
}

/*! Returns the name of this RuleActionParam. */
QString RuleActionParam::paramName() const
{
    return m_paramName;
}

void RuleActionParam::setParamName(const QString &paramName)
{
    m_paramName = paramName;
}

/*! Returns the value of this RuleActionParam. */
QVariant RuleActionParam::value() const
{
    return m_value;
}

/*! Sets the \a value of this RuleActionParam. */
void RuleActionParam::setValue(const QVariant &value)
{
    m_value = value;
}

/*! Return the EventTypeId of the \l{Event} with the \l{Param} which will be taken over in the  \l{RuleAction}. */
EventTypeId RuleActionParam::eventTypeId() const
{
    return m_eventTypeId;
}

/*! Sets the \a eventTypeId of the \l{Event} with the \l{Param} which will be taken over in the \l{RuleAction}. */
void RuleActionParam::setEventTypeId(const EventTypeId &eventTypeId)
{
    m_eventTypeId = eventTypeId;
}

/*! Returns the eventParamTypeId of this RuleActionParam. */
ParamTypeId RuleActionParam::eventParamTypeId() const
{
    return m_eventParamTypeId;
}

/*! Sets the \a eventParamTypeId of this RuleActionParam. */
void RuleActionParam::setEventParamTypeId(const ParamTypeId &eventParamTypeId)
{
    m_eventParamTypeId = eventParamTypeId;
}

/*! Returns the deviceId identifying the device to use a state value from. */
DeviceId RuleActionParam::stateDeviceId() const
{
    return m_stateDeviceId;
}

/*! Sets the deviceId identifying the device to use a state value from. */
void RuleActionParam::setStateDeviceId(const DeviceId &stateDeviceId)
{
    m_stateDeviceId = stateDeviceId;
}

/*! Returns the stateTypeId identifying the state to use the value. */
StateTypeId RuleActionParam::stateTypeId() const
{
    return m_stateTypeId;
}

/*! Sets the stateTypeId identifying the state to use the value from. */
void RuleActionParam::setStateTypeId(const StateTypeId &stateTypeId)
{
    m_stateTypeId = stateTypeId;
}

/*! Returns true if the \tt{(paramTypeId AND value) XOR (paramTypeId AND eventTypeId AND eventParamName)} of this RuleActionParam are set.*/
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
    return !m_stateDeviceId.isNull() && !m_stateTypeId.isNull();
}

/*! Writes the paramTypeId, value, eventId and eventParamTypeId of the given \a ruleActionParam to \a dbg. */
QDebug operator<<(QDebug dbg, const RuleActionParam &ruleActionParam)
{
    dbg.nospace() << "RuleActionParam(ParamTypeId: " << ruleActionParam.paramTypeId().toString() << ", Name:" << ruleActionParam.paramName() << ", Value:" << ruleActionParam.value();
    if (ruleActionParam.eventTypeId() != EventTypeId()) {
       dbg.nospace() << ", EventTypeId:" << ruleActionParam.eventTypeId().toString() << ", EventParamTypeId:" << ruleActionParam.eventParamTypeId().toString() << ")";
    } else {
       dbg.nospace() << ")";
    }
    return dbg;
}

// ActionTypeParamList
/*!
    \class RuleActionParamList
    \brief Holds a list of \l{RuleActionParam}{RuleActionParams}

    \ingroup types
    \inmodule libnymea

    \sa RuleActionParam, RuleAction,
*/

/*! Returns true if this \l{RuleActionParamList} contains a \l{RuleActionParam} with the given \a ruleActionParamTypeId. */
bool RuleActionParams::hasParam(const ParamTypeId &ruleActionParamTypeId) const
{
    return m_ids.contains(ruleActionParamTypeId);
}

/*! Returns true if this \l{RuleActionParamList} contains a \l{RuleActionParam} with the given \a ruleActionParamName. */
bool RuleActionParams::hasParam(const QString &ruleActionParamName) const
{
    foreach (const RuleActionParam &param, *this) {
        if (param.paramName() == ruleActionParamName) {
            return true;
        }
    }
    return false;
}

/*! Returns the value of the \l{RuleActionParam} with the given \a ruleActionParamTypeId. */
QVariant RuleActionParams::paramValue(const ParamTypeId &ruleActionParamTypeId) const
{
    foreach (const RuleActionParam &param, *this) {
        if (param.paramTypeId() == ruleActionParamTypeId) {
            return param.value();
        }
    }

    return QVariant();
}

/*! Returns true if the \a value of the \l{RuleActionParam} with the given \a ruleActionParamTypeId could be set successfully. */
bool RuleActionParams::setParamValue(const ParamTypeId &ruleActionParamTypeId, const QVariant &value)
{
    for (int i = 0; i < count(); i++) {
        if (this->operator [](i).paramTypeId()  == ruleActionParamTypeId) {
            this->operator [](i).setValue(value);
            return true;
        }
    }

    return false;
}

/*! Appends the given \a ruleActionParam to a RuleActionParamList. */
RuleActionParams RuleActionParams::operator<<(const RuleActionParam &ruleActionParam)
{
    this->append(ruleActionParam);
    m_ids.append(ruleActionParam.paramTypeId());
    return *this;
}

/*! Writes the ruleActionParam of the given \a ruleActionParams to \a dbg. */
QDebug operator<<(QDebug dbg, const RuleActionParams &ruleActionParams)
{
    dbg.nospace() << "RuleActionParamList (count:" << ruleActionParams.count() << ")" << endl;
    for (int i = 0; i < ruleActionParams.count(); i++ ) {
        dbg.nospace() << "     " << i << ": " << ruleActionParams.at(i) << endl;
    }

    return dbg.space();
}

QVariant RuleActionParams::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void RuleActionParams::put(const QVariant &variant)
{
    append(variant.value<RuleActionParam>());
}
