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

#include <QObject>
#include <QUrl>

#include "device.h"

class DeviceManager;

class LIBNYMEA_EXPORT DevicePairingInfo: public QObject
{
    Q_OBJECT
public:
    DevicePairingInfo(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const DeviceId &deviceId, const QString &deviceName, const ParamList &params, const DeviceId &parentDeviceId, DeviceManager *parent, quint32 timeout = 0);

    PairingTransactionId transactionId() const;

    DeviceClassId deviceClassId() const;
    DeviceId deviceId() const;
    QString deviceName() const;
    ParamList params() const;
    DeviceId parentDeviceId() const;

    QUrl oAuthUrl() const;
    void setOAuthUrl(const QUrl &oAuthUrl);

    Device::DeviceError status() const;
    QString displayMessage() const;
    QString translatedDisplayMessage(const QLocale &locale) const;

public slots:
    void finish(Device::DeviceError status, const QString &displayMessage = QString());

signals:
    void finished();
    void aborted();

private:
    PairingTransactionId m_transactionId;
    DeviceClassId m_deviceClassId;
    DeviceId m_deviceId;
    QString m_deviceName;
    ParamList m_params;
    DeviceId m_parentDeviceId;

    QUrl m_oAuthUrl;

    bool m_finished = false;
    Device::DeviceError m_status = Device::DeviceErrorNoError;
    QString m_displayMessage;
    DeviceManager *m_deviceManager = nullptr;
};

#endif // DEVICEPAIRINGINFO_H
