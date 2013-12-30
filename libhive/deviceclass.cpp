#include "deviceclass.h"

DeviceClass::DeviceClass(const QUuid &id, QObject *parent):
    QObject(parent),
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
