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
    \class Rule
    \brief This class represents a rule.

    \ingroup rules
    \inmodule server

    A Rule is always triggered by an \l{EventDescriptor}, has \l{State}{States}
    to be compared and \l{Action}{Actions} to be executed.

    \sa EventDescriptor, State, Action
*/

//    Additionally a Rule is either of type \l{Rule::RuleTypeAll} or \l{Rule::RuleTypeAny}
//    which determines if all or any of the \l{State}{States} must be matching
//    in order for the \l{Action}{Actions} to be executed.


//! \enum Rule::RuleType

//    Note: There is no RuleTypeNone. If you don't want to compare any
//    states, construct a rule without states in which case it doesn't
//    matter what the Rule's type is.

//    \value RuleTypeAll
//    All States must match in order for the Rule to apply.
//    \value RuleTypeAny
//    Any State must match in order for the Rule to apply.


#include "rule.h"

#include <QDebug>

/*! Constructs an empty, invalid rule. */
Rule::Rule()
{

}

/*! Constructs a Rule with the given \a id, \a eventDescriptorList, \a stateEvaluator and \a actions.*/
Rule::Rule(const RuleId &id, const QList<EventDescriptor> &eventDescriptorList, const StateEvaluator &stateEvaluator, const QList<Action> &actions):
    m_id(id),
    m_eventDescriptors(eventDescriptorList),
    m_stateEvaluator(stateEvaluator),
    m_actions(actions)
{
}

/*! Returns the id or the Rule. */
RuleId Rule::id() const
{
    return m_id;
}

/*! Returns the \l{EventDescriptor} for this Rule.*/
QList<EventDescriptor> Rule::eventDescriptors() const
{
    return m_eventDescriptors;
}

/*! Returns the StateEvaluator that needs to evaluate successfully in order for this to Rule apply. */
StateEvaluator Rule::stateEvaluator() const
{
    return m_stateEvaluator;
}

/*! Returns the \l{Action}{Actions} to be executed when this Rule is matched and states match. */
QList<Action> Rule::actions() const
{
    return m_actions;
}

/*! Returns wheter the rule is enabled or not. */
bool Rule::enabled() const {
    return m_enabled;
}

/*! Set the \a enabled flag of this rule. In order to actually enable/disable the rule you still need to
 * update the RulesEngine */
void Rule::setEnabled(bool enabled)
{
    m_enabled = enabled;
}
