#ifndef DEVICEDESCRIPTION_H
#define DEVICEDESCRIPTION_H

#include <typeutils.h>
#include <types/param.h>

#include <QVariantMap>

class DeviceDescriptor
{
public:
    DeviceDescriptor();
    DeviceDescriptor(const DeviceClassId &deviceClassId, const QString &title = QString(), const QString &description = QString());
    DeviceDescriptor(const DeviceDescriptorId &id, const DeviceClassId &deviceClassId, const QString &title = QString(), const QString &description = QString());

    bool isValid() const;

    DeviceDescriptorId id() const;
    DeviceClassId deviceClassId() const;

    QString title() const;
    void setTitle(const QString &title);

    QString description() const;
    void setDescription(const QString &description);

    QList<Param> params() const;
    void setParams(const QList<Param> &params);

private:
    DeviceDescriptorId m_id;
    DeviceClassId m_deviceClassId;
    QString m_title;
    QString m_description;
    QList<Param> m_params;
};

#endif // DEVICEDESCRIPTION_H
