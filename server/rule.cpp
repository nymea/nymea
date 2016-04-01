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

    \ingroup rules
    \inmodule core

    A Rule is always triggered by an \l{EventDescriptor}, has \l{State}{States}
    to be compared and \l{RuleAction}{RuleActions} to be executed.

    \sa EventDescriptor, State, RuleAction
*/

#include "rule.h"
#include "loggingcategories.h"

#include <QDebug>

namespace guhserver {

/*! Constructs an empty, invalid rule. */
Rule::Rule():
    m_id(RuleId()),
    m_name(QString()),
    m_timeDescriptor(TimeDescriptor()),
    m_stateEvaluator(StateEvaluator()),
    m_eventDescriptors(QList<EventDescriptor>()),
    m_actions(QList<RuleAction>()),
    m_exitActions(QList<RuleAction>()),
    m_enabled(false),
    m_active(false),
    m_executable(false)
{

}

/*! Returns the id of this Rule. */
RuleId Rule::id() const
{
    return m_id;
}

void Rule::setId(const RuleId &ruleId)
{
    m_id = ruleId;
}

/*! Returns the name of this rule. */
QString Rule::name() const
{
    return m_name;
}

void Rule::setName(const QString &name)
{
    m_name = name;
}

/*! Returns true if the rule is active. */
bool Rule::active() const
{
    return m_active;
}

/*! Returns the \l{TimeDescriptor} or this Rule. */
TimeDescriptor Rule::timeDescriptor() const
{
    return m_timeDescriptor;
}

void Rule::setTimeDescriptor(const TimeDescriptor &timeDescriptor)
{
    m_timeDescriptor = timeDescriptor;
}

/*! Returns the StateEvaluator that needs to evaluate successfully in order for this to Rule apply. */
StateEvaluator Rule::stateEvaluator() const
{
    return m_stateEvaluator;
}

void Rule::setStateEvaluator(const StateEvaluator &stateEvaluator)
{
    m_stateEvaluator = stateEvaluator;
}

/*! Returns the \l{EventDescriptor} for this Rule.*/
QList<EventDescriptor> Rule::eventDescriptors() const
{
    return m_eventDescriptors;
}

void Rule::setEventDescriptors(const QList<EventDescriptor> &eventDescriptors)
{
    m_eventDescriptors = eventDescriptors;
}

/*! Returns the \l{RuleAction}{RuleActions} to be executed when this Rule is matched and states match. */
QList<RuleAction> Rule::actions() const
{
    return m_actions;
}

void Rule::setActions(const QList<RuleAction> actions)
{
    m_actions = actions;
}

/*! Returns the \l{RuleAction}{RuleActions} to be executed when this Rule leaves the active state. */
QList<RuleAction> Rule::exitActions() const
{
    return m_exitActions;
}

void Rule::setExitActions(const QList<RuleAction> exitActions)
{
    m_exitActions = exitActions;
}

/*! Returns true if the rule is enabled. */
bool Rule::enabled() const {
    return m_enabled;
}

/*! Set the \a enabled flag of this rule. In order to actually enable/disable the rule you still need to
 * update the \l{RuleEngine} */
void Rule::setEnabled(const bool &enabled)
{
    m_enabled = enabled;
}

/*! Returns true if the rule is executable. */
bool Rule::executable() const
{
    return m_executable;
}

/*! Set the rule \a executable. */
void Rule::setExecutable(const bool &executable)
{
    m_executable = executable;
}

bool Rule::isValid() const
{
    return !m_id.isNull();
}

bool Rule::isConsistent() const
{
    // check if this rules is based on any event and contains exit actions
    if (!eventDescriptors().isEmpty() && !exitActions().isEmpty()) {
        qCWarning(dcRuleEngine) << "Rule not consistent. The exitActions will never be executed if the rule contains an eventDescriptor.";
        return false;
    }

    // check if there are any actions
    if (actions().isEmpty()) {
        qCWarning(dcRuleEngine) << "Rule not consistent. A rule without actions has no effect.";
        return false;
    }

    return true;
}


void Rule::setActive(const bool &active)
{
    m_active = active;
}

}
