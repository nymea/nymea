#ifndef TRIGGER_H
#define TRIGGER_H

#include <QString>
#include <QUuid>
#include <QVariantList>

class Trigger
{
public:
    Trigger(const QUuid &deviceClassid, const QVariantMap &params);

    QUuid deviceClassId() const;

    QVariantMap params() const;
    void setParams(const QVariantMap &params);

private:
    QUuid m_deviceClassId;
    QVariantMap m_params;
};

#endif // TRIGGER_H
