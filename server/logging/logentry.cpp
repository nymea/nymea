/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "logentry.h"

#include <QDebug>

LogEntry::LogEntry(QDateTime timestamp, Logging::LoggingLevel level, Logging::LoggingSource source, int errorCode):
    m_timestamp(timestamp),
    m_level(level),
    m_source(source),
    m_eventType(Logging::LoggingEventTypeTrigger),
    m_active(false),
    m_errorCode(errorCode)
{

}

LogEntry::LogEntry(Logging::LoggingLevel level, Logging::LoggingSource source, int errorCode):
    LogEntry(QDateTime::currentDateTime(), level, source, errorCode)
{

}

LogEntry::LogEntry(Logging::LoggingSource source):
    LogEntry(Logging::LoggingLevelInfo, source)
{

}

QDateTime LogEntry::timestamp() const
{
    return m_timestamp;
}

Logging::LoggingLevel LogEntry::level() const
{
    return m_level;
}

Logging::LoggingSource LogEntry::source() const
{
    return m_source;
}

QUuid LogEntry::typeId() const
{
    return m_typeId;
}

void LogEntry::setTypeId(const QUuid &typeId) {
    m_typeId = typeId;
}

DeviceId LogEntry::deviceId() const
{
    return m_deviceId;
}

void LogEntry::setDeviceId(const DeviceId &deviceId)
{
    m_deviceId = deviceId;
}

QString LogEntry::value() const
{
    return m_value;
}

void LogEntry::setValue(const QString &value)
{
    m_value = value;
}

Logging::LoggingEventType LogEntry::eventType() const
{
    return m_eventType;
}

bool LogEntry::active() const
{
    return m_active;
}

void LogEntry::setActive(bool active)
{
    m_eventType = Logging::LoggingEventTypeActiveChange;
    m_active = active;
}

int LogEntry::errorCode() const
{
    return m_errorCode;
}
