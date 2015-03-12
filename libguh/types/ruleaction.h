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

#ifndef RULEACTION_H
#define RULEACTION_H

#include "action.h"
#include "ruleactionparam.h"

class RuleAction
{
public:
    explicit RuleAction(const ActionTypeId &actionTypeId = ActionTypeId(), const DeviceId &deviceId = DeviceId());
    RuleAction(const RuleAction &other);

    ActionId id() const;
    bool isValid() const;

    bool isEventBased() const;

    Action toAction() const;

    ActionTypeId actionTypeId() const;
    DeviceId deviceId() const;

    RuleActionParamList ruleActionParams() const;
    void setRuleActionParams(const RuleActionParamList &ruleActionParams);
    RuleActionParam ruleActionParam(const QString &ruleActionParamName) const;

    void operator=(const RuleAction &other);

private:
    ActionId m_id;
    ActionTypeId m_actionTypeId;
    DeviceId m_deviceId;
    RuleActionParamList m_ruleActionParams;
};

#endif // RULEACTION_H
