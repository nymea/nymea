/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICEDISCOVERYINFO_H
#define DEVICEDISCOVERYINFO_H

#include "typeutils.h"

#include "device.h"
#include "devicedescriptor.h"

#include <QMetaType>

class LIBNYMEA_EXPORT DeviceDiscoveryInfo
{
public:
    DeviceDiscoveryInfo();
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

Q_DECLARE_METATYPE(DeviceDiscoveryInfo)

#endif // DEVICEDISCOVERYINFO_H
