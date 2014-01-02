#ifndef RULE_H
#define RULE_H

#include "action.h"
#include "trigger.h"

#include <QUuid>

class Rule
{
public:
    Rule(const QUuid &id, const Trigger &trigger, const Action &action);

    QUuid id() const;
    Trigger trigger() const;
    Action action() const;

private:
    QUuid m_id;
    Trigger m_trigger;
    Action m_action;
};

#endif // RULE_H
