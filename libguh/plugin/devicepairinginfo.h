/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stuerz <simon.stuerz@guh.guru>                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICEPAIRINGINFO_H
#define DEVICEPAIRINGINFO_H

#include "libguh.h"
#include "typeutils.h"
#include "types/param.h"

class LIBGUH_EXPORT DevicePairingInfo
{
public:
    DevicePairingInfo();
    DevicePairingInfo(const DeviceClassId &deviceClassId, const QString &deviceName, const ParamList &params);
    DevicePairingInfo(const DeviceClassId &deviceClassId, const QString &deviceName, const DeviceDescriptorId &deviceDescriptorId);

    DeviceClassId deviceClassId() const;

    QString deviceName() const;

    ParamList params() const;

    DeviceDescriptorId deviceDescriptorId() const;

private:
    DeviceClassId m_deviceClassId;
    QString m_deviceName;
    ParamList m_params;
    DeviceDescriptorId m_deviceDescriptorId;
};

#endif // DEVICEPAIRINGINFO_H
