#include "ruleaction.h"

RuleAction::RuleAction(const ActionTypeId &actionTypeId, const DeviceId &deviceId) :
    m_id(ActionId::createActionId()),
    m_actionTypeId(actionTypeId),
    m_deviceId(deviceId)
{

}

RuleAction::RuleAction(const RuleAction &other) :
    m_id(other.id()),
    m_actionTypeId(other.actionTypeId()),
    m_deviceId(other.deviceId()),
    m_ruleActionParams(other.ruleActionParams())
{

}

ActionId RuleAction::id() const
{
    return m_id;
}

bool RuleAction::isValid() const
{
    return !m_actionTypeId.isNull() && !m_deviceId.isNull();
}

bool RuleAction::isEventBased() const
{
    foreach (const RuleActionParam &param, m_ruleActionParams) {
        if (param.eventTypeId() != EventTypeId()) {
            return true;
        }
    }
    return false;
}

Action RuleAction::toAction() const
{
    Action action(m_actionTypeId, m_deviceId);
    ParamList params;
    foreach (const RuleActionParam &ruleActionParam, m_ruleActionParams) {
        Param param;
        param.setName(ruleActionParam.name());
        param.setValue(ruleActionParam.value());
        params.append(param);
    }
    action.setParams(params);
    return action;
}

ActionTypeId RuleAction::actionTypeId() const
{
    return m_actionTypeId;
}

DeviceId RuleAction::deviceId() const
{
    return m_deviceId;
}

RuleActionParamList RuleAction::ruleActionParams() const
{
    return m_ruleActionParams;
}

void RuleAction::setRuleActionParams(const RuleActionParamList &ruleActionParams)
{
    m_ruleActionParams = ruleActionParams;
}

RuleActionParam RuleAction::ruleActionParam(const QString &ruleActionParamName) const
{
    foreach (const RuleActionParam &ruleActionParam, m_ruleActionParams) {
        if (ruleActionParam.name() == ruleActionParamName) {
            return ruleActionParam;
        }
    }
    return RuleActionParam(QString());
}

void RuleAction::operator=(const RuleAction &other)
{
    m_id = other.id();
    m_actionTypeId = other.actionTypeId();
    m_ruleActionParams = other.ruleActionParams();
}
