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

#include "browseritemresult.h"

#include <QTimer>

BrowserItemResult::BrowserItemResult(Device *device, const QString &itemId, const QLocale &locale, QObject *parent, quint32 timeout):
    QObject(parent),
    m_device(device),
    m_itemId(itemId),
    m_locale(locale)
{
    connect(this, &BrowserItemResult::finished, this, &BrowserItemResult::deleteLater, Qt::QueuedConnection);

    if (timeout > 0) {
        QTimer::singleShot(timeout, this, [this] {
            emit aborted();
            finish(Device::DeviceErrorTimeout);
        });
    }
}

Device *BrowserItemResult::device() const
{
    return m_device;
}

QString BrowserItemResult::itemId() const
{
    return m_itemId;
}

QLocale BrowserItemResult::locale() const
{
    return m_locale;
}

BrowserItem BrowserItemResult::item() const
{
    return m_item;
}

bool BrowserItemResult::isFinished() const
{
    return m_finished;
}

Device::DeviceError BrowserItemResult::status() const
{
    return m_status;
}

void BrowserItemResult::finish(const BrowserItem &item)
{
    m_item = item;
    finish(Device::DeviceErrorNoError);
}

void BrowserItemResult::finish(Device::DeviceError status)
{
    m_finished = true;
    m_status = status;
    staticMetaObject.invokeMethod(this, "finished", Qt::QueuedConnection);
}
