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


#ifndef DEVICESETUPINFO_H
#define DEVICESETUPINFO_H

#include "device.h"

class DeviceSetupInfo
{
public:
    DeviceSetupInfo();
    DeviceSetupInfo(const DeviceId &deviceId, Device::DeviceError status = Device::DeviceErrorNoError, const QString &displayMessage = QString());

    DeviceId deviceId() const;

    Device::DeviceError status() const;
    void setStatus(Device::DeviceError status);

    QString displayMessage() const;
    void setDisplayMessage(const QString &displayMessage);

private:
    DeviceId m_deviceId;
    Device::DeviceError m_status = Device::DeviceErrorSetupFailed;
    QString m_displayMessage;
};

#endif // DEVICESETUPINFO_H
