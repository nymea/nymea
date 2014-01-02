#ifndef RULE_H
#define RULE_H

#include "action.h"

#include <QUuid>

class Rule
{
public:
    Rule(const QUuid &id, const QUuid &triggerTypeId, const Action &action);

    QUuid id() const;
    QUuid triggerTypeId() const;
    Action action() const;

private:
    QUuid m_id;
    QUuid m_triggerTypeId;
    Action m_action;
};

#endif // RULE_H
