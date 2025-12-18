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

#include "logginghandler.h"
#include "logging/logengine.h"
#include "logging/logentry.h"
#include "loggingcategories.h"
#include "nymeacore.h"

namespace nymeaserver {

LoggingHandler::LoggingHandler(LogEngine *logEngine, QObject *parent)
    : JsonHandler(parent)
    , m_logEngine(logEngine)
{
    // Enums
    registerEnum<Types::SampleRate>();
    registerEnum<Qt::SortOrder>();

    // Objects
    registerObject<LogEntry, LogEntries>();

    // Methods
    QString description;
    QVariantMap params;
    QVariantMap returns;
    description = "Get the LogEntries matching the given filter. \n"
                  "\"sources\": Builtin sources are: \"core\", \"rules\", \"scripts\", \"integrations\". May be extended by experience plugins.\n"
                  "\"columns\": Columns to be returned.\n"
                  "\"filter\": A map of column:value entries. Only = is supported currently.\n"
                  "\"startTime\": The datetime of the oldest entry, in ms.\n"
                  "\"endTime\": The datetime of the newest entry, in ms.\n"
                  "\"sampleRate\": If given, returns a sampled series of the values, filling in gaps with the previous value.\n"
                  "\"sortOrder\": Sort order of results. Note that this impacts the filling of gaps when resampling.\n"
                  "\"limit\": Maximum amount of entries to be returned.\n"
                  "\"offset\": Offset to be skipped before returning entries.";
    params.insert("sources", QVariantList() << enumValueName(String));
    params.insert("o:columns", QVariantList() << enumValueName(String));
    params.insert("o:filter", enumValueName(Variant));
    params.insert("o:startTime", enumValueName(Uint));
    params.insert("o:endTime", enumValueName(Uint));
    params.insert("o:sampleRate", enumRef<Types::SampleRate>());
    params.insert("o:sortOrder", enumRef<Qt::SortOrder>());
    params.insert("o:limit", enumValueName(Int));
    params.insert("o:offset", enumValueName(Int));
    returns.insert("o:logEntries", objectRef<LogEntries>());
    returns.insert("count", enumValueName(Int));
    returns.insert("offset", enumValueName(Int));
    registerMethod("GetLogEntries", description, params, returns, Types::PermissionScopeControlThings);

    // Notifications
    params.clear();
    description = "Emitted when a log entry is added. This will only be emitted for discrete series, not for resampled entries";
    params.insert("logEntry", objectRef<LogEntry>());
    registerNotification("LogEntryAdded", description, params);

    connect(m_logEngine, &LogEngine::logEntryAdded, this, [this](const LogEntry &logEntry) { emit LogEntryAdded({{"logEntry", packLogEntry(logEntry)}}); });
}

QString LoggingHandler::name() const
{
    return "Logging";
}

JsonReply *LoggingHandler::GetLogEntries(const QVariantMap &params) const
{
    JsonReply *reply = createAsyncReply("GetLogEntries");

    QVariantMap filter = params.value("filter").toMap();
    QDateTime startTime;
    if (params.contains("startTime")) {
        startTime = QDateTime::fromMSecsSinceEpoch(params.value("startTime").toULongLong());
    }
    QDateTime endTime;
    if (params.contains("endTime")) {
        endTime = QDateTime::fromMSecsSinceEpoch(params.value("endTime").toULongLong());
    }
    Types::SampleRate sampleRate = Types::SampleRateAny;
    if (params.contains("sampleRate")) {
        QMetaEnum sampleRateEnum = QMetaEnum::fromType<Types::SampleRate>();
        sampleRate = static_cast<Types::SampleRate>(sampleRateEnum.keyToValue(params.value("sampleRate").toByteArray()));
    }
    QStringList columns;
    if (params.contains("columns")) {
        columns = params.value("columns").toStringList();
    }

    int offset = params.value("offset").toInt();
    int limit = params.value("limit").toInt();

    Qt::SortOrder sortOrder = Qt::AscendingOrder;
    if (params.contains("sortOrder")) {
        sortOrder = enumNameToValue<Qt::SortOrder>(params.value("sortOrder").toString());
    }

    QStringList sources = params.value("sources").toStringList();
    LogFetchJob *job = m_logEngine->fetchLogEntries(sources, columns, startTime, endTime, filter, sampleRate, sortOrder, offset, limit);
    connect(job, &LogFetchJob::finished, reply, [reply](const LogEntries &entries) {
        QVariantList entryMaps;
        foreach (const LogEntry &logEntry, entries) {
            QVariantMap logEntryMap;
            logEntryMap.insert("timestamp", logEntry.timestamp().toMSecsSinceEpoch());
            logEntryMap.insert("source", logEntry.source());
            QVariantMap values;
            foreach (const QString &valueKey, logEntry.values().keys()) {
                values.insert(valueKey, logEntry.values().value(valueKey));
            }
            logEntryMap.insert("values", values);
            entryMaps.append(logEntryMap);
        }
        QVariantMap params{{"count", entries.count()}, {"offset", 0}, {"logEntries", entryMaps}};
        reply->setData(params);
        reply->finished();
    });
    return reply;
}

QVariantMap LoggingHandler::packLogEntry(const LogEntry &logEntry)
{
    QVariantMap logEntryMap;
    logEntryMap.insert("timestamp", logEntry.timestamp().toMSecsSinceEpoch());
    logEntryMap.insert("source", logEntry.source());
    QVariantMap values;
    foreach (const QString &valueKey, logEntry.values().keys()) {
        values.insert(valueKey, logEntry.values().value(valueKey));
    }
    logEntryMap.insert("values", values);
    return logEntryMap;
}

} // namespace nymeaserver
