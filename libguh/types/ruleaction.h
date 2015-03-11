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
