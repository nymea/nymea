#include "deviceclass.h"

DeviceClass::DeviceClass(const QUuid &id):
    m_id(id)
{

}

DeviceClass::~DeviceClass()
{

}

QUuid DeviceClass::id() const
{
    return m_id;
}

QString DeviceClass::name() const
{
    return m_name;
}

void DeviceClass::setName(const QString &name)
{
    m_name = name;
}
