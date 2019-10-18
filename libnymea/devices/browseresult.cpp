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

#include "browseresult.h"

#include <QTimer>

BrowseResult::BrowseResult(Device *device, const QString &itemId, const QLocale &locale, QObject *parent, quint32 timeout):
    QObject(parent),
    m_device(device),
    m_itemId(itemId),
    m_locale(locale)
{
    connect(this, &BrowseResult::finished, this, &BrowseResult::deleteLater, Qt::QueuedConnection);

    if (timeout > 0) {
        QTimer::singleShot(timeout, this, [this] {
            emit aborted();
            finish(Device::DeviceErrorTimeout);
        });
    }
}

Device *BrowseResult::device() const
{
    return m_device;
}

QString BrowseResult::itemId() const
{
    return m_itemId;
}

QLocale BrowseResult::locale() const
{
    return m_locale;
}

BrowserItems BrowseResult::items() const
{
    return m_items;
}

bool BrowseResult::isFinished() const
{
    return m_finished;
}

Device::DeviceError BrowseResult::status() const
{
    return m_status;
}

void BrowseResult::addItem(const BrowserItem &item)
{
    m_items.append(item);
}

void BrowseResult::addItems(const BrowserItems &items)
{
    m_items.append(items);
}

void BrowseResult::finish(Device::DeviceError status)
{
    m_finished = true;
    m_status = status;
    staticMetaObject.invokeMethod(this, "finished", Qt::QueuedConnection);
}
