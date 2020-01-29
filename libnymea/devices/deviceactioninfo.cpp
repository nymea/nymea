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

#include "deviceactioninfo.h"

#include "devicemanager.h"

#include <QTimer>

DeviceActionInfo::DeviceActionInfo(Device *device, const Action &action, DeviceManager *parent, quint32 timeout):
    QObject(parent),
    m_device(device),
    m_action(action),
    m_deviceManager(parent)
{
    connect(this, &DeviceActionInfo::finished, this, &DeviceActionInfo::deleteLater, Qt::QueuedConnection);

    if (timeout > 0) {
        QTimer::singleShot(timeout, this, [this] {
            emit aborted();
            finish(Device::DeviceErrorTimeout);
        });
    }
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
