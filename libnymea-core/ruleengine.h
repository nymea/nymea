/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef RULEENGINE_H
#define RULEENGINE_H

#include "rule.h"
#include "types/event.h"
#include "types/deviceclass.h"
#include "stateevaluator.h"

#include <QObject>
#include <QList>
#include <QUuid>

namespace nymeaserver {

class RuleEngine : public QObject
{
    Q_OBJECT
    Q_ENUMS(RuleError)
    Q_ENUMS(RemovePolicy)
public:
    enum RuleError {
        RuleErrorNoError,
        RuleErrorInvalidRuleId,
        RuleErrorRuleNotFound,
        RuleErrorDeviceNotFound,
        RuleErrorEventTypeNotFound,
        RuleErrorStateTypeNotFound,
        RuleErrorActionTypeNotFound,
        RuleErrorInvalidParameter,
        RuleErrorInvalidRuleFormat,
        RuleErrorMissingParameter,
        RuleErrorInvalidRuleActionParameter,
        RuleErrorInvalidStateEvaluatorValue,
        RuleErrorTypesNotMatching,
        RuleErrorNotExecutable,
        RuleErrorInvalidTimeDescriptor,
        RuleErrorInvalidRepeatingOption,
        RuleErrorInvalidCalendarItem,
        RuleErrorInvalidTimeEventItem,
        RuleErrorContainsEventBasesAction,
        RuleErrorNoExitActions,
        RuleErrorInterfaceNotFound
    };

    enum RemovePolicy {
        RemovePolicyCascade,
        RemovePolicyUpdate
    };

    explicit RuleEngine(QObject *parent = nullptr);
    ~RuleEngine();
    void init();

    QList<Rule> evaluateEvent(const Event &event);
    QList<Rule> evaluateTime(const QDateTime &dateTime);

    RuleError addRule(const Rule &rule, bool fromEdit = false);
    RuleError editRule(const Rule &rule);

    QList<Rule> rules() const;
    QList<RuleId> ruleIds() const;

    RuleError removeRule(const RuleId &ruleId, bool fromEdit = false);

    RuleError enableRule(const RuleId &ruleId);
    RuleError disableRule(const RuleId &ruleId);

    RuleError executeActions(const RuleId &ruleId);
    RuleError executeExitActions(const RuleId &ruleId);

    Rule findRule(const RuleId &ruleId);
    QList<RuleId> findRules(const DeviceId &deviceId) const;
    QList<DeviceId> devicesInRules() const;

    void removeDeviceFromRule(const RuleId &id, const DeviceId &deviceId);

signals:
    void ruleAdded(const Rule &rule);
    void ruleRemoved(const RuleId &ruleId);
    void ruleConfigurationChanged(const Rule &rule);

private:
    bool containsEvent(const Rule &rule, const Event &event, const DeviceClassId &deviceClassId);
    bool containsState(const StateEvaluator &stateEvaluator, const Event &stateChangeEvent);

    RuleError checkRuleAction(const RuleAction &ruleAction, const Rule &rule);
    RuleError checkRuleActionParam(const RuleActionParam &ruleActionParam, const ActionType &actionType, const Rule &rule);

    QVariant::Type getActionParamType(const ActionTypeId &actionTypeId, const ParamTypeId &paramTypeId);
    QVariant::Type getEventParamType(const EventTypeId &eventTypeId, const ParamTypeId &paramTypeId);

    void appendRule(const Rule &rule);
    void saveRule(const Rule &rule);

private:
    QList<RuleId> m_ruleIds; // Keeping a list of RuleIds to keep sorting order...
    QHash<RuleId, Rule> m_rules; // ...but use a Hash for faster finding
    QList<RuleId> m_activeRules;

    QDateTime m_lastEvaluationTime;
};

}

Q_DECLARE_METATYPE(nymeaserver::RuleEngine::RuleError)


#endif // RULEENGINE_H
