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

#ifndef RULEENGINE_H
#define RULEENGINE_H

#include "rule.h"
#include "types/event.h"
#include "stateevaluator.h"

#include <QObject>
#include <QList>
#include <QUuid>

class RuleEngine : public QObject
{
    Q_OBJECT
public:
    enum RuleError {
        RuleErrorNoError,
        RuleErrorInvalidRuleId,
        RuleErrorRuleNotFound,
        RuleErrorDeviceNotFound,
        RuleErrorEventTypeNotFound,
        RuleErrorActionTypeNotFound
    };

    explicit RuleEngine(QObject *parent = 0);

    QList<Action> evaluateEvent(const Event &event);

    RuleError addRule(const RuleId &ruleId, const EventDescriptor &eventDescriptor, const QList<Action> &actions);
    RuleError addRule(const RuleId &ruleId, const QList<EventDescriptor> &eventDescriptorList, const StateEvaluator &stateEvaluator, const QList<Action> &actions);
    QList<Rule> rules() const;

    RuleError removeRule(const RuleId &ruleId);

    Rule findRule(const RuleId &ruleId);

signals:
    void ruleAdded(const RuleId &ruleId);
    void ruleRemoved(const RuleId &ruleId);

private:
    bool containsEvent(const Rule &rule, const Event &event);

private:
    QString m_settingsFile;
    QList<Rule> m_rules;

};

#endif // RULEENGINE_H
