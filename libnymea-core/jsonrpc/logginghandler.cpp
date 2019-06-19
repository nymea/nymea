    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::LoggingHandler
    \brief This subclass of \l{JsonHandler} processes the JSON requests for the \tt Logging namespace of the JSON-RPC API.

    \ingroup json
    \inmodule core

    This \l{JsonHandler} will be created in the \l{JsonRPCServer} and used to handle JSON-RPC requests
    for the \tt {Logging} namespace of the API.

    \sa LogEngine, JsonHandler, JsonRPCServer
*/

/*! \fn void nymeaserver::LoggingHandler::LogEntryAdded(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{LogEntry} was added to the database.
    The \a params contain the map for the notification.
*/

/*! \fn void nymeaserver::LoggingHandler::LogDatabaseUpdated(const QVariantMap &params);
    This signal is emitted to the API notifications when the logging aatabase has been updated (i.e. \l{Device} or \l{Rule} removed).
    The \a params contain the map for the notification.
*/

#include "logginghandler.h"
#include "logging/logengine.h"
#include "logging/logfilter.h"
#include "loggingcategories.h"
#include "nymeacore.h"

namespace nymeaserver {

/*! Constructs a new \l LoggingHandler with the given \a parent. */
LoggingHandler::LoggingHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap params;
    QVariantMap returns;

    QVariantMap timeFilter;
    params.clear(); returns.clear();
    setDescription("GetLogEntries", "Get the LogEntries matching the given filter. "
                   "The result set will contain entries matching all filter rules combined. "
                   "If multiple options are given for a single filter type, the result set will "
                   "contain entries matching any of those. The offset starts at the newest entry "
                   "in the result set. By default all items are returned. Example: If the specified "
                   "filter returns a total amount of 100 entries:\n"
                   "- a offset value of 10 would include the oldest 90 entries\n"
                   "- a offset value of 0 would return all 100 entries\n\n"
                   "The offset is particularly useful in combination with the maxCount property and "
                   "can be used for pagination. E.g. A result set of 10000 entries can be fetched in "
                   " batches of 1000 entries by fetching\n"
                   "1) offset 0, maxCount 1000: Entries 0 to 9999\n"
                   "2) offset 10000, maxCount 1000: Entries 10000 - 19999\n"
                   "3) offset 20000, maxCount 1000: Entries 20000 - 29999\n"
                   "...");
    timeFilter.insert("o:startDate", JsonTypes::basicTypeToString(JsonTypes::Int));
    timeFilter.insert("o:endDate", JsonTypes::basicTypeToString(JsonTypes::Int));
    params.insert("o:timeFilters", QVariantList() << timeFilter);
    params.insert("o:loggingSources", QVariantList() << JsonTypes::loggingSourceRef());
    params.insert("o:loggingLevels", QVariantList() << JsonTypes::loggingLevelRef());
    params.insert("o:eventTypes", QVariantList() << JsonTypes::loggingEventTypeRef());
    params.insert("o:typeIds", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("o:deviceIds", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("o:values", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::Variant));
    params.insert("o:limit", JsonTypes::basicTypeToString(JsonTypes::Int));
    params.insert("o:offset", JsonTypes::basicTypeToString(JsonTypes::Int));
    setParams("GetLogEntries", params);
    returns.insert("loggingError", JsonTypes::loggingErrorRef());
    returns.insert("o:logEntries", QVariantList() << JsonTypes::logEntryRef());
    returns.insert("count", JsonTypes::basicTypeToString(JsonTypes::Int));
    returns.insert("offset", JsonTypes::basicTypeToString(JsonTypes::Int));
    setReturns("GetLogEntries", returns);

    // Notifications
    params.clear();
    setDescription("LogEntryAdded", "Emitted whenever an entry is appended to the logging system. ");
    params.insert("logEntry", JsonTypes::logEntryRef());
    setParams("LogEntryAdded", params);

    params.clear();
    setDescription("LogDatabaseUpdated", "Emitted whenever the database was updated. "
                   "The database will be updated when a log entry was deleted. A log "
                   "entry will be deleted when the corresponding device or a rule will "
                   "be removed, or when the oldest entry of the database was deleted to "
                   "keep to database in the size limits.");
    setParams("LogDatabaseUpdated", params);

    connect(NymeaCore::instance()->logEngine(), &LogEngine::logEntryAdded, this, &LoggingHandler::logEntryAdded);
    connect(NymeaCore::instance()->logEngine(), &LogEngine::logDatabaseUpdated, this, &LoggingHandler::logDatabaseUpdated);
}

/*! Returns the name of the \l{LoggingHandler}. In this case \b Logging.*/
QString LoggingHandler::name() const
{
    return "Logging";
}

void LoggingHandler::logEntryAdded(const LogEntry &logEntry)
{
    QVariantMap params;
    params.insert("logEntry", JsonTypes::packLogEntry(logEntry));
    emit LogEntryAdded(params);
}

void LoggingHandler::logDatabaseUpdated()
{
    emit LogDatabaseUpdated(QVariantMap());
}

JsonReply* LoggingHandler::GetLogEntries(const QVariantMap &params) const
{
    LogFilter filter = JsonTypes::unpackLogFilter(params);

    QVariantList entries;
    foreach (const LogEntry &entry, NymeaCore::instance()->logEngine()->logEntries(filter)) {
        entries.append(JsonTypes::packLogEntry(entry));
    }
    QVariantMap returns = statusToReply(Logging::LoggingErrorNoError);

    returns.insert("logEntries", entries);
    returns.insert("offset", filter.offset());
    returns.insert("count", entries.count());
    return createReply(returns);
}

}
