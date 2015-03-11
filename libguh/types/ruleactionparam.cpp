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
    \brief Holds the parameters for a \l{RuleAction}.

    \ingroup types
    \inmodule libguh

    A RuleActionParam allows rules to take over an \l{Event} parameter into a rule
    \l{RuleAction}.

    \sa Rule, RuleAction,
*/

#include "ruleactionparam.h"

/*! Constructs a \l{RuleActionParam} with the given \a param.
 *  \sa Param, */
RuleActionParam::RuleActionParam(const Param &param) :
    m_name(param.name()),
    m_value(param.value())
{
}

/*! Constructs a \l{RuleActionParam} with the given \a name, \a value and \a eventTypeId.
 *  \sa Param, Event, */
RuleActionParam::RuleActionParam(const QString &name, const QVariant &value, const EventTypeId &eventTypeId) :
    m_name(name),
    m_value(value),
    m_eventTypeId(eventTypeId)
{
}

/*! Returns the name of this RuleActionParam. */
QString RuleActionParam::name() const
{
    return m_name;
}

/*! Sets the \a name of this RuleActionParam. */
void RuleActionParam::setName(const QString &name)
{
    m_name = name;
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

/*! Returns true if the name and the value of this RuleActionParam are set.*/
bool RuleActionParam::isValid() const
{
    return !m_name.isEmpty() && m_value.isValid();
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

/*! Writes the name, value and eventId of the given \a ruleActionParam to \a dbg. */
QDebug operator<<(QDebug dbg, const RuleActionParam &ruleActionParam)
{
    dbg.nospace() << "RuleActionParam(Name: " << ruleActionParam.name() << ", Value:" << ruleActionParam.value() << ", EventTypeId:" << ruleActionParam.eventTypeId().toString() << ")";

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

/*! Returns true if this \l{RuleActionParamList} contains a RuleActionParam with the given \a paramName. */
bool RuleActionParamList::hasParam(const QString &ruleActionParamName) const
{
    foreach (const RuleActionParam &param, *this) {
        if (param.name() == ruleActionParamName) {
            return true;
        }
    }
    return false;
}

/*! Returns the value of the \l{RuleActionParam} with the given \a ruleActionParamName. */
QVariant RuleActionParamList::paramValue(const QString &ruleActionParamName) const
{
    foreach (const RuleActionParam &param, *this) {
        if (param.name() == ruleActionParamName) {
            return param.value();
        }
    }
    return QVariant();
}

/*! Sets the value of the \l{RuleActionParam} with the given \a ruleActionParamName to the given \a value. */
void RuleActionParamList::setParamValue(const QString &ruleActionParamName, const QVariant &value)
{
    for (int i = 0; i < count(); i++) {
        if (this->operator [](i).name()  == ruleActionParamName) {
            this->operator [](i).setValue(value);
            return;
        }
    }
}

/*! Appends the given \a ruleActionParam to a RuleActionParamList. */
RuleActionParamList RuleActionParamList::operator<<(const RuleActionParam &ruleActionParam)
{
    this->append(ruleActionParam);
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
