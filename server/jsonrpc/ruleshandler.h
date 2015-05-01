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

#ifndef RULESHANDLER_H
#define RULESHANDLER_H

#include "jsonhandler.h"

class RulesHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit RulesHandler(QObject *parent = 0);

    QString name() const override;

    Q_INVOKABLE JsonReply* GetRules(const QVariantMap &params);
    Q_INVOKABLE JsonReply* GetRuleDetails(const QVariantMap &params);

    Q_INVOKABLE JsonReply* AddRule(const QVariantMap &params);
    Q_INVOKABLE JsonReply* RemoveRule(const QVariantMap &params);
    Q_INVOKABLE JsonReply* FindRules(const QVariantMap &params);

    Q_INVOKABLE JsonReply* EnableRule(const QVariantMap &params);
    Q_INVOKABLE JsonReply* DisableRule(const QVariantMap &params);

signals:
    void RuleRemoved(const QVariantMap &params);
    void RuleAdded(const QVariantMap &params);

private:
    QVariant::Type getActionParamType(const ActionTypeId &actionTypeId, const QString &paramName);
    QVariant::Type getEventParamType(const EventTypeId &eventTypeId, const QString &paramName);

    bool checkEventDescriptors(const QList<EventDescriptor> eventDescriptors, const EventTypeId &eventTypeId);

private slots:
    void ruleRemovedNotification(const RuleId &ruleId);
    void ruleAddedNotification(const Rule &rule);

};

#endif // RULESHANDLER_H
