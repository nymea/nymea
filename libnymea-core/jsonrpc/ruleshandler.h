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

}

#endif // RULESHANDLER_H
