#ifndef TRIGGER_H
#define TRIGGER_H

#include <QString>
#include <QUuid>
#include <QVariantList>

class Trigger
{
public:
    Trigger(const QUuid &triggerTypeId, const QUuid &deviceId, const QVariantMap &params);

    QUuid triggerTypeId() const;
    QUuid deviceId() const;

    QVariantMap params() const;
    void setParams(const QVariantMap &params);

    bool operator ==(const Trigger &other) const;

private:
    QUuid m_triggerTypeId;
    QUuid m_deviceId;
    QVariantMap m_params;
};

#endif // TRIGGER_H
