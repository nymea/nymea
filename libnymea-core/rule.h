/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef RULE_H
#define RULE_H

#include "types/state.h"
#include "types/ruleaction.h"
#include "types/eventdescriptor.h"
#include "stateevaluator.h"
#include "time/timedescriptor.h"

#include <QUuid>

namespace guhserver {

class Rule
{
public:
    Rule();

    RuleId id() const;
    void setId(const RuleId &ruleId);

    QString name() const;
    void setName(const QString &name);

    bool active() const;
    bool statesActive() const;
    bool timeActive() const;

    TimeDescriptor timeDescriptor() const;
    void setTimeDescriptor(const TimeDescriptor &timeDescriptor);

    StateEvaluator stateEvaluator() const;
    void setStateEvaluator(const StateEvaluator &stateEvaluator);

    QList<EventDescriptor> eventDescriptors() const;
    void setEventDescriptors(const QList<EventDescriptor> &eventDescriptors);

    QList<RuleAction> actions() const;
    void setActions(const QList<RuleAction> actions);

    QList<RuleAction> exitActions() const;
    void setExitActions(const QList<RuleAction> exitActions);

    bool enabled() const;
    void setEnabled(const bool &enabled);

    bool executable() const;
    void setExecutable(const bool &executable);

    // verification methods
    bool isValid() const;
    bool isConsistent() const;

private:
    friend class RuleEngine;
    void setStatesActive(const bool &statesActive);
    void setTimeActive(const bool &timeActive);
    void setActive(const bool &active);

private:
    RuleId m_id;
    QString m_name;
    TimeDescriptor m_timeDescriptor;
    StateEvaluator m_stateEvaluator;
    QList<EventDescriptor> m_eventDescriptors;
    QList<RuleAction> m_actions;
    QList<RuleAction> m_exitActions;

    bool m_enabled;
    bool m_active;
    bool m_statesActive;
    bool m_timeActive;
    bool m_executable;
};

}

#endif // RULE_H
