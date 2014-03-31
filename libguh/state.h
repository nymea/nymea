#ifndef STATE_H
#define STATE_H

#include <QUuid>
#include <QVariant>

class State
{
public:
    State(const QUuid &stateTypeId, const QUuid &deviceId);

    QUuid stateTypeId() const;
    QUuid deviceId() const;

    QVariant value() const;
    void setValue(const QVariant &value);

private:
    QUuid m_stateTypeId;
    QUuid m_deviceId;
    QVariant m_value;
};

#endif // STATE_H
