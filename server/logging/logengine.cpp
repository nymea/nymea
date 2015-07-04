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

#include "logengine.h"
#include "loggingcategories.h"
#include "logging.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMetaEnum>
#include <QDateTime>

#define DB_SCHEMA_VERSION 2

namespace guhserver {

LogEngine::LogEngine(QObject *parent):
    QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("/tmp/guhd-logs.sqlite");

    if (!m_db.isValid()) {
        qCWarning(dcLogEngine) << "Database not valid:" << m_db.lastError().driverText() << m_db.lastError().databaseText();
        return;
    }
    if (!m_db.open()) {
        qCWarning(dcLogEngine) << "Error opening log database:" << m_db.lastError().driverText() << m_db.lastError().databaseText();
        return;
    }

    initDB();
}

QList<LogEntry> LogEngine::logEntries(const LogFilter &filter) const
{
    qCDebug(dcLogEngine) << "Read logging database" << m_db.databaseName();

    QList<LogEntry> results;
    QSqlQuery query;

    QString queryCall = "SELECT * FROM entries;";
    if (filter.isEmpty()) {
        query.exec(queryCall);
    } else {
        queryCall = QString("SELECT * FROM entries WHERE %1;").arg(filter.queryString());
        query.exec(queryCall);
    }

    while (query.next()) {
        LogEntry entry(
                    QDateTime::fromTime_t(query.value("timestamp").toLongLong()),
                    (Logging::LoggingLevel)query.value("loggingLevel").toInt(),
                    (Logging::LoggingSource)query.value("sourceType").toInt(),
                    query.value("errorCode").toInt());
        entry.setTypeId(query.value("typeId").toUuid());
        entry.setDeviceId(DeviceId(query.value("deviceId").toString()));
        entry.setValue(query.value("value").toString());
        if ((Logging::LoggingEventType)query.value("loggingEventType").toInt() == Logging::LoggingEventTypeActiveChange) {
            entry.setActive(query.value("active").toBool());
        }
        //qCDebug(dcLogEngine) << entry;
        results.append(entry);
    }
    qCDebug(dcLogEngine) << "Fetched" << results.count() << "entries for db query:" << queryCall;

    return results;
}

void LogEngine::logSystemEvent(bool active, Logging::LoggingLevel level)
{
    LogEntry entry(level, Logging::LoggingSourceSystem);
    entry.setActive(active);
    appendLogEntry(entry);
    emit logEntryAdded(entry);
}

void LogEngine::logEvent(const Event &event)
{
    QStringList valueList;
    Logging::LoggingSource sourceType;
    if (event.isStateChangeEvent()) {
        sourceType = Logging::LoggingSourceStates;
        valueList << event.param("value").value().toString();
    } else {
        sourceType = Logging::LoggingSourceEvents;
        foreach (const Param &param, event.params()) {
            valueList << param.value().toString();
        }
    }
    LogEntry entry(sourceType);
    entry.setTypeId(event.eventTypeId());
    entry.setDeviceId(event.deviceId());
    entry.setValue(valueList.join(", "));
    appendLogEntry(entry);
    emit logEntryAdded(entry);
}

void LogEngine::logAction(const Action &action, Logging::LoggingLevel level, int errorCode)
{
    QStringList valueList;
    foreach (const Param &param, action.params()) {
        valueList << param.value().toString();
    }
    LogEntry entry(level, Logging::LoggingSourceActions, errorCode);
    entry.setTypeId(action.actionTypeId());
    entry.setDeviceId(action.deviceId());
    entry.setValue(valueList.join(", "));
    appendLogEntry(entry);
    emit logEntryAdded(entry);
}

void LogEngine::logRuleTriggered(const Rule &rule)
{
    LogEntry entry(Logging::LoggingSourceRules);
    entry.setTypeId(rule.id());
    appendLogEntry(entry);
    emit logEntryAdded(entry);
}

void LogEngine::logRuleActiveChanged(const Rule &rule)
{
    LogEntry entry(Logging::LoggingSourceRules);
    entry.setTypeId(rule.id());
    entry.setActive(rule.active());
    appendLogEntry(entry);
    emit logEntryAdded(entry);
}

