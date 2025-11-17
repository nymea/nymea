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

#ifndef RULE_H
#define RULE_H

#include "types/state.h"
#include "types/eventdescriptor.h"
#include "time/timedescriptor.h"
#include "ruleaction.h"
#include "stateevaluator.h"

#include <QUuid>

namespace nymeaserver {

class Rule
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(bool active READ active)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled USER true)
    Q_PROPERTY(bool executable READ executable WRITE setExecutable USER true)
    Q_PROPERTY(EventDescriptors eventDescriptors READ eventDescriptors WRITE setEventDescriptors USER true)
    Q_PROPERTY(RuleActions actions READ actions WRITE setActions)
    Q_PROPERTY(RuleActions exitActions READ exitActions WRITE setExitActions USER true)
    Q_PROPERTY(nymeaserver::StateEvaluator stateEvaluator READ stateEvaluator WRITE setStateEvaluator USER true)
    Q_PROPERTY(TimeDescriptor timeDescriptor READ timeDescriptor WRITE setTimeDescriptor USER true)

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

    EventDescriptors eventDescriptors() const;
    void setEventDescriptors(const EventDescriptors &eventDescriptors);

    RuleActions actions() const;
    void setActions(const RuleActions actions);

    RuleActions exitActions() const;
    void setExitActions(const RuleActions exitActions);

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
    EventDescriptors m_eventDescriptors;
    RuleActions m_actions;
    RuleActions m_exitActions;

    bool m_enabled;
    bool m_active;
    bool m_statesActive;
    bool m_timeActive;
    bool m_executable;
};

class Rules: QList<Rule>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    Rules();
    Rules(const QList<Rule> &other);
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};

QDebug operator<<(QDebug dbg, const Rule &rule);

}
Q_DECLARE_METATYPE(nymeaserver::Rules)

#endif // RULE_H
