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

#ifndef RULE_H
#define RULE_H

#include "state.h"
#include "action.h"
#include "event.h"

#include <QUuid>

class Rule
{
public:
    enum RuleType {
        RuleTypeAll,
        RuleTypeAny
    };

    Rule(const QUuid &id, const Event &event, const QList<State> &states, const QList<Action> &actions);

    QUuid id() const;
    Event event() const;
    QList<State> states() const;
    QList<Action> actions() const;

    RuleType ruleType() const;
    void setRuleType(RuleType ruleType);

private:
    QUuid m_id;
    Event m_event;
    QList<State> m_states;
    QList<Action> m_actions;
    RuleType m_ruleType;
};

#endif // RULE_H
