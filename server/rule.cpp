/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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
    \class guhserver::Rule
    \brief This class represents a rule.

    \ingroup core
    \inmodule server

    A Rule is always triggered by an \l{EventDescriptor}, has \l{State}{States}
    to be compared and \l{RuleAction}{RuleActions} to be executed.

    \sa EventDescriptor, State, RuleAction
*/

#include "rule.h"

#include <QDebug>

namespace guhserver {

/*! Constructs an empty, invalid rule. */
Rule::Rule():
    Rule(RuleId(), QString(), QList<EventDescriptor>(), StateEvaluator(), QList<RuleAction>(), QList<RuleAction>())
{
}

/*! Constructs a Rule with the given \a id, \a name, \a eventDescriptorList, \a stateEvaluator and \a actions.*/
Rule::Rule(const RuleId &id, const QString &name, const QList<EventDescriptor> &eventDescriptorList, const StateEvaluator &stateEvaluator, const QList<RuleAction> &actions) :
    m_id(id),
    m_name(name),
    m_eventDescriptors(eventDescriptorList),
    m_stateEvaluator(stateEvaluator),
    m_actions(actions),
    m_enabled(false),
    m_active(false)
{

}

/*! Constructs a Rule with the given \a id, \a name, \a eventDescriptorList, \a stateEvaluator, \a actions and \a exitActions.*/
Rule::Rule(const RuleId &id, const QString &name, const QList<EventDescriptor> &eventDescriptorList, const StateEvaluator &stateEvaluator, const QList<RuleAction> &actions, const QList<RuleAction> &exitActions):
    m_id(id),
    m_name(name),
    m_eventDescriptors(eventDescriptorList),
    m_stateEvaluator(stateEvaluator),
    m_actions(actions),
    m_exitActions(exitActions),
    m_enabled(false),
    m_active(false)
{
}

/*! Constructs a Rule with the given \a id, \a name, \a stateEvaluator, \a actions and \a exitActions. This type of rule
 *  works only state based and executes the \a actions once the rule enters the active state and executes the \a exitActions
 *  once the rule exits the active state.*/
Rule::Rule(const RuleId &id, const QString &name, const StateEvaluator &stateEvaluator, const QList<RuleAction> &actions, const QList<RuleAction> &exitActions) :
    m_id(id),
    m_name(name),
    m_stateEvaluator(stateEvaluator),
    m_actions(actions),
    m_exitActions(exitActions),
    m_enabled(false),
    m_active(false)
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

/*! Returns the \l{RuleAction}{RuleActions} to be executed when this Rule is matched and states match. */
QList<RuleAction> Rule::actions() const
{
    return m_actions;
}

/*! Returns the \l{RuleAction}{RuleActions} to be executed when this Rule leaves the active state. */
QList<RuleAction> Rule::exitActions() const
{
    return m_exitActions;
}

/*! Returns the name of this rule. */
QString Rule::name() const
{
    return m_name;
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

/*! Returns true if the rule is active. */
bool Rule::active() const
{
    return m_active;
}

void Rule::setName(const QString &name)
{
    m_name = name;
}

void Rule::setActive(bool active)
{
    m_active = active;
}

}
