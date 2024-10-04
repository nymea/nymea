/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2024, nymea GmbH
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

#include "logentry.h"

LogEntry::LogEntry()
{

}

LogEntry::LogEntry(const QDateTime &timestamp, const QString &source, const QVariantMap &values):
    m_timestamp(timestamp),
    m_source(source),
    m_values(values)
{

}

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

LogEntries::LogEntries(): QList<LogEntry>()
{

}

LogEntries::LogEntries(const QList<LogEntry> &other):
    QList<LogEntry>(other)
{

}

QVariant LogEntries::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void LogEntries::put(const QVariant &variant)
{
    append(variant.value<LogEntry>());
}
