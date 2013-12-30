#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QUuid>

class Device: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUuid id READ id CONSTANT)

public:
    Device(QObject *parent = 0);

    QUuid id() const;

private:
    QUuid m_id;
};

#endif
