/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

/*!
    \class Rule
    \brief This class represents a rule.

    \ingroup rules
    \inmodule server

    A Rule is always evented by an \l{Event}, has \l{State}{States}
    to be compared and \l{Action}{Actions} to be executed. Additionally a
    Rule is either of type \l{Rule::RuleTypeAll} or \l{Rule::RuleTypeAny}
    which determines if all or any of the \l{State}{States} must be matching
    in order for the \l{Action}{Actions} to be executed.

    \sa Event, State, Action
*/

/*! \enum Rule::RuleType

    Note: There is no RuleTypeNone. If you don't want to compare any
    states, construct a rule without states in which case it doesn't
    matter what the Rule's type is.

    \value RuleTypeAll
    All States must match in order for the Rule to apply.
    \value RuleTypeAny
    Any State must match in order for the Rule to apply.
*/

#include "rule.h"

#include <QDebug>

/*! Constructs a Rule with the given \a id, \a event, \a states and \a actions. The ruleType will default to
 \l{Rule::RuleTypeAll}.*/
Rule::Rule(const QUuid &id, const Event &event, const QList<State> &states, const QList<Action> &actions):
    m_id(id),
    m_events(QList<Event>() << event),
    m_states(states),
    m_actions(actions),
    m_ruleType(RuleTypeAll)
{
}

/*! Returns the id or the Rule. */
QUuid Rule::id() const
{
    return m_id;
}

/*! Returns the \l{Event} that events this Rule.*/
QList<Event> Rule::events() const
{
    return m_events;
}

/*! Returns the \l{State}{States} that need to be matching in order for this to Rule apply. */
QList<State> Rule::states() const
{
    return m_states;
}

/*! Returns the \l{Action}{Actions} to be executed when this Rule is eventd and states match. */
QList<Action> Rule::actions() const
{
    return m_actions;
}

/*! Returns the type of the rule. This defines how states are compared. The default is \l{Rule::RuleTypeAll}.*/
Rule::RuleType Rule::ruleType() const
{
    return m_ruleType;
}

/*! Set the type of the rule to \a ruleType. This defines how states are compared. The default is \l{Rule::RuleTypeAll}.*/
void Rule::setRuleType(Rule::RuleType ruleType)
{
    m_ruleType = ruleType;
}
