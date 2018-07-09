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
#include "action.h"
#include "ruleactionparam.h"

class LIBNYMEA_EXPORT RuleAction
{
public:
    enum Type {
        TypeDevice,
        TypeInterface
    };
    explicit RuleAction(const ActionTypeId &actionTypeId = ActionTypeId(), const DeviceId &deviceId = DeviceId(), const RuleActionParamList &params = RuleActionParamList());
    explicit RuleAction(const QString &interface, const QString &interfaceAction, const RuleActionParamList &params = RuleActionParamList());
    RuleAction(const RuleAction &other);

    ActionId id() const;
    bool isValid() const;

    Type type() const;

    bool isEventBased() const;

    Action toAction() const;

    ActionTypeId actionTypeId() const;
    DeviceId deviceId() const;

    QString interface() const;
    QString interfaceAction() const;

    RuleActionParamList ruleActionParams() const;
    void setRuleActionParams(const RuleActionParamList &ruleActionParams);
    RuleActionParam ruleActionParam(const ParamTypeId &ruleActionParamTypeId) const;
    RuleActionParam ruleActionParam(const QString &ruleActionParamName) const;

    void operator=(const RuleAction &other);

private:
    ActionId m_id;
    ActionTypeId m_actionTypeId;
    DeviceId m_deviceId;
    QString m_interface;
    QString m_interfaceAction;
    RuleActionParamList m_ruleActionParams;
};

QDebug operator<<(QDebug dbg, const RuleAction &ruleAction);
QDebug operator<<(QDebug dbg, const QList<RuleAction> &ruleActionList);

#endif // RULEACTION_H
