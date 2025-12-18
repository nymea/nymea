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

#include "logentry.h"

LogEntry::LogEntry() {}

LogEntry::LogEntry(const QDateTime &timestamp, const QString &source, const QVariantMap &values)
    : m_timestamp(timestamp)
    , m_source(source)
    , m_values(values)
{}

QDateTime LogEntry::timestamp() const
{
    return m_timestamp;
}

QString LogEntry::source() const
{
    return m_source;
}

QVariantMap LogEntry::values() const
{
    return m_values;
}

LogEntries::LogEntries()
    : QList<LogEntry>()
{}

LogEntries::LogEntries(const QList<LogEntry> &other)
    : QList<LogEntry>(other)
{}

QVariant LogEntries::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void LogEntries::put(const QVariant &variant)
{
    append(variant.value<LogEntry>());
}
