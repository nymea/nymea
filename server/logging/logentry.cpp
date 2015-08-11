/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

/*! \fn QDebug guhserver::operator<< (QDebug dbg, const LogEntry &entry);;
    Writes the \l{LogEntry} \a entry to the given \a dbg. This method gets used just for debugging.
*/

#include "logentry.h"
#include "jsonrpc/jsontypes.h"

#include <QDebug>

namespace guhserver {

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

QString LogEntry::levelString() const
{
    switch (m_level) {
    case Logging::LoggingLevelAlert:
        return "LoggingLevelAlert";
    case Logging::LoggingLevelInfo:
        return "LoggingLevelInfo";
    default:
        return "< Unknown >";
    }
}

Logging::LoggingSource LogEntry::source() const
{
    return m_source;
}

QString LogEntry::sourceString() const
{
    switch (m_source) {
    case Logging::LoggingSourceActions:
        return "LoggingSourceActions";
    case Logging::LoggingSourceEvents:
        return "LoggingSourceEvents";
    case Logging::LoggingSourceRules:
        return "LoggingSourceRules";
    case Logging::LoggingSourceStates:
        return "LoggingSourceStates";
    case Logging::LoggingSourceSystem:
        return "LoggingSourceSystem";
    default:
        return "< Unknown >";
    }
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

QString LogEntry::eventTypeString() const
{
    switch (m_eventType) {
    case Logging::LoggingEventTypeActiveChange:
        return "LoggingEventTypeActiveChange";
    case Logging::LoggingEventTypeTrigger:
        return "LoggingEventTypeTrigger";
    default:
        return "< Unknown >";
    }
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

QDebug operator<<(QDebug dbg, const LogEntry &entry)
{
    dbg.nospace() << "LogEntry (count:" << entry.timestamp().toString() << endl;
    dbg.nospace() << " time stamp: " << entry.timestamp().toTime_t() << endl;
    dbg.nospace() << "   DeviceId: " << entry.deviceId().toString() << endl;
    dbg.nospace() << "    type id: " << entry.typeId().toString() << endl;
    dbg.nospace() << "     source: " << entry.sourceString() << endl;
    dbg.nospace() << "      level: " << entry.levelString() << endl;
    dbg.nospace() << "  eventType: " << entry.eventTypeString() << endl;
    dbg.nospace() << " error code: " << entry.errorCode() << endl;
    dbg.nospace() << "     active: " << entry.active() << endl;
    dbg.nospace() << "      value: " << entry.value() << endl;
    return dbg.space();
}

}
