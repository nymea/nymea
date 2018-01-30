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
                   "Each list element of a given filter will be connected with OR "
                   "to each other. Each of the given filters will be connected with AND "
                   "to each other.");
    timeFilter.insert("o:startDate", JsonTypes::basicTypeToString(JsonTypes::Int));
    timeFilter.insert("o:endDate", JsonTypes::basicTypeToString(JsonTypes::Int));
    params.insert("o:timeFilters", QVariantList() << timeFilter);
    params.insert("o:loggingSources", QVariantList() << JsonTypes::loggingSourceRef());
    params.insert("o:loggingLevels", QVariantList() << JsonTypes::loggingLevelRef());
    params.insert("o:eventTypes", QVariantList() << JsonTypes::loggingEventTypeRef());
    params.insert("o:typeIds", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("o:deviceIds", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("o:values", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::Variant));
    setParams("GetLogEntries", params);
    returns.insert("loggingError", JsonTypes::loggingErrorRef());
    returns.insert("o:logEntries", QVariantList() << JsonTypes::logEntryRef());
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
    qCDebug(dcJsonRpc) << "Notify \"Logging.LogEntryAdded\"";
    QVariantMap params;
    params.insert("logEntry", JsonTypes::packLogEntry(logEntry));
    emit LogEntryAdded(params);
}

void LoggingHandler::logDatabaseUpdated()
{
    qCDebug(dcJsonRpc) << "Notify \"Logging.LogDatabaseUpdated\"";
    emit LogDatabaseUpdated(QVariantMap());
}

JsonReply* LoggingHandler::GetLogEntries(const QVariantMap &params) const
{
    qCDebug(dcJsonRpc) << "Asked for log entries" << params;

    LogFilter filter = JsonTypes::unpackLogFilter(params);

    QVariantList entries;
    foreach (const LogEntry &entry, NymeaCore::instance()->logEngine()->logEntries(filter)) {
        entries.append(JsonTypes::packLogEntry(entry));
    }
    QVariantMap returns = statusToReply(Logging::LoggingErrorNoError);
    returns.insert("logEntries", entries);
    return createReply(returns);
}

}
