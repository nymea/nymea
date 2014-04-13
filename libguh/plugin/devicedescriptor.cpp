#include "devicedescriptor.h"

DeviceDescriptor::DeviceDescriptor()
{

}

DeviceDescriptor::DeviceDescriptor(const DeviceDescriptorId &id, const DeviceClassId &deviceClassId, const QString &title, const QString &description) :
    m_id(id),
    m_deviceClassId(deviceClassId),
    m_title(title),
    m_description(description)
{
}

bool DeviceDescriptor::isValid() const
{
    return !m_id.isNull() && !m_deviceClassId.isNull();
}

DeviceDescriptorId DeviceDescriptor::id() const
{
    return m_id;
}

DeviceClassId DeviceDescriptor::deviceClassId() const
{
    return m_deviceClassId;
}

QString DeviceDescriptor::title() const
{
    return m_title;
}

void DeviceDescriptor::setTitle(const QString &title)
{
    m_title = title;
}

QString DeviceDescriptor::description() const
{
    return m_description;
}

void DeviceDescriptor::setDescription(const QString &description)
{
    m_description = description;
}

QVariantMap DeviceDescriptor::params() const
{
    return m_params;
}

void DeviceDescriptor::setParams(const QVariantMap &params)
{
    m_params = params;
}
