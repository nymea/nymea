/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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
    \brief Holds the parameters for a rule action.

    \ingroup types
    \inmodule libguh

    A RuleActionParam allows in rules to take over an \l{Event} parameter into a rule
    \l{Action}.

    \sa Rule, Action, Param, ParamType, ParamDescriptor
*/

#include "ruleactionparam.h"

/*! Constructs a \l{RuleActionParam} with the given \a param.
 *  \sa Param, */
RuleActionParam::RuleActionParam(const Param &param)
{
    m_name = param.name();
    m_value = param.value();
}

/*! Constructs a \l{RuleActionParam} with the given \a name, \a value and \a eventId.
 *  \sa Param, Event, */
RuleActionParam::RuleActionParam(const QString &name, const QVariant &value, const EventId &eventId) :
    m_name(name),
    m_value(value),
    m_eventId(eventId)
{
}

/*! Return the EventId of the \l{Event} with the \l{Param} which will be taken over in the \l{Rule}{rule} \l{Action}{action}. */
EventId RuleActionParam::eventId() const
{
    return m_eventId;
}

/*! Sets the \a eventId of the \l{Event} with the \l{Param} which will be taken over in the \l{Rule}{rule} \l{Action}{action}. */
void RuleActionParam::setEventId(const EventId &eventId)
{
    m_eventId = eventId;
}

/*! Writes the name and value of the given \a param to \a dbg. */
QDebug operator<<(QDebug dbg, const RuleActionParam &param)
{
    dbg.nospace() << "RuleActionParam(Name: " << param.name() << ", Value:" << param.value() << ", EventId:" << param.eventId().toString() << ")";

    return dbg.space();
}
