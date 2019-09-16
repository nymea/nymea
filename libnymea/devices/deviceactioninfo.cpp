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

#include "deviceactioninfo.h"

#include "devicemanager.h"

DeviceActionInfo::DeviceActionInfo(Device *device, const Action &action, DeviceManager *parent):
    QObject(parent),
    m_device(device),
    m_action(action),
    m_deviceManager(parent)
{
    connect(this, &DeviceActionInfo::finished, this, &DeviceActionInfo::deleteLater, Qt::QueuedConnection);
}

Device *DeviceActionInfo::device() const
{
    return m_device;
}

Action DeviceActionInfo::action() const
{
    return m_action;
}

bool DeviceActionInfo::isFinished() const
{
    return m_finished;
}

Device::DeviceError DeviceActionInfo::status() const
{
    return m_status;
}

QString DeviceActionInfo::displayMessage() const
{
    return m_displayMessage;
}

QString DeviceActionInfo::translatedDisplayMessage(const QLocale &locale)
{
    if (!m_deviceManager) {
        return m_displayMessage;
    }
    return m_deviceManager->translate(m_device->pluginId(), m_displayMessage.toUtf8(), locale);
}

void DeviceActionInfo::finish(Device::DeviceError status, const QString &displayMessage)
{
    m_finished = true;
    m_status = status;
    m_displayMessage = displayMessage;
    staticQtMetaObject.invokeMethod(this, "finished", Qt::QueuedConnection);
}
