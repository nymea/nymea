/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef RULEENGINE_H
#define RULEENGINE_H

#include "rule.h"
#include "stateevaluator.h"
#include "types/event.h"
#include "types/deviceclass.h"

#include <QObject>
#include <QList>
#include <QUuid>
#include <QSettings>

namespace nymeaserver {

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
    Q_ENUM(RuleError)

    enum RemovePolicy {
        RemovePolicyCascade,
        RemovePolicyUpdate
    };
    Q_ENUM(RemovePolicy)

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
    void saveRuleActions(NymeaSettings *settings, const QList<RuleAction> &ruleActions);
    QList<RuleAction> loadRuleActions(NymeaSettings *settings);

private:
    QList<RuleId> m_ruleIds; // Keeping a list of RuleIds to keep sorting order...
    QHash<RuleId, Rule> m_rules; // ...but use a Hash for faster finding
    QList<RuleId> m_activeRules;

    QDateTime m_lastEvaluationTime;
};

}

Q_DECLARE_METATYPE(nymeaserver::RuleEngine::RuleError)


#endif // RULEENGINE_H
