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

#include "devicepairinginfo.h"
#include "devicemanager.h"

#include <QTimer>

DevicePairingInfo::DevicePairingInfo(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const DeviceId &deviceId, const QString &deviceName, const ParamList &params, const DeviceId &parentDeviceId, DeviceManager *parent, quint32 timeout):
    QObject(parent),
    m_transactionId(pairingTransactionId),
    m_deviceClassId(deviceClassId),
    m_deviceId(deviceId),
    m_deviceName(deviceName),
    m_params(params),
    m_parentDeviceId(parentDeviceId)
{
    connect(this, &DevicePairingInfo::finished, this, &DevicePairingInfo::deleteLater, Qt::QueuedConnection);

    if (timeout > 0) {
        QTimer::singleShot(timeout, this, [this] {
            emit aborted();
            finish(Device::DeviceErrorTimeout);
        });
    }
}

PairingTransactionId DevicePairingInfo::transactionId() const
{
    return m_transactionId;
}

DeviceClassId DevicePairingInfo::deviceClassId() const
{
    return m_deviceClassId;
}

DeviceId DevicePairingInfo::deviceId() const
{
    return m_deviceId;
}

QString DevicePairingInfo::deviceName() const
{
    return m_deviceName;
}

ParamList DevicePairingInfo::params() const
{
    return m_params;
}

DeviceId DevicePairingInfo::parentDeviceId() const
{
    return m_parentDeviceId;
}

QUrl DevicePairingInfo::oAuthUrl() const
{
    return m_oAuthUrl;
}

void DevicePairingInfo::setOAuthUrl(const QUrl &oAuthUrl)
{
    m_oAuthUrl = oAuthUrl;
}

Device::DeviceError DevicePairingInfo::status() const
{
    return m_status;
}

QString DevicePairingInfo::displayMessage() const
{
    return m_displayMessage;
}

QString DevicePairingInfo::translatedDisplayMessage(const QLocale &locale) const
{
    if (!m_deviceManager) {
        return m_displayMessage;
    }
    DeviceClass deviceClass = m_deviceManager->findDeviceClass(m_deviceClassId);
    return m_deviceManager->translate(deviceClass.pluginId(), m_displayMessage.toUtf8(), locale);
}

void DevicePairingInfo::finish(Device::DeviceError status, const QString &displayMessage)
{
    m_finished = true;
    m_status = status;
    m_displayMessage = displayMessage;
    staticMetaObject.invokeMethod(this, "finished", Qt::QueuedConnection);
}