void LogEngine::appendLogEntry(const LogEntry &entry)
{
    QString queryString = QString("INSERT INTO entries (timestamp, loggingEventType, loggingLevel, sourceType, typeId, deviceId, value, active, errorCode) values ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9');")
            .arg(entry.timestamp().toTime_t())
            .arg(entry.eventType())
            .arg(entry.level())
            .arg(entry.source())
            .arg(entry.typeId().toString())
            .arg(entry.deviceId().toString())
            .arg(entry.value())
            .arg(entry.active())
            .arg(entry.errorCode());

    QSqlQuery query;
    query.exec(queryString);

    if (query.lastError().isValid()) {
        qCWarning(dcLogEngine) << "Error writing log entry. Driver error:" << query.lastError().driverText() << "Database error:" << query.lastError().databaseText();
    }
}

void LogEngine::initDB()
{
    m_db.close();
    m_db.open();

    QSqlQuery query;

    if (!m_db.tables().contains("metadata")) {
        query.exec("CREATE TABLE metadata (key varchar(10), data varchar(40));");
        query.exec(QString("INSERT INTO metadata (key, data) VALUES('version', '%1');").arg(DB_SCHEMA_VERSION));
    }

    query.exec("SELECT data FROM metadata WHERE key = 'version';");
    if (query.next()) {
        int version = query.value("data").toInt();
        if (version != DB_SCHEMA_VERSION) {
            qCWarning(dcLogEngine) << "Log schema version not matching! Schema upgrade not implemented yet. Logging might fail.";
        } else {
            qCDebug(dcLogEngine) << "Log database schema version" << DB_SCHEMA_VERSION << "matches";
        }
    } else {
        qCWarning(dcLogEngine) << "Broken log database. Version not found in metadata table.";
    }

    if (!m_db.tables().contains("sourceTypes")) {
        query.exec("CREATE TABLE sourceTypes (id int, name varchar(20), PRIMARY KEY(id));");
        //qCDebug(dcLogEngine) << query.lastError().databaseText();
        QMetaEnum logTypes = Logging::staticMetaObject.enumerator(Logging::staticMetaObject.indexOfEnumerator("LoggingSource"));
        Q_ASSERT_X(logTypes.isValid(), "LogEngine", "Logging has no enum LoggingSource");
        for (int i = 0; i < logTypes.keyCount(); i++) {
            query.exec(QString("INSERT INTO sourceTypes (id, name) VALUES(%1, '%2');").arg(i).arg(logTypes.key(i)));
        }
    }

    if (!m_db.tables().contains("loggingEventTypes")) {
        query.exec("CREATE TABLE loggingEventTypes (id int, name varchar(20), PRIMARY KEY(id));");
        //qCDebug(dcLogEngine) << query.lastError().databaseText();
        QMetaEnum logTypes = Logging::staticMetaObject.enumerator(Logging::staticMetaObject.indexOfEnumerator("LoggingEventType"));
        Q_ASSERT_X(logTypes.isValid(), "LogEngine", "Logging has no enum LoggingEventType");
        for (int i = 0; i < logTypes.keyCount(); i++) {
            query.exec(QString("INSERT INTO loggingEventTypes (id, name) VALUES(%1, '%2');").arg(i).arg(logTypes.key(i)));
        }
    }

    if (!m_db.tables().contains("entries")) {
        query.exec("CREATE TABLE entries "
                   "("
                   "timestamp int,"
                   "loggingLevel int,"
                   "sourceType int,"
                   "typeId varchar(38),"
                   "deviceId varchar(38),"
                   "value varchar(100),"
                   "loggingEventType int,"
                   "active bool,"
                   "errorCode int,"
                   "FOREIGN KEY(sourceType) REFERENCES sourceTypes(id),"
                   "FOREIGN KEY(loggingEventType) REFERENCES loggingEventTypes(id)"
                   ");");
        if (query.lastError().isValid()) {
            qCWarning(dcLogEngine) << "Error creating log table in database. Driver error:" << query.lastError().driverText() << "Database error:" << query.lastError().databaseText();
        }
    }
}

}
