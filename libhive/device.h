#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QUuid>

class Device: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUuid id READ id CONSTANT)

public:
    Device(const QUuid &deviceClassId, QObject *parent = 0);

    QUuid id() const;
    QUuid deviceClassId() const;

    QString name() const;
    void setName(const QString &name);

private:
    QUuid m_id;
    QUuid m_deviceClassId;
    QString m_name;
};

#endif
