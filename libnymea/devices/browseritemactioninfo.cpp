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

#include "browseritemactioninfo.h"

#include <QTimer>

BrowserItemActionInfo::BrowserItemActionInfo(Device *device, const BrowserItemAction &browserItemAction, QObject *parent, quint32 timeout):
    QObject(parent),
    m_device(device),
    m_browserItemAction(browserItemAction)
{
    connect(this, &BrowserItemActionInfo::finished, this, &BrowserItemActionInfo::deleteLater, Qt::QueuedConnection);

    if (timeout > 0) {
        QTimer::singleShot(timeout, this, [this] {
            emit aborted();
            finish(Device::DeviceErrorTimeout);
        });
    }
}

Device *BrowserItemActionInfo::device() const
{
    return m_device;
}

BrowserItemAction BrowserItemActionInfo::browserItemAction() const
{
    return m_browserItemAction;
}

bool BrowserItemActionInfo::isFinished() const
{
    return m_finished;
}

Device::DeviceError BrowserItemActionInfo::status() const
{
    return m_status;
}

void BrowserItemActionInfo::finish(Device::DeviceError status)
{
    m_finished = true;
    m_status = status;
    staticMetaObject.invokeMethod(this, "finished", Qt::QueuedConnection);
}
