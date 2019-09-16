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

#include "devicesetupinfo.h"

#include "deviceplugin.h"
#include "devicemanager.h"

DeviceSetupInfo::DeviceSetupInfo(Device *device, DeviceManager *deviceManager):
    QObject(deviceManager),
    m_device(device),
    m_deviecManager(deviceManager)
{
    connect(this, &DeviceSetupInfo::finished, this, &DeviceSetupInfo::deleteLater, Qt::QueuedConnection);
}

Device *DeviceSetupInfo::device() const
{
    return m_device;
}

bool DeviceSetupInfo::isFinished() const
{
    return m_finished;
}

Device::DeviceError DeviceSetupInfo::status() const
{
    return m_status;
}

QString DeviceSetupInfo::displayMessage() const
{
    return m_displayMessage;
}

QString DeviceSetupInfo::translatedDisplayMessage(const QLocale &locale)
{
    if (!m_deviecManager || !m_device) {
        return m_displayMessage;
    }

    return m_deviecManager->translate(m_device->pluginId(), m_displayMessage.toUtf8(), locale);
}

void DeviceSetupInfo::finish(Device::DeviceError status, const QString &displayMessage)
{
    m_finished = true;
    m_status = status;
    m_displayMessage = displayMessage;
    staticMetaObject.invokeMethod(this, "finished", Qt::QueuedConnection);
}
