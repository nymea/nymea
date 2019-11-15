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
#include "logging/logentry.h"
#include "logging/logvaluetool.h"
#include "loggingcategories.h"
#include "nymeacore.h"

namespace nymeaserver {

/*! Constructs a new \l LoggingHandler with the given \a parent. */
LoggingHandler::LoggingHandler(QObject *parent) :
    JsonHandler(parent)
{
    // Enums
    registerEnum<Logging::LoggingSource>();
    registerEnum<Logging::LoggingLevel>();
    registerEnum<Logging::LoggingEventType>();
    registerEnum<Logging::LoggingError>();

    // Objects
    registerObject<LogEntry, LogEntries>();

    // Methods
    QString description; QVariantMap params; QVariantMap returns;
    description = "Get the LogEntries matching the given filter. "
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
                   "...";
    QVariantMap timeFilter;
    timeFilter.insert("o:startDate", enumValueName(Int));
    timeFilter.insert("o:endDate", enumValueName(Int));
    params.insert("o:timeFilters", QVariantList() << timeFilter);
    params.insert("o:loggingSources", QVariantList() << enumRef<Logging::LoggingSource>());
    params.insert("o:loggingLevels", QVariantList() << enumRef<Logging::LoggingLevel>());
    params.insert("o:eventTypes", QVariantList() << enumRef<Logging::LoggingEventType>());
    params.insert("o:typeIds", QVariantList() << enumValueName(Uuid));
    params.insert("o:deviceIds", QVariantList() << enumValueName(Uuid));
    params.insert("o:values", QVariantList() << enumValueName(Variant));
    params.insert("o:limit", enumValueName(Int));
    params.insert("o:offset", enumValueName(Int));
    returns.insert("loggingError", enumRef<Logging::LoggingError>());
    returns.insert("o:logEntries", objectRef<LogEntries>());
    returns.insert("count", enumValueName(Int));
    returns.insert("offset", enumValueName(Int));
    registerMethod("GetLogEntries", description, params, returns);

    // Notifications
    params.clear();
    description = "Emitted whenever an entry is appended to the logging system. ";
    params.insert("logEntry", objectRef<LogEntry>());
    registerNotification("LogEntryAdded", description, params);

    params.clear();
    description = "Emitted whenever the database was updated. "
                   "The database will be updated when a log entry was deleted. A log "
                   "entry will be deleted when the corresponding device or a rule will "
                   "be removed, or when the oldest entry of the database was deleted to "
                   "keep to database in the size limits.";
    registerNotification("LogDatabaseUpdated", description, params);

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
    params.insert("logEntry", packLogEntry(logEntry));
    emit LogEntryAdded(params);
}

void LoggingHandler::logDatabaseUpdated()
{
    emit LogDatabaseUpdated(QVariantMap());
}

JsonReply* LoggingHandler::GetLogEntries(const QVariantMap &params) const
{
    LogFilter filter = unpackLogFilter(params);

    LogEntriesFetchJob *job = NymeaCore::instance()->logEngine()->fetchLogEntries(filter);

    JsonReply *reply = createAsyncReply("GetLogEntries");

    connect(job, &LogEntriesFetchJob::finished, reply, [reply, job, filter](){

        QVariantList entries;
        foreach (const LogEntry &entry, job->results()) {
            entries.append(packLogEntry(entry));
        }
        QVariantMap returns;
        returns.insert("loggingError", enumValueName<Logging::LoggingError>(Logging::LoggingErrorNoError));
        returns.insert("logEntries", entries);
        returns.insert("offset", filter.offset());
        returns.insert("count", entries.count());

        reply->setData(returns);
        reply->finished();
    });

    return reply;
}

QVariantMap LoggingHandler::packLogEntry(const LogEntry &logEntry)
{
    QVariantMap logEntryMap;
    logEntryMap.insert("timestamp", logEntry.timestamp().toMSecsSinceEpoch());
    logEntryMap.insert("loggingLevel", enumValueName<Logging::LoggingLevel>(logEntry.level()));
    logEntryMap.insert("source", enumValueName<Logging::LoggingSource>(logEntry.source()));
    logEntryMap.insert("eventType", enumValueName<Logging::LoggingEventType>(logEntry.eventType()));

    if (logEntry.eventType() == Logging::LoggingEventTypeActiveChange)
        logEntryMap.insert("active", logEntry.active());

    if (logEntry.eventType() == Logging::LoggingEventTypeEnabledChange)
        logEntryMap.insert("active", logEntry.active());

    if (logEntry.level() == Logging::LoggingLevelAlert) {
        switch (logEntry.source()) {
        case Logging::LoggingSourceRules:
            logEntryMap.insert("errorCode", enumValueName<RuleEngine::RuleError>(static_cast<RuleEngine::RuleError>(logEntry.errorCode())));
            break;
        case Logging::LoggingSourceActions:
        case Logging::LoggingSourceEvents:
        case Logging::LoggingSourceStates:
        case Logging::LoggingSourceBrowserActions:
            logEntryMap.insert("errorCode", enumValueName<Device::DeviceError>(static_cast<Device::DeviceError>(logEntry.errorCode())));
            break;
        case Logging::LoggingSourceSystem:
            // FIXME: Update this once we support error codes for the general system
            //            logEntryMap.insert("errorCode", "");
            break;
        }
    }

    switch (logEntry.source()) {
    case Logging::LoggingSourceActions:
    case Logging::LoggingSourceEvents:
    case Logging::LoggingSourceStates:
    case Logging::LoggingSourceBrowserActions:
        if (!logEntry.typeId().isNull()) {
            logEntryMap.insert("typeId", logEntry.typeId());
        }
        logEntryMap.insert("deviceId", logEntry.deviceId());
        logEntryMap.insert("value", LogValueTool::convertVariantToString(logEntry.value()));
        break;
    case Logging::LoggingSourceSystem:
        logEntryMap.insert("active", logEntry.active());
        break;
    case Logging::LoggingSourceRules:
        logEntryMap.insert("typeId", logEntry.typeId().toString());
        break;
    }

    return logEntryMap;
}

LogFilter LoggingHandler::unpackLogFilter(const QVariantMap &logFilterMap)
{
    LogFilter filter;
    if (logFilterMap.contains("timeFilters")) {
        QVariantList timeFilters = logFilterMap.value("timeFilters").toList();
        foreach (const QVariant &timeFilter, timeFilters) {
            QVariantMap timeFilterMap = timeFilter.toMap();
            QDateTime startDate; QDateTime endDate;
            if (timeFilterMap.contains("startDate"))
                startDate = QDateTime::fromTime_t(timeFilterMap.value("startDate").toUInt());

            if (timeFilterMap.contains("endDate"))
                endDate = QDateTime::fromTime_t(timeFilterMap.value("endDate").toUInt());

            filter.addTimeFilter(startDate, endDate);
        }
    }

    if (logFilterMap.contains("loggingSources")) {
        QVariantList loggingSources = logFilterMap.value("loggingSources").toList();
        foreach (const QVariant &source, loggingSources) {
            filter.addLoggingSource(enumNameToValue<Logging::LoggingSource>(source.toString()));
        }
    }
    if (logFilterMap.contains("loggingLevels")) {
        QVariantList loggingLevels = logFilterMap.value("loggingLevels").toList();
        foreach (const QVariant &level, loggingLevels) {
            filter.addLoggingLevel(enumNameToValue<Logging::LoggingLevel>(level.toString()));
        }
    }
    if (logFilterMap.contains("eventTypes")) {
        QVariantList eventTypes = logFilterMap.value("eventTypes").toList();
        foreach (const QVariant &eventType, eventTypes) {
            filter.addLoggingEventType(enumNameToValue<Logging::LoggingEventType>(eventType.toString()));
        }
    }
    if (logFilterMap.contains("typeIds")) {
        QVariantList typeIds = logFilterMap.value("typeIds").toList();
        foreach (const QVariant &typeId, typeIds) {
            filter.addTypeId(typeId.toUuid());
        }
    }
    if (logFilterMap.contains("deviceIds")) {
        QVariantList deviceIds = logFilterMap.value("deviceIds").toList();
        foreach (const QVariant &deviceId, deviceIds) {
            filter.addDeviceId(DeviceId(deviceId.toString()));
        }
    }
    if (logFilterMap.contains("values")) {
        QVariantList values = logFilterMap.value("values").toList();
        foreach (const QVariant &value, values) {
            filter.addValue(value.toString());
        }
    }
    if (logFilterMap.contains("limit")) {
        filter.setLimit(logFilterMap.value("limit", -1).toInt());
    }
    if (logFilterMap.contains("offset")) {
        filter.setOffset(logFilterMap.value("offset").toInt());
    }

    return filter;
}

}
