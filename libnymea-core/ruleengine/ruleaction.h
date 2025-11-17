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

#ifndef RULEACTION_H
#define RULEACTION_H

#include "libnymea.h"
#include "types/action.h"
#include "types/browseritemaction.h"
#include "ruleactionparam.h"

class LIBNYMEA_EXPORT RuleAction
{
    Q_GADGET
    Q_PROPERTY(QUuid thingId READ thingId WRITE setThingId USER true)
    Q_PROPERTY(QUuid actionTypeId READ actionTypeId WRITE setActionTypeId USER true)
    Q_PROPERTY(QString interface READ interface WRITE setInterface USER true)
    Q_PROPERTY(QString interfaceAction READ interfaceAction WRITE setInterfaceAction USER true)
    Q_PROPERTY(QString browserItemId READ browserItemId WRITE setBrowserItemId USER true)
    Q_PROPERTY(RuleActionParams ruleActionParams READ ruleActionParams WRITE setRuleActionParams USER true)

public:
    enum Type {
        TypeThing,
        TypeInterface,
        TypeBrowser
    };
    explicit RuleAction(const ActionTypeId &actionTypeId = ActionTypeId(), const ThingId &thingId = ThingId(), const RuleActionParams &params = RuleActionParams());
    explicit RuleAction(const QString &interface, const QString &interfaceAction, const RuleActionParams &params = RuleActionParams());
    explicit RuleAction(const ThingId &thingId, const QString &browserItemId);
    RuleAction(const RuleAction &other);

    bool isValid() const;

    Type type() const;

    bool isEventBased() const;
    bool isStateBased() const;

    Action toAction() const;
    BrowserItemAction toBrowserItemAction() const;

    ThingId thingId() const;
    void setThingId(const ThingId &thingId);

    ActionTypeId actionTypeId() const;
    void setActionTypeId(const ActionTypeId &actionTypeId);

    QString browserItemId() const;
    void setBrowserItemId(const QString &browserItemId);

    QString interface() const;
    void setInterface(const QString &interface);

    QString interfaceAction() const;
    void setInterfaceAction(const QString &interfaceAction);

    RuleActionParams ruleActionParams() const;
    void setRuleActionParams(const RuleActionParams &ruleActionParams);
    RuleActionParam ruleActionParam(const ParamTypeId &ruleActionParamTypeId) const;
    RuleActionParam ruleActionParam(const QString &ruleActionParamName) const;

    void operator=(const RuleAction &other);

private:
    ThingId m_thingId;
    ActionTypeId m_actionTypeId;
    QString m_browserItemId;
    QString m_interface;
    QString m_interfaceAction;
    RuleActionParams m_ruleActionParams;
};
Q_DECLARE_METATYPE(RuleAction)

class RuleActions: public QList<RuleAction>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    RuleActions();
    RuleActions(const QList<RuleAction> &other);
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
Q_DECLARE_METATYPE(RuleActions)

QDebug operator<<(QDebug dbg, const RuleAction &ruleAction);
QDebug operator<<(QDebug dbg, const QList<RuleAction> &ruleActionList);

#endif // RULEACTION_H
