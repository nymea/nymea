#ifndef DEVICEDISCOVERYINFO_H
#define DEVICEDISCOVERYINFO_H

#include "typeutils.h"

#include "device.h"
#include "devicedescriptor.h"

class DeviceDiscoveryInfo
{
public:
    DeviceDiscoveryInfo(const DeviceClassId &deviceClassId);

    DiscoveryTransactionId id() const;
    DeviceClassId deviceClassId() const;

    Device::DeviceError status() const;
    void setStatus(Device::DeviceError status);

    DeviceDescriptors deviceDescriptors() const;
    void setDeviceDescriptors(const DeviceDescriptors &deviceDescriptors);

private:
    DiscoveryTransactionId m_id;
    DeviceClassId m_deviceClassId;
    Device::DeviceError m_status = Device::DeviceErrorNoError;
    DeviceDescriptors m_deviceDescriptors;
};

#endif // DEVICEDISCOVERYINFO_H
