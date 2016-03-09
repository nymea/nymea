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

/*!
    \class guhserver::LogEntry
    \brief  Represents an entry of the log database.

    \ingroup logs
    \inmodule core

    A \l{LogEntry} represents an a guh event which can be stored from the \l{LogEngine} to the database.
    Each LogEntry has a timestamp an can be loaded from the database and stored in the database.

    \sa LogEngine, LogFilter, LogsResource, LoggingHandler
*/

/*! \fn QDebug guhserver::operator<< (QDebug dbg, const LogEntry &entry);;
    Writes the \l{LogEntry} \a entry to the given \a dbg. This method gets used just for debugging.
*/

#include "logentry.h"
#include "jsonrpc/jsontypes.h"

#include <QDebug>

namespace guhserver {

/*! Constructs a \l{LogEntry} with the given \a timestamp, \a level, \a source and \a errorCode.*/
LogEntry::LogEntry(QDateTime timestamp, Logging::LoggingLevel level, Logging::LoggingSource source, int errorCode):
    m_timestamp(timestamp),
    m_level(level),
    m_source(source),
    m_eventType(Logging::LoggingEventTypeTrigger),
    m_active(false),
    m_errorCode(errorCode)
{

}

/*! Constructs a \l{LogEntry} with the given \a level, \a source and \a errorCode.*/
LogEntry::LogEntry(Logging::LoggingLevel level, Logging::LoggingSource source, int errorCode):
    LogEntry(QDateTime::currentDateTime(), level, source, errorCode)
{

}

/*! Constructs a \l{LogEntry} with the given \a source.*/
LogEntry::LogEntry(Logging::LoggingSource source):
    LogEntry(Logging::LoggingLevelInfo, source)
{

}

/*! Returns the timestamp of this \l{LogEntry}. */
QDateTime LogEntry::timestamp() const
{
    return m_timestamp;
}

/*! Returns the level of this \l{LogEntry}. */
Logging::LoggingLevel LogEntry::level() const
{
    return m_level;
}

/*! Returns the source of this \l{LogEntry}. */
Logging::LoggingSource LogEntry::source() const
{
    return m_source;
}

/*! Returns the type ID of this \l{LogEntry}. */
QUuid LogEntry::typeId() const
{
    return m_typeId;
}

/*! Sets the \a typeId of this \l{LogEntry}. */
void LogEntry::setTypeId(const QUuid &typeId) {
    m_typeId = typeId;
}

/*! Returns the deviceId of this \l{LogEntry}. */
DeviceId LogEntry::deviceId() const
{
    return m_deviceId;
}

/*! Sets the \a deviceId of this \l{LogEntry}. */
void LogEntry::setDeviceId(const DeviceId &deviceId)
{
    m_deviceId = deviceId;
}

/*! Returns the value of this \l{LogEntry}. */
QString LogEntry::value() const
{
    return m_value;
}

/*! Sets the \a value of this \l{LogEntry}. */
void LogEntry::setValue(const QString &value)
{
    m_value = value;
}

/*! Returns the event type of this \l{LogEntry}. */
Logging::LoggingEventType LogEntry::eventType() const
{
    return m_eventType;
}

/*! Returns true if this \l{LogEntry} is a system active type. */
bool LogEntry::active() const
{
    return m_active;
}

/*! Sets this \l{LogEntry} to \a active. */
void LogEntry::setActive(bool active)
{
    m_eventType = Logging::LoggingEventTypeActiveChange;
    m_active = active;
}

/*! Returns the error code of this \l{LogEntry}. */
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
    dbg.nospace() << "     source: " << JsonTypes::loggingSourceToString(entry.source()) << endl;
    dbg.nospace() << "      level: " << JsonTypes::loggingLevelToString(entry.level()) << endl;
    dbg.nospace() << "  eventType: " << JsonTypes::loggingEventTypeToString(entry.eventType()) << endl;
    dbg.nospace() << " error code: " << entry.errorCode() << endl;
    dbg.nospace() << "     active: " << entry.active() << endl;
    dbg.nospace() << "      value: " << entry.value() << endl;
    return dbg.space();
}

}
