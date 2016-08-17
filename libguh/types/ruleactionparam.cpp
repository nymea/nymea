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

/*!
    \class RuleActionParam
    \brief Holds the parameters for a \l{RuleAction}.

    \ingroup guh-types
    \ingroup rules
    \inmodule libguh

    A RuleActionParam allows rules to take over an \l{Event} parameter into a rule
    \l{RuleAction}.

    \sa guhserver::Rule, RuleAction,
*/

#include "ruleactionparam.h"

/*! Constructs a \l{RuleActionParam} with the given \a param.
 *  \sa Param, */
RuleActionParam::RuleActionParam(const Param &param) :
    m_paramTypeId(param.paramTypeId()),
    m_value(param.value()),
    m_eventTypeId(EventTypeId()),
    m_eventParamName(QString())
{
}

/*! Constructs a \l{RuleActionParam} with the given \a name, \a value, \a eventTypeId and \a eventParamName.
 *  \sa Param, Event, */
RuleActionParam::RuleActionParam(const ParamTypeId &paramTypeId, const QVariant &value, const EventTypeId &eventTypeId, const QString &eventParamName) :
    m_paramTypeId(paramTypeId),
    m_value(value),
    m_eventTypeId(eventTypeId),
    m_eventParamName(eventParamName)
{
}

/*! Returns the name of the eventParam for this RuleActionParam. */
QString RuleActionParam::eventParamName() const
{
    return m_eventParamName;
}

/*! Sets the \a eventParamName of this RuleActionParam. */
void RuleActionParam::setEventParamName(const QString &eventParamName)
{
    m_eventParamName = eventParamName;
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

/*! Returns true if the name and value or the name, eventTypeId and eventParamName of this RuleActionParam are set.*/
bool RuleActionParam::isValid() const
{
    bool validValue = (!m_paramTypeId.isNull() && m_value.isValid() && m_eventTypeId.isNull() && m_eventParamName.isEmpty());
    bool validEvent = (!m_paramTypeId.isNull() && m_eventTypeId.isNull() && !m_eventParamName.isEmpty() && !m_value.isValid());
    return validValue ^ validEvent;
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

/*! Writes the name, value, eventId and eventParamName of the given \a ruleActionParam to \a dbg. */
QDebug operator<<(QDebug dbg, const RuleActionParam &ruleActionParam)
{
    dbg.nospace() << "RuleActionParam(ParamTypeId: " << ruleActionParam.paramTypeId() << ", Value:" << ruleActionParam.value();
    if (ruleActionParam.eventTypeId() != EventTypeId()) {
       dbg.nospace() << ", EventTypeId:" << ruleActionParam.eventTypeId().toString() << ", EventParamName:" << ruleActionParam.eventParamName() << ")";
    } else {
       dbg.nospace() << ")";
    }
    return dbg.space();
}

// ActionTypeParamList
/*!
    \class RuleActionParamList
    \brief Holds a list of \l{RuleActionParam}{RuleActionParams}

    \ingroup types
    \inmodule libguh

    \sa RuleActionParam, RuleAction,
*/

/*! Returns true if this \l{RuleActionParamList} contains a \l{RuleActionParam} with the given \a ruleActionParamTypeId. */
bool RuleActionParamList::hasParam(const ParamTypeId &ruleActionParamTypeId) const
{
    return m_ids.contains(ruleActionParamTypeId);
}

/*! Returns the value of the \l{RuleActionParam} with the given \a ruleActionParamTypeId. */
QVariant RuleActionParamList::paramValue(const ParamTypeId &ruleActionParamTypeId) const
{
    foreach (const RuleActionParam &param, *this) {
        if (param.paramTypeId() == ruleActionParamTypeId) {
            return param.value();
        }
    }
    return QVariant();
}

/*! Sets the value of the \l{RuleActionParam} with the given \a ruleActionParamTypeId to the given \a value. */
void RuleActionParamList::setParamValue(const ParamTypeId &ruleActionParamTypeId, const QVariant &value)
{
    for (int i = 0; i < count(); i++) {
        if (this->operator [](i).paramTypeId()  == ruleActionParamTypeId) {
            this->operator [](i).setValue(value);
            return;
        }
    }
}

/*! Appends the given \a ruleActionParam to a RuleActionParamList. */
RuleActionParamList RuleActionParamList::operator<<(const RuleActionParam &ruleActionParam)
{
    this->append(ruleActionParam);
    m_ids.append(ruleActionParam.paramTypeId());
    return *this;
}

/*! Writes the ruleActionParam of the given \a ruleActionParams to \a dbg. */
QDebug operator<<(QDebug dbg, const RuleActionParamList &ruleActionParams)
{
    dbg.nospace() << "RuleActionParamList (count:" << ruleActionParams.count() << ")" << endl;
    for (int i = 0; i < ruleActionParams.count(); i++ ) {
        dbg.nospace() << "     " << i << ": " << ruleActionParams.at(i) << endl;
    }

    return dbg.space();
}
