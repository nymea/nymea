/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
