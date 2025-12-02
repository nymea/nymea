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

/*!
    \class nymeaserver::Rule
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

namespace nymeaserver {

/*! Constructs an empty, invalid \l{Rule}. */
Rule::Rule():
    m_id(RuleId()),
    m_name(QString()),
    m_timeDescriptor(TimeDescriptor()),
    m_stateEvaluator(StateEvaluator()),
    m_eventDescriptors(QList<EventDescriptor>()),
    m_actions(QList<RuleAction>()),
    m_exitActions(QList<RuleAction>()),
    m_enabled(true),
    m_active(false),
    m_statesActive(false),
    m_timeActive(false),
    m_executable(false)
{

}

/*! Returns the id of this \l{Rule}. */
RuleId Rule::id() const
{
    return m_id;
}

/*! Sets the \a ruleId of this \l{Rule}. */
void Rule::setId(const RuleId &ruleId)
{
    m_id = ruleId;
}

/*! Returns the name of this rule. */
QString Rule::name() const
{
    return m_name;
}

/*! Sets the \a name of this \l{Rule}. */
void Rule::setName(const QString &name)
{
    m_name = name;
}

/*! Returns true if the rule is active. */
bool Rule::active() const
{
    return m_active;
}

/*! Returns true if the rule is active regarding the StateEvaluator evaluation. */
bool Rule::statesActive() const
{
    return m_statesActive;
}

/*! Returns true if the rule is active regarding the TimeDescriptor evaluation. */
bool Rule::timeActive() const
{
    if (m_timeDescriptor.calendarItems().isEmpty())
        return true;

    return m_timeActive;
}

/*! Returns the \l{TimeDescriptor} or this Rule. */
TimeDescriptor Rule::timeDescriptor() const
{
    return m_timeDescriptor;
}

/*! Sets the \a timeDescriptor of this \l{Rule}. */
void Rule::setTimeDescriptor(const TimeDescriptor &timeDescriptor)
{
    m_timeDescriptor = timeDescriptor;
}

/*! Returns the StateEvaluator that needs to evaluate successfully in order for this to Rule apply. */
StateEvaluator Rule::stateEvaluator() const
{
    return m_stateEvaluator;
}

/*! Sets the \a stateEvaluator of this \l{Rule}. */
void Rule::setStateEvaluator(const StateEvaluator &stateEvaluator)
{
    m_stateEvaluator = stateEvaluator;
}

/*! Returns the \l{EventDescriptor} for this Rule.*/
EventDescriptors Rule::eventDescriptors() const
{
    return m_eventDescriptors;
}

/*! Sets the \a eventDescriptors of this \l{Rule}. */
void Rule::setEventDescriptors(const EventDescriptors &eventDescriptors)
{
    m_eventDescriptors = eventDescriptors;
}

/*! Returns the \l{RuleAction}{RuleActions} to be executed when this Rule is matched and states match. */
RuleActions Rule::actions() const
{
    return m_actions;
}

/*! Sets the \a actions of this \l{Rule}. */
void Rule::setActions(const RuleActions actions)
{
    m_actions = actions;
}

/*! Returns the \l{RuleAction}{RuleActions} to be executed when this Rule leaves the active state. */
RuleActions Rule::exitActions() const
{
    return m_exitActions;
}

/*! Sets the \a exitActions of this \l{Rule}. */
void Rule::setExitActions(const RuleActions exitActions)
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

/*! Returns true if this \l{Rule} is valid. A \l{Rule} with a valid \l{id()} is valid. */
bool Rule::isValid() const
{
    return !m_id.isNull();
}

/*! Returns true if this \l{Rule} is consistent. */
bool Rule::isConsistent() const
{
    // check if this rules is based on any event and contains exit actions
    if (!eventDescriptors().isEmpty() && stateEvaluator().isEmpty() && timeDescriptor().calendarItems().isEmpty() && !exitActions().isEmpty()) {
        return false;
    }

    // check if this rules is based on any time events and contains exit actions
    if (!timeDescriptor().timeEventItems().isEmpty() && stateEvaluator().isEmpty() && timeDescriptor().calendarItems().isEmpty() && !exitActions().isEmpty()) {
        return false;
    }

    // check if there are any actions
    if (actions().isEmpty()) {
        return false;
    }

    return true;
}

void Rule::setStatesActive(const bool &statesActive)
{
    m_statesActive = statesActive;
}

void Rule::setTimeActive(const bool &timeActive)
{
    m_timeActive = timeActive;
}

void Rule::setActive(const bool &active)
{
    m_active = active;
}

/*! Print a Rule with all its contents to QDebug. Note that this might print a lot of data.
 * It is useful to debug, but be cautionous with using this in production code.  */
QDebug operator<<(QDebug dbg, const Rule &rule)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << '\n' << "=== Rule begin ===" << '\n';
    dbg.nospace() << "ID:" << rule.id().toString() << '\n';
    dbg.nospace() << "Name:" << rule.name() << '\n';
    dbg.nospace() << "Enabled:" << rule.enabled() << '\n';
    dbg.nospace() << "Active:" << rule.active() << '\n';
    dbg.nospace() << rule.eventDescriptors();
    dbg.nospace() << rule.timeDescriptor();
    dbg.nospace() << rule.stateEvaluator();
    dbg.nospace() << "Actions:" << rule.actions();
    dbg.nospace() << "ExitActions:" << rule.exitActions();
    dbg.nospace() << "=== Rule end  ===";
    return dbg;
}

Rules::Rules()
{

}

Rules::Rules(const QList<Rule> &other): QList<Rule>(other)
{

}

QVariant Rules::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void Rules::put(const QVariant &variant)
{
    append(variant.value<Rule>());
}

}
