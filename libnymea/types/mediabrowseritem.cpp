// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "mediabrowseritem.h"

MediaBrowserItem::MediaBrowserItem(const QString &id, const QString &displayName, bool browsable, bool executable)
    : BrowserItem(id, displayName, browsable, executable)
{
    // Init defaults
    m_extendedProperties["mediaIcon"] = static_cast<int>(MediaBrowserIconNone);
    m_extendedProperties["playCount"] = 0;

    m_extendedPropertiesFlags = BrowserItem::ExtendedPropertiesMedia;
}

MediaBrowserItem::MediaBrowserIcon MediaBrowserItem::mediaIcon() const
{
    return static_cast<MediaBrowserIcon>(m_extendedProperties.value("mediaIcon").toInt());
}

void MediaBrowserItem::setMediaIcon(MediaBrowserIcon mediaIcon)
{
    m_extendedProperties["mediaIcon"] = static_cast<int>(mediaIcon);
}

int MediaBrowserItem::playCount() const
{
    return m_extendedProperties.value("playCount").toInt();
}

void MediaBrowserItem::setPlayCount(int playCount)
{
    m_extendedProperties["playCount"] = playCount;
}
