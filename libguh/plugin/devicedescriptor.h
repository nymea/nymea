#ifndef DEVICEDESCRIPTION_H
#define DEVICEDESCRIPTION_H

#include <typeutils.h>

#include <QVariantMap>

class DeviceDescriptor
{
public:
    DeviceDescriptor(const DeviceDescriptorId &id, const DeviceClassId &deviceClassId, const QString &title = QString(), const QString &description = QString());

    DeviceDescriptorId id() const;
    DeviceClassId deviceClassId() const;

    QString title() const;
    void setTitle(const QString &title);

    QString description() const;
    void setDescription(const QString &description);

    QVariantMap params() const;
    void setParams(const QVariantMap &params);

private:
    DeviceDescriptorId m_id;
    DeviceClassId m_deviceClassId;
    QString m_title;
    QString m_description;
    QVariantMap m_params;
};

#endif // DEVICEDESCRIPTION_H
