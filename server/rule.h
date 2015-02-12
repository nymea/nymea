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

#ifndef RULE_H
#define RULE_H

#include "types/state.h"
#include "types/action.h"
#include "types/eventdescriptor.h"
#include "stateevaluator.h"

#include <QUuid>

class Rule
{
public:
    Rule();
    Rule(const RuleId &id, const QList<EventDescriptor> &eventDescriptorList, const StateEvaluator &stateEvaluator, const QList<Action> &actions);

    RuleId id() const;
    QList<EventDescriptor> eventDescriptors() const;
    StateEvaluator stateEvaluator() const;
    QList<Action> actions() const;

    bool enabled() const;
    void setEnabled(bool enabled);

    bool active() const;

private:
    friend class RuleEngine;
    void setActive(bool active);

private:
    RuleId m_id;
    QList<EventDescriptor> m_eventDescriptors;
    StateEvaluator m_stateEvaluator;
    QList<Action> m_actions;

    bool m_enabled;
    bool m_active;
};

#endif // RULE_H
