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

#include "servicedata.h"

ServiceData::ServiceData(const ThingId &thingId, const QDateTime &timestamp)
    : m_thingId(thingId)
    , m_timestamp(timestamp)
{}

ThingId ServiceData::thingId() const
{
    return m_thingId;
}

void ServiceData::setThingId(const ThingId &thingId)
{
    m_thingId = thingId;
}

QDateTime ServiceData::timestamp() const
{
    return m_timestamp;
}

void ServiceData::setTimestamp(const QDateTime &timestamp)
{
    m_timestamp = timestamp;
}

QHash<QString, QVariant> ServiceData::data() const
{
    return m_data;
}

void ServiceData::insert(const QString &key, const QVariant &data)
{
    m_data.insert(key, data);
}

void ServiceData::insert(const QHash<QString, QVariant> data)
{
    foreach (const QString &key, data.keys()) {
        m_data.insert(key, data.value(key));
    }
}
