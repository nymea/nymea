/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef DEVICEPAIRINGINFO_H
#define DEVICEPAIRINGINFO_H

#include "libnymea.h"
#include "typeutils.h"
#include "device.h"
#include "types/param.h"
#include <QUrl>

class LIBNYMEA_EXPORT DevicePairingInfo
{
public:
    DevicePairingInfo();
    DevicePairingInfo(const DeviceClassId &deviceClassId, const QString &deviceName, const ParamList &params, const DeviceId &deviceId = DeviceId());

    PairingTransactionId transactionId() const;

    DeviceClassId deviceClassId() const;

    DeviceId deviceId() const;

    QString deviceName() const;

    ParamList params() const;

    Device::DeviceError status() const;
    void setStatus(Device::DeviceError status);

    QString message() const;
    void setMessage(const QString &message);

    QUrl oAuthUrl() const;
    void setOAuthUrl(const QUrl &url);

private:
    PairingTransactionId m_transactionId;
    DeviceClassId m_deviceClassId;
    DeviceId m_deviceId;
    QString m_deviceName;
    ParamList m_params;

    Device::DeviceError m_status = Device::DeviceErrorNoError;
    QString m_message;
    QUrl m_oAuthUrl;
};

#endif // DEVICEPAIRINGINFO_H
