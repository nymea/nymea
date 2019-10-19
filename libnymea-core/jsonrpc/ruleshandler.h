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

#ifndef RULESHANDLER_H
#define RULESHANDLER_H

#include "jsonrpc/jsonhandler.h"

#include "ruleengine/rule.h"

namespace nymeaserver {

class RulesHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit RulesHandler(QObject *parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply *GetRules(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetRuleDetails(const QVariantMap &params);

    Q_INVOKABLE JsonReply *AddRule(const QVariantMap &params);
    Q_INVOKABLE JsonReply *EditRule(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveRule(const QVariantMap &params);
    Q_INVOKABLE JsonReply *FindRules(const QVariantMap &params);

    Q_INVOKABLE JsonReply *EnableRule(const QVariantMap &params);
    Q_INVOKABLE JsonReply *DisableRule(const QVariantMap &params);

    Q_INVOKABLE JsonReply *ExecuteActions(const QVariantMap &params);
    Q_INVOKABLE JsonReply *ExecuteExitActions(const QVariantMap &params);

signals:
    void RuleRemoved(const QVariantMap &params);
    void RuleAdded(const QVariantMap &params);
    void RuleActiveChanged(const QVariantMap &params);
    void RuleConfigurationChanged(const QVariantMap &params);

private slots:
    void ruleRemovedNotification(const RuleId &ruleId);
    void ruleAddedNotification(const Rule &rule);
    void ruleActiveChangedNotification(const Rule &rule);
    void ruleConfigurationChangedNotification(const Rule &rule);

private:
    static QVariantMap packRuleDescription(const Rule &rule);
    static QVariantMap packParamDescriptor(const ParamDescriptor &paramDescriptor);
    static QVariantMap packEventDescriptor(const EventDescriptor &eventDescriptor);
    static QVariantMap packStateEvaluator(const StateEvaluator &stateEvaluator);
    static QVariantMap packStateDescriptor(const StateDescriptor &stateDescriptor);
    static QVariantMap packTimeDescriptor(const TimeDescriptor &timeDescriptor);
    static QVariantMap packCalendarItem(const CalendarItem &calendarItem);
    static QVariantMap packRepeatingOption(const RepeatingOption &option);
    static QVariantMap packTimeEventItem(const TimeEventItem &timeEventItem);
    static QVariantMap packRuleActionParam(const RuleActionParam &ruleActionParam);
    static QVariantMap packRuleAction(const RuleAction &ruleAction);

    static QVariantMap packRule(const Rule &rule);

    static QList<ParamDescriptor> unpackParamDescriptors(const QVariantList &paramList);
    static ParamDescriptor unpackParamDescriptor(const QVariantMap &paramMap);
    static EventDescriptor unpackEventDescriptor(const QVariantMap &eventDescriptorMap);
    static RepeatingOption unpackRepeatingOption(const QVariantMap &repeatingOptionMap);
    static CalendarItem unpackCalendarItem(const QVariantMap &calendarItemMap);
    static TimeDescriptor unpackTimeDescriptor(const QVariantMap &timeDescriptorMap);
    static TimeEventItem unpackTimeEventItem(const QVariantMap &timeEventItemMap);
    static StateDescriptor unpackStateDescriptor(const QVariantMap &stateDescriptorMap);
    static StateEvaluator unpackStateEvaluator(const QVariantMap &stateEvaluatorMap);
    static RuleActionParam unpackRuleActionParam(const QVariantMap &ruleActionParamMap);
    static RuleActionParamList unpackRuleActionParams(const QVariantList &ruleActionParamList);
    static RuleAction unpackRuleAction(const QVariantMap &ruleActionMap);

    static Rule unpackRule(const QVariantMap &ruleMap);


};

}

#endif // RULESHANDLER_H
