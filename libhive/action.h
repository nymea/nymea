#ifndef ACTION_H
#define ACTION_H

#include <QUuid>
#include <QVariantList>

class Action
{
public:
    explicit Action(const QUuid &deviceId, const QUuid &actionTypeId);

    bool isValid() const;

    QUuid actionTypeId() const;
    QUuid deviceId() const;

    QVariantMap params() const;
    void setParams(const QVariantMap &params);

private:
    QUuid m_actionTypeId;
    QUuid m_deviceId;
    QVariantMap m_params;
};

#endif // ACTION_H
