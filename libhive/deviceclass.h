#ifndef DEVICECLASS_H
#define DEVICECLASS_H

#include <QUuid>

class DeviceClass
{
public:
    DeviceClass(const QUuid &id);
    virtual ~DeviceClass();

    QUuid id() const;

    QString name() const;
    void setName(const QString &name);

private:
    QUuid m_id;
    QString m_name;
};

#endif
