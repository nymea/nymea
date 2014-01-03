#ifndef ACTION_H
#define ACTION_H

#include <QUuid>
#include <QVariantList>

class Action
{
public:
    explicit Action(const QUuid &deviceId, const QUuid &id = QUuid::createUuid());

    bool isValid() const;

    QUuid id() const;
    QUuid deviceId() const;

    QString name() const;
    void setName(const QString &name);

    QVariantMap params() const;
    void setParams(const QVariantMap &params);

private:
    QUuid m_id;
    QUuid m_deviceId;
    QString m_name;
    QVariantMap m_params;
};

#endif // ACTION_H
