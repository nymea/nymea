#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include <QUuid>
#include <QVariantList>

class Event
{
public:
    Event(const QUuid &eventTypeId, const QUuid &deviceId, const QVariantMap &params);

    QUuid eventTypeId() const;
    QUuid deviceId() const;

    QVariantMap params() const;
    void setParams(const QVariantMap &params);

    bool operator ==(const Event &other) const;

private:
    QUuid m_eventTypeId;
    QUuid m_deviceId;
    QVariantMap m_params;
};

#endif // EVENT_H
