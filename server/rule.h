#ifndef RULE_H
#define RULE_H

#include <QUuid>

class Rule
{
public:
    Rule(const QUuid &id, const QUuid &triggerId, const QUuid &actionId);

    QUuid id() const;
    QUuid triggerId() const;
    QUuid actionId() const;

private:
    QUuid m_id;
    QUuid m_triggerId;
    QUuid m_actionId;
};

#endif // RULE_H
