#ifndef ACTION_H
#define ACTION_H

#include <QUuid>
#include <QVariantList>

class Action
{
public:
    Action(const QUuid &id = QUuid(), const QUuid &deviceId = QUuid());
    bool isValid() const;

    QUuid id() const;
    QUuid deviceId() const;

    QString name() const;
    void setName(const QString &name);

    QVariantList params() const;
    void setParams(const QVariantList &params);

private:
    QUuid m_id;
    QUuid m_deviceId;
    QString m_name;
    QVariantList m_params;
};

#endif // ACTION_H
