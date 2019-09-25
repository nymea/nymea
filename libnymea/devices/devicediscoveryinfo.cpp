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

#include "devicediscoveryinfo.h"
#include "devicemanager.h"

#include <QTimer>

DeviceDiscoveryInfo::DeviceDiscoveryInfo(const DeviceClassId &deviceClassId, const ParamList &params, DeviceManager *deviceManager, quint32 timeout):
    QObject(deviceManager),
    m_deviceClassId(deviceClassId),
    m_params(params),
    m_deviceManager(deviceManager)
{
    connect(this, &DeviceDiscoveryInfo::finished, this, &DeviceDiscoveryInfo::deleteLater, Qt::QueuedConnection);

    if (timeout > 0) {
        QTimer::singleShot(timeout, this, [this] {
            emit aborted();
            finish(Device::DeviceErrorTimeout);
        });
    }
}

DeviceClassId DeviceDiscoveryInfo::deviceClassId() const
{
    return m_deviceClassId;
}

ParamList DeviceDiscoveryInfo::params() const
{
    return m_params;
}

bool DeviceDiscoveryInfo::isFinished() const
{
    return m_finished;
}

Device::DeviceError DeviceDiscoveryInfo::status() const
{
    return m_status;
}

void DeviceDiscoveryInfo::addDeviceDescriptor(const DeviceDescriptor &deviceDescriptor)
{
    m_deviceDescriptors.append(deviceDescriptor);
}

void DeviceDiscoveryInfo::addDeviceDescriptors(const DeviceDescriptors &deviceDescriptors)
{
    m_deviceDescriptors.append(deviceDescriptors);
}

DeviceDescriptors DeviceDiscoveryInfo::deviceDescriptors() const
{
    return m_deviceDescriptors;
}

QString DeviceDiscoveryInfo::displayMessage() const
{
    return m_displayMessage;
}

QString DeviceDiscoveryInfo::translatedDisplayMessage(const QLocale &locale)
{
    if (!m_deviceManager) {
        return m_displayMessage;
    }
    DeviceClass deviceClass = m_deviceManager->findDeviceClass(m_deviceClassId);
    return m_deviceManager->translate(deviceClass.pluginId(), m_displayMessage.toUtf8(), locale);
}

void DeviceDiscoveryInfo::finish(Device::DeviceError status, const QString &displayMessage)
{
    m_finished = true;
    m_status = status;
    m_displayMessage = displayMessage;
    staticMetaObject.invokeMethod(this, "finished", Qt::QueuedConnection);
}
