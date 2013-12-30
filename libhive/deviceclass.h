#ifndef DEVICECLASS_H
#define DEVICECLASS_H

#include <QObject>
#include <QUuid>

class DeviceClass: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUuid id READ id CONSTANT)

public:
    DeviceClass(const QUuid &id, QObject *parent = 0);
    virtual ~DeviceClass();

    virtual QUuid id() const;

private:
    QUuid m_id;
};

#endif
