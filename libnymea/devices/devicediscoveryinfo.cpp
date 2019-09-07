#include "devicediscoveryinfo.h"

DeviceDiscoveryInfo::DeviceDiscoveryInfo(const DeviceClassId &deviceClassId):
    m_id(DiscoveryTransactionId::createDiscoveryTransactionId()),
    m_deviceClassId(deviceClassId)
{

}

DiscoveryTransactionId DeviceDiscoveryInfo::id() const
{
    return m_id;
}

DeviceClassId DeviceDiscoveryInfo::deviceClassId() const
{
    return m_deviceClassId;
}

Device::DeviceError DeviceDiscoveryInfo::status() const
{
    return m_status;
}

void DeviceDiscoveryInfo::setStatus(Device::DeviceError status)
{
    m_status = status;
}

DeviceDescriptors DeviceDiscoveryInfo::deviceDescriptors() const
{
    return m_deviceDescriptors;
}

void DeviceDiscoveryInfo::setDeviceDescriptors(const DeviceDescriptors &deviceDescriptors)
{
    m_deviceDescriptors = deviceDescriptors;
}
