/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#include "logginghandler.h"
#include "logging/logengine.h"
#include "logging/logfilter.h"
#include "loggingcategories.h"
#include "guhcore.h"

namespace guhserver {

LoggingHandler::LoggingHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap params;
    QVariantMap returns;

    // Notifications
    params.clear(); returns.clear();
    setDescription("LogEntryAdded", "Emitted whenever an entry is appended to the logging system.");
    params.insert("logEntry", JsonTypes::logEntryRef());
    setParams("LogEntryAdded", params);

    params.clear(); returns.clear();
    setDescription("GetLogEntries", "Get the LogEntries matching the given filter.");

    params.insert("o:startTime", JsonTypes::basicTypeToString(JsonTypes::Int));
    params.insert("o:endTime", JsonTypes::basicTypeToString(JsonTypes::Int));
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

    connect(GuhCore::instance()->logEngine(), &LogEngine::logEntryAdded, this, &LoggingHandler::logEntryAdded);
}

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

JsonReply* LoggingHandler::GetLogEntries(const QVariantMap &params) const
{
    qCDebug(dcJsonRpc) << "asked for log entries" << params;

    LogFilter filter = JsonTypes::unpackLogFilter(params);

    QVariantList entries;
    foreach (const LogEntry &entry, GuhCore::instance()->logEngine()->logEntries(filter)) {
        entries.append(JsonTypes::packLogEntry(entry));
    }
    QVariantMap returns = statusToReply(Logging::LoggingErrorNoError);
    returns.insert("logEntries", entries);
    return createReply(returns);
}

}
