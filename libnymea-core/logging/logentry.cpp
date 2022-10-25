/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::LogEntry
    \brief  Represents an entry of the log database.

    \ingroup logs
    \inmodule core

    A \l{LogEntry} represents an a nymea event which can be stored from the \l{LogEngine} to the database.
    Each LogEntry has a timestamp an can be loaded from the database and stored in the database.

    \sa LogEngine, LogFilter, LogsResource, LoggingHandler
*/

/*! \fn QDebug nymeaserver::operator<< (QDebug dbg, const LogEntry &entry);;
    Writes the \l{LogEntry} \a entry to the given \a dbg. This method gets used just for debugging.
*/

#include "logentry.h"
#include "nymeacore.h"

#include <QDebug>
#include <QMetaEnum>

namespace nymeaserver {

LogEntry::LogEntry()
{

}

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
    LogEntry(NymeaCore::instance()->timeManager()->currentDateTime(), level, source, errorCode)
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

/*! Returns the thingId of this \l{LogEntry}. */
ThingId LogEntry::thingId() const
{
    return m_thingId;
}

/*! Sets the \a thingId of this \l{LogEntry}. */
void LogEntry::setThingId(const ThingId &thingId)
{
    m_thingId = thingId;
}

/*! Returns the value of this \l{LogEntry}. */
QVariant LogEntry::value() const
{
    return m_value;
}

/*! Sets the \a value of this \l{LogEntry}. */
void LogEntry::setValue(const QVariant &value)
{
    m_value = value;
}

/*! Returns the event type of this \l{LogEntry}. */
Logging::LoggingEventType LogEntry::eventType() const
{
    return m_eventType;
}

/*! Sets the \a eventType of this \l{LogEntry}. */
void LogEntry::setEventType(const Logging::LoggingEventType &eventType)
{
    m_eventType = eventType;
}

/*! Returns true if this \l{LogEntry} is a system active type. */
bool LogEntry::active() const
{
    return m_active;
}

/*! Sets this \l{LogEntry} to \a active. */
void LogEntry::setActive(bool active)
{
    m_active = active;
}

/*! Returns the error code of this \l{LogEntry}. */
int LogEntry::errorCode() const
{
    return m_errorCode;
}


QDebug operator<<(QDebug dbg, const LogEntry &entry)
{
    QDebugStateSaver saver(dbg);
    QMetaEnum metaEnum;
    dbg.nospace() << "LogEntry (" << entry.timestamp().toString() << ")" << endl;
    dbg.nospace() << " time stamp: " << entry.timestamp().toTime_t() << endl;
    dbg.nospace() << "    ThingId: " << entry.thingId().toString() << endl;
    dbg.nospace() << "    type id: " << entry.typeId().toString() << endl;
    metaEnum = QMetaEnum::fromType<Logging::LoggingSource>();
    dbg.nospace() << "     source: " << metaEnum.valueToKey(entry.source()) << endl;
    metaEnum = QMetaEnum::fromType<Logging::LoggingLevel>();
    dbg.nospace() << "      level: " << metaEnum.valueToKey(entry.level()) << endl;
    metaEnum = QMetaEnum::fromType<Logging::LoggingEventType>();
    dbg.nospace() << "  eventType: " << metaEnum.valueToKey(entry.eventType()) << endl;
    dbg.nospace() << " error code: " << entry.errorCode() << endl;
    dbg.nospace() << "     active: " << entry.active() << endl;
    dbg.nospace() << "      value: " << entry.value() << endl;
    return dbg;
}

LogEntries::LogEntries()
{

}

LogEntries::LogEntries(const QList<LogEntry> &other): QList<LogEntry>(other)
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

}
