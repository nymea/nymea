/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef RULEACTION_H
#define RULEACTION_H

#include "libnymea.h"
#include "types/action.h"
#include "types/browseritemaction.h"
#include "ruleactionparam.h"

class LIBNYMEA_EXPORT RuleAction
{
    Q_GADGET
    Q_PROPERTY(QUuid deviceId READ deviceId WRITE setDeviceId USER true)
    Q_PROPERTY(QUuid actionTypeId READ actionTypeId WRITE setActionTypeId USER true)
    Q_PROPERTY(QString interface READ interface WRITE setInterface USER true)
    Q_PROPERTY(QString interfaceAction READ interfaceAction WRITE setInterfaceAction USER true)
    Q_PROPERTY(QString browserItemId READ browserItemId WRITE setBrowserItemId USER true)
    Q_PROPERTY(RuleActionParams ruleActionParams READ ruleActionParams WRITE setRuleActionParams USER true)

public:
    enum Type {
        TypeDevice,
        TypeInterface,
        TypeBrowser
    };
    explicit RuleAction(const ActionTypeId &actionTypeId = ActionTypeId(), const DeviceId &deviceId = DeviceId(), const RuleActionParams &params = RuleActionParams());
    explicit RuleAction(const QString &interface, const QString &interfaceAction, const RuleActionParams &params = RuleActionParams());
    explicit RuleAction(const DeviceId &deviceId, const QString &browserItemId);
    RuleAction(const RuleAction &other);

    ActionId id() const;
    bool isValid() const;

    Type type() const;

    bool isEventBased() const;
    bool isStateBased() const;

    Action toAction() const;
    BrowserItemAction toBrowserItemAction() const;

    DeviceId deviceId() const;
    void setDeviceId(const DeviceId &deviceId);

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
    ActionId m_id;
    DeviceId m_deviceId;
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
