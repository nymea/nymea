#ifndef STATE_H
#define STATE_H

#include <QUuid>
#include <QVariant>

class State
{
public:
    State(const QUuid &stateTypeId);

    QUuid stateTypeId() const;

    QVariant value() const;
    void setValue(const QVariant &value);

private:
    QUuid m_stateTypeId;
    QVariant m_value;
};

#endif // STATE_H
