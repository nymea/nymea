/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
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
