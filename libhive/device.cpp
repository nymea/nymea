#include "device.h"

Device::Device(QObject *parent):
    QObject(parent)
{

}

QUuid Device::id() const
{
    return m_id;
}
