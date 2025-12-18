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

#ifndef RULESHANDLER_H
#define RULESHANDLER_H

#include "jsonrpc/jsonhandler.h"

#include "ruleengine/rule.h"

namespace nymeaserver {

class RuleEngine;

class RulesHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit RulesHandler(RuleEngine *ruleEngine, QObject *parent = nullptr);

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
    QVariantMap packRuleDescription(const Rule &rule);

private:
    RuleEngine *m_ruleEngine = nullptr;
};

} // namespace nymeaserver

#endif // RULESHANDLER_H
