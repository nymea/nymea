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

#include "nymeasettings.h"
#include "logengine.h"
#include "loggingcategories.h"
#include "logging.h"
#include "logvaluetool.h"

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QMetaEnum>
#include <QDateTime>
#include <QFileInfo>
#include <QTime>
#include <QtConcurrent/QtConcurrent>

#define DB_SCHEMA_VERSION 4

namespace nymeaserver {

// IMPORTANT:
// DatabaseJobs run threaded, however, QSql is *not* threadsafe.
// It is crucial to *not* access m_db while the job queue is being processed.
// That is, entire setup of the DB must happen before processQueue() is called
// and teardown must happen only after the job queue is empty.

LogEngine::LogEngine(const QString &driver, const QString &dbName, const QString &hostname, const QString &username, const QString &password, int maxDBSize, QObject *parent):
    QObject(parent),
    m_username(username),
    m_password(password),
    m_dbMaxSize(maxDBSize)
{
    m_db = QSqlDatabase::addDatabase(driver, "logs");
    m_db.setDatabaseName(dbName);
    m_db.setHostName(hostname);
    m_trimSize = qRound(0.01 * m_dbMaxSize);
    m_maxQueueLength = 1000;

    qCDebug(dcLogEngine) << "Opening logging database" << m_db.databaseName() << "(Max size:" << m_dbMaxSize << "trim size:" << m_trimSize << ")";

    if (!m_db.isValid()) {
        qCWarning(dcLogEngine) << "Database not valid:" << m_db.lastError().driverText() << m_db.lastError().databaseText();
        rotate(m_db.databaseName());
    }

    if (!initDB(username, password)) {
        qCWarning(dcLogEngine()) << "Error initializing database. Trying to correct it.";
        if (QFileInfo(m_db.databaseName()).exists()) {
            rotate(m_db.databaseName());
            if (!initDB(username, password)) {
                qCWarning(dcLogEngine()) << "Error fixing log database. Giving up. Logs can't be stored.";
                return;
            }
        }
    }

    connect(&m_jobWatcher, SIGNAL(finished()), this, SLOT(handleJobFinished()));
    checkDBSize();
}

LogEngine::~LogEngine()
{
    // Process the job queue before allowing to shut down
    while (m_currentJob) {
        qCDebug(dcLogEngine()) << "Waiting for job to finish... (" << m_jobQueue.count() << "jobs left in queue)";
        m_jobWatcher.waitForFinished();
        // Make sure that the job queue is processes
        // We can't call processQueue ourselves because thread synchronisation is done via queued connections
        qApp->processEvents();
    }
    qCDebug(dcLogEngine()) << "Closing Database";
    m_db.close();
}

LogEntriesFetchJob *LogEngine::fetchLogEntries(const LogFilter &filter)
{
    QList<LogEntry> results;

    QString limitString;
    if (filter.limit() >= 0) {
        limitString.append(QString("LIMIT %1 ").arg(filter.limit()));
    }
    if (filter.offset() > 0) {
        limitString.append(QString("OFFSET %1").arg(QString::number(filter.offset())));
    }

    QString queryString;
    if (filter.isEmpty()) {
        queryString = QString("SELECT * FROM entries ORDER BY timestamp DESC %1;").arg(limitString);
    } else {
        queryString = QString("SELECT * FROM entries WHERE %1 ORDER BY timestamp DESC %2;").arg(filter.queryString()).arg(limitString);
    }

    DatabaseJob *job = new DatabaseJob(m_db, queryString, filter.values());
    LogEntriesFetchJob *fetchJob = new LogEntriesFetchJob(this);

    connect(job, &DatabaseJob::finished, this, [job, fetchJob](){
        fetchJob->deleteLater();
        if (job->error().isValid()) {
            qCWarning(dcLogEngine) << "Error fetching log entries. Driver error:" << job->error().driverText() << "Database error:" << job->error().databaseText();
            fetchJob->finished();
            return;
        }

        foreach (const QSqlRecord &result, job->results()) {
            LogEntry entry(
                        QDateTime::fromMSecsSinceEpoch(result.value("timestamp").toLongLong()),
                        static_cast<Logging::LoggingLevel>(result.value("loggingLevel").toInt()),
                        static_cast<Logging::LoggingSource>(result.value("sourceType").toInt()),
                        result.value("errorCode").toInt());
            entry.setTypeId(result.value("typeId").toUuid());
            entry.setThingId(ThingId(result.value("thingId").toString()));
            entry.setValue(result.value("value").toString());
            entry.setEventType(static_cast<Logging::LoggingEventType>(result.value("loggingEventType").toInt()));
            entry.setActive(result.value("active").toBool());

            fetchJob->m_results.append(entry);
        }
        qCDebug(dcLogEngine) << "Fetched" << fetchJob->results().count() << "entries for db query:" << job->executedQuery();
        fetchJob->finished();
    });

    enqueJob(job, true);

    return fetchJob;
}

ThingsFetchJob *LogEngine::fetchThings()
{
    QString queryString = QString("SELECT thingId FROM entries WHERE thingId != \"%1\" GROUP BY thingId;").arg(QUuid().toString());

    DatabaseJob *job = new DatabaseJob(m_db, queryString);
    ThingsFetchJob *fetchJob = new ThingsFetchJob(this);
    connect(job, &DatabaseJob::finished, this, [job, fetchJob](){
        fetchJob->deleteLater();
        if (job->error().type() != QSqlError::NoError) {
            qCWarning(dcLogEngine()) << "Error fetching device entries from log database:" << job->error().driverText() << job->error().databaseText();
            fetchJob->finished();
            return;
        }

        foreach (const QSqlRecord &result, job->results()) {
            fetchJob->m_results.append(ThingId(result.value("thingId").toUuid()));
        }
        fetchJob->finished();
    });
    enqueJob(job, true);
    return fetchJob;
}

bool LogEngine::jobsRunning() const
{
    return !m_jobQueue.isEmpty() || m_currentJob;
}

void LogEngine::setMaxLogEntries(int maxLogEntries, int trimSize)
{
    m_dbMaxSize = maxLogEntries;
    m_trimSize = trimSize;
    trim();
}

void LogEngine::clearDatabase()
{
    qCWarning(dcLogEngine) << "Clearing logging database.";

    QString queryDeleteString = QString("DELETE FROM entries;");

    DatabaseJob *job = new DatabaseJob(m_db, queryDeleteString);

    connect(job, &DatabaseJob::finished, this, [this, job](){
        if (job->error().type() != QSqlError::NoError) {
            qCWarning(dcLogEngine) << "Error clearing log database. Driver error:" << job->error().driverText() << "Database error:" << job->error().databaseText();
            return;
        }
        m_entryCount = 0;
        emit logDatabaseUpdated();
    });

    enqueJob(job);
}

void LogEngine::logSystemEvent(const QDateTime &dateTime, bool active, Logging::LoggingLevel level)
{
    LogEntry entry(dateTime, level, Logging::LoggingSourceSystem);
    entry.setEventType(Logging::LoggingEventTypeActiveChange);
    entry.setActive(active);
    appendLogEntry(entry);
}

void LogEngine::logEvent(const Event &event)
{
    QVariantList valueList;
    Logging::LoggingSource sourceType;
    if (event.isStateChangeEvent()) {
        sourceType = Logging::LoggingSourceStates;

        // There should only be one param
        if (!event.params().isEmpty())
            valueList << event.params().first().value();

    } else {
        sourceType = Logging::LoggingSourceEvents;
        foreach (const Param &param, event.params()) {
            valueList << param.value();
        }
    }

    LogEntry entry(sourceType);
    entry.setTypeId(event.eventTypeId());
    entry.setThingId(event.thingId());
    if (valueList.count() == 1) {
        entry.setValue(valueList.first());
    } else {
        entry.setValue(valueList);
    }
    appendLogEntry(entry);
}

void LogEngine::logAction(const Action &action, Logging::LoggingLevel level, int errorCode)
{
    LogEntry entry(level, Logging::LoggingSourceActions, errorCode);
    entry.setTypeId(action.actionTypeId());
    entry.setThingId(action.thingId());

    if (action.params().isEmpty()) {
        entry.setValue(QVariant());
    } else if (action.params().count() == 1) {
        entry.setValue(action.params().first().value());
    } else {
        QVariantList valueList;
        foreach (const Param &param, action.params()) {
            valueList << param.value();
        }
        entry.setValue(valueList);
    }
    appendLogEntry(entry);
}

void LogEngine::logBrowserAction(const BrowserAction &browserAction, Logging::LoggingLevel level, int errorCode)
{
    LogEntry entry(level, Logging::LoggingSourceBrowserActions, errorCode);
    entry.setThingId(browserAction.thingId());
    entry.setValue(browserAction.itemId());
    appendLogEntry(entry);
}

void LogEngine::logBrowserItemAction(const BrowserItemAction &browserItemAction, Logging::LoggingLevel level, int errorCode)
{
    LogEntry entry(level, Logging::LoggingSourceBrowserActions, errorCode);
    entry.setThingId(browserItemAction.thingId());
    entry.setTypeId(browserItemAction.actionTypeId());
    entry.setValue(browserItemAction.itemId());
    appendLogEntry(entry);
}

void LogEngine::logRuleTriggered(const Rule &rule)
{
    LogEntry entry(Logging::LoggingSourceRules);
    entry.setTypeId(rule.id());
    entry.setEventType(Logging::LoggingEventTypeTrigger);
    appendLogEntry(entry);
}

void LogEngine::logRuleActiveChanged(const Rule &rule)
{
    LogEntry entry(Logging::LoggingSourceRules);
    entry.setTypeId(rule.id());
    entry.setActive(rule.active());
    entry.setEventType(Logging::LoggingEventTypeActiveChange);
    appendLogEntry(entry);
}

void LogEngine::logRuleEnabledChanged(const Rule &rule, const bool &enabled)
{
    LogEntry entry(Logging::LoggingSourceRules);
    entry.setTypeId(rule.id());
    entry.setEventType(Logging::LoggingEventTypeEnabledChange);
    entry.setActive(enabled);
    appendLogEntry(entry);
}

void LogEngine::logRuleActionsExecuted(const Rule &rule)
{
    LogEntry entry(Logging::LoggingSourceRules);
    entry.setTypeId(rule.id());
    entry.setEventType(Logging::LoggingEventTypeActionsExecuted);
    appendLogEntry(entry);
}

void LogEngine::logRuleExitActionsExecuted(const Rule &rule)
{
    LogEntry entry(Logging::LoggingSourceRules);
    entry.setTypeId(rule.id());
    entry.setEventType(Logging::LoggingEventTypeExitActionsExecuted);
    appendLogEntry(entry);
}

void LogEngine::removeThingLogs(const ThingId &thingId)
{
    qCDebug(dcLogEngine) << "Deleting log entries from device" << thingId.toString();

    QString queryDeleteString = QString("DELETE FROM entries WHERE thingId = '%1';").arg(thingId.toString());

    DatabaseJob *job = new DatabaseJob(m_db, queryDeleteString);
    connect(job, &DatabaseJob::finished, this, [this, job, thingId](){
        if (job->error().type() != QSqlError::NoError) {
            qCWarning(dcLogEngine) << "Error deleting log entries from device" << thingId.toString() << ". Driver error:" << job->error().driverText() << "Database error:" << job->error().databaseText();
            return;
        }

        emit logDatabaseUpdated();
        checkDBSize();
    });

    enqueJob(job);
}

void LogEngine::removeRuleLogs(const RuleId &ruleId)
{
    qCDebug(dcLogEngine) << "Deleting log entries from rule" << ruleId.toString();

    QString queryDeleteString = QString("DELETE FROM entries WHERE typeId = '%1';").arg(ruleId.toString());

    DatabaseJob *job = new DatabaseJob(m_db, queryDeleteString);

    connect(job, &DatabaseJob::finished, this, [this, job, ruleId](){

        if (job->error().type() != QSqlError::NoError) {
            qCWarning(dcLogEngine) << "Error deleting log entries from rule" << ruleId.toString() << ". Driver error:" << job->error().driverText() << "Database error:" << job->error().databaseText();
            return;
        }

        emit logDatabaseUpdated();
        checkDBSize();
    });

    enqueJob(job);
}

void LogEngine::appendLogEntry(const LogEntry &entry)
{
    qCDebug(dcLogEngine()) << "Adding log entry:" << entry;
    QString queryString = QString("INSERT INTO entries (timestamp, loggingEventType, loggingLevel, sourceType, typeId, thingId, value, active, errorCode) values (?, ?, ?, ?, ?, ?, ?, ?, ?);");
    QVariantList bindValues;
    bindValues.append(entry.timestamp().toMSecsSinceEpoch());
    bindValues.append(entry.eventType());
    bindValues.append(entry.level());
    bindValues.append(entry.source());
    bindValues.append(entry.typeId().toString());
    bindValues.append(entry.thingId().toString());
    bindValues.append(LogValueTool::convertVariantToString(entry.value()));
    bindValues.append(entry.active());
    bindValues.append(entry.errorCode());

    DatabaseJob *job = new DatabaseJob(m_db, queryString, bindValues);

    // Check for log flooding. If we are exceeding the queue we'll start flagging log events of a certain type.
    // If we'll get more log events of the same type while the queue is still exceededd, we'll discard the old
    // ones and queue up the new one instead. The most recent one is more important (i.e. we don't want to lose
    // the last event in a series).
    if (m_jobQueue.count() > m_maxQueueLength) {
        qCDebug(dcLogEngine()) << "An excessive amount of data is being logged. (" << m_jobQueue.length() << "jobs in the queue)";
        if (m_flaggedJobs.contains(entry.typeId().toString() + entry.thingId().toString())) {
            if (m_flaggedJobs.value(entry.typeId().toString() + entry.thingId().toString()).count() > 10) {
                qCWarning(dcLogEngine()) << "Discarding log entry because of excessive log flooding.";
                DatabaseJob *job = m_flaggedJobs[entry.typeId().toString() + entry.thingId().toString()].takeFirst();
                int jobIdx = m_jobQueue.indexOf(job);
                m_jobQueue.takeAt(jobIdx)->deleteLater();
            }
        }
        m_flaggedJobs[entry.typeId().toString() + entry.thingId().toString()].append(job);
    }

    connect(job, &DatabaseJob::finished, this, [this, job, entry](){

        m_flaggedJobs[entry.typeId().toString() + entry.thingId().toString()].removeAll(job);

        if (job->error().type() != QSqlError::NoError) {
            qCWarning(dcLogEngine) << "Error writing log entry. Driver error:" << job->error().driverText() << "Database error:" << job->error().databaseText();
            qCWarning(dcLogEngine) << entry;
            m_dbMalformed = true;
            return;
        }


        emit logEntryAdded(entry);

        m_entryCount++;
        trim();
    });

    enqueJob(job);
}

void LogEngine::checkDBSize()
{
    DatabaseJob *job = new DatabaseJob(m_db, "SELECT COUNT(*) FROM entries;");
    connect(job, &DatabaseJob::finished, this, [this, job](){
        if (job->error().type() != QSqlError::NoError || job->results().count() == 0) {
            qCWarning(dcLogEngine()) << "Error fetching log DB size. Driver error:" << job->error().driverText() << "Database error:" << job->error().databaseText();
            m_entryCount = 0;
            return;
        }
        m_entryCount = job->results().first().value(0).toInt();
    });
    enqueJob(job, true);
}

void LogEngine::trim()
{
    if (m_dbMaxSize == -1 || m_entryCount < m_dbMaxSize) {
        // No trimming required
        return;
    }
    QDateTime startTime = QDateTime::currentDateTime();

    QString queryDeleteString = QString("DELETE FROM entries WHERE ROWID IN (SELECT ROWID FROM entries ORDER BY timestamp DESC LIMIT -1 OFFSET %1);").arg(QString::number(m_dbMaxSize - m_trimSize));

    DatabaseJob *deleteJob = new DatabaseJob(m_db, queryDeleteString);

    connect(deleteJob, &DatabaseJob::finished, this, [this, deleteJob, startTime](){
        if (deleteJob->error().type() != QSqlError::NoError) {
            qCWarning(dcLogEngine) << "Error deleting oldest log entries to keep size. Driver error:" << deleteJob->error().driverText() << "Database error:" << deleteJob->error().databaseText();
        }
        qCDebug(dcLogEngine()) << "Ran housekeeping on log database in" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms. (Deleted" << m_entryCount - (m_dbMaxSize - m_trimSize) << "entries)";
        m_entryCount = m_dbMaxSize - m_trimSize;

        emit logDatabaseUpdated();
    });

    qCDebug(dcLogEngine()) << "Scheduling housekeeping job.";
    enqueJob(deleteJob, true);
}

void LogEngine::enqueJob(DatabaseJob *job, bool priority)
{
    if (priority) {
        m_jobQueue.prepend(job);
    } else {
        m_jobQueue.append(job);
    }
    qCDebug(dcLogEngine()) << "Scheduled job at position" << (priority ? 0 : m_jobQueue.count() - 1) << "(" << m_jobQueue.count() << "jobs in the queue)";
    processQueue();
}

void LogEngine::processQueue()
{
    if (!m_initialized) {
        return;
    }

    if (m_jobQueue.isEmpty()) {
        emit jobsRunningChanged();
        return;
    }

    if (m_currentJob) {
        return;
    }

    emit jobsRunningChanged();

    if (m_dbMalformed) {
        qCWarning(dcLogEngine()) << "Database is malformed. Trying to recover...";
        m_db.close();
        rotate(m_db.databaseName());
        initDB(m_username, m_password);
        m_dbMalformed = false;
    }


    DatabaseJob *job = m_jobQueue.takeFirst();
    qCDebug(dcLogEngine()) << "Processing DB queue. (" << m_jobQueue.count() << "jobs left in queue," << m_entryCount << "entries in DB)";
    m_currentJob = job;

    QFuture<DatabaseJob*> future = QtConcurrent::run([job](){
        QSqlQuery query(job->m_db);
        query.prepare(job->m_queryString);

        foreach (const QVariant &value, job->m_bindValues) {
            query.addBindValue(value);
        }

        query.exec();

        job->m_error = query.lastError();
        job->m_executedQuery = query.executedQuery();

        if (!query.lastError().isValid()) {
            while (query.next()) {
                job->m_results.append(query.record());
            }
        }

       return job;
    });

    m_jobWatcher.setFuture(future);
}

void LogEngine::handleJobFinished()
{
    DatabaseJob *job = m_jobWatcher.result();
    job->finished();
    job->deleteLater();
    m_currentJob = nullptr;

    qCDebug(dcLogEngine()) << "DB job finished. (" << m_entryCount << "entries in DB)";
    processQueue();
}

void LogEngine::rotate(const QString &dbName)
{
    int index = 1;
    while (QFileInfo(QString("%1.%2").arg(dbName).arg(index)).exists()) {
        index++;
    }
    qCDebug(dcLogEngine()) << "Backing up old database file to" << QString("%1.%2").arg(dbName).arg(index);
    QFile f(dbName);
    if (!f.rename(QString("%1.%2").arg(dbName).arg(index))) {
        qCWarning(dcLogEngine()) << "Error backing up old database.";
    } else {
        qCDebug(dcLogEngine()) << "Successfully moved old database";
    }
}

bool LogEngine::migrateDatabaseVersion3to4()
{
    QSqlQuery renameQuery = m_db.exec("ALTER TABLE entries RENAME TO _entries_v3;");
    if (m_db.lastError().isValid()) {
        qCWarning(dcLogEngine) << "Error migrating database verion 3 -> 4 (renaming table). Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
        return false;
    }
    qCDebug(dcLogEngine()) << "Renamed entries table to entries_v3:" << m_db.lastError().text();
    m_db.close();
    m_db.open(m_username, m_password);

    QSqlQuery createQuery = m_db.exec("CREATE TABLE entries "
                                      "("
                                      "timestamp BIGINT,"
                                      "loggingLevel INT,"
                                      "sourceType INT,"
                                      "typeId VARCHAR(38),"
                                      "thingId VARCHAR(38),"
                                      "value VARCHAR(100),"
                                      "loggingEventType INT,"
                                      "active BOOL,"
                                      "errorCode INT,"
                                      "FOREIGN KEY(sourceType) REFERENCES sourceTypes(id),"
                                      "FOREIGN KEY(loggingEventType) REFERENCES loggingEventTypes(id)"
                                      ");");
    if (m_db.lastError().isValid()) {
        qCWarning(dcLogEngine) << "Error migrating database verion 3 -> 4 (creating new table). Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
        return false;
    }
    qCDebug(dcLogEngine()) << "Created new entries table:" << m_db.lastError().text();

    qCDebug(dcLogEngine()) << "Updating database version to" << DB_SCHEMA_VERSION;
    m_db.exec(QString("UPDATE metadata SET data = %1 WHERE `key` = 'version';").arg(DB_SCHEMA_VERSION));
    if (m_db.lastError().isValid()) {
        qCWarning(dcLogEngine) << "Error updating database verion 3 -> 4. Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
        return false;
    }

    qCDebug(dcLogEngine()) << "Migrated database schema from version 3 to 4.";

    return true;

}

void LogEngine::migrateEntries3to4()
{
    QString selectQuery = QString("SELECT * FROM _entries_v3;");

    DatabaseJob *job = new DatabaseJob(m_db, selectQuery);

    connect(job, &DatabaseJob::finished, this, [this, job](){

        if (job->error().type() != QSqlError::NoError) {
            qCWarning(dcLogEngine) << "Error fetching entries to migrate. Driver error:" << job->error().driverText() << "Database error:" << job->error().databaseText();
            m_dbMalformed = true;
            return;
        }

        if (job->results().isEmpty()) {
            qCDebug(dcLogEngine()) << "No items to migrate from schema 3 to 4 remaining.";
            finalizeMigration3To4();
            return;
        }

        int count = job->results().count();

        QSqlRecord result = job->results().first();
        QString encodedValue = result.value("value").toByteArray();
        QString decodedValue = LogValueTool::convertVariantToString(LogValueTool::deserializeValue(encodedValue));

        QString insertCall = QString("INSERT INTO entries (timestamp, loggingEventType, loggingLevel, sourceType, typeId, thingId, value, active, errorCode) values ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9');")
                .arg(result.value("timestamp").toLongLong() * 1000)
                .arg(result.value("loggingEventType").toInt())
                .arg(result.value("loggingLevel").toInt())
                .arg(result.value("sourceType").toInt())
                .arg(result.value("typeId").toString())
                .arg(result.value("deviceId").toString())
                .arg(decodedValue)
                .arg(result.value("active").toBool())
                .arg(result.value("errorCode").toInt());

        DatabaseJob *insertJob = new DatabaseJob(m_db, insertCall);
        connect(insertJob, &DatabaseJob::finished, this, [this, insertJob, count, result](){
            if (insertJob->error().type() != QSqlError::NoError) {
                qCWarning(dcLogEngine) << "Error fetching entries to migrate. Driver error:" << insertJob->error().driverText() << "Database error:" << insertJob->error().databaseText();
                m_dbMalformed = true;
                return;
            }

            QString deleteCall = QString("DELETE FROM _entries_v3 WHERE timestamp = '%1' AND loggingEventType = '%2' AND loggingLevel = '%3' AND sourceType = '%4' AND typeId = '%5' AND deviceId = '%6' AND value = '%7' AND active = '%8' AND errorCode = '%9';")
                    .arg(result.value("timestamp").toLongLong())
                    .arg(result.value("loggingEventType").toInt())
                    .arg(result.value("loggingLevel").toInt())
                    .arg(result.value("sourceType").toInt())
                    .arg(result.value("typeId").toString())
                    .arg(result.value("deviceId").toString())
                    .arg(result.value("value").toString())
                    .arg(result.value("active").toBool())
                    .arg(result.value("errorCode").toInt());

            DatabaseJob *deleteJob = new DatabaseJob(m_db, deleteCall);
            connect(deleteJob, &DatabaseJob::finished, this, [this, deleteJob, count, result](){
                if (deleteJob->error().type() != QSqlError::NoError) {
                    qCWarning(dcLogEngine) << "Error deleting old entry during migration. Driver error:" << deleteJob->error().driverText() << "Database error:" << deleteJob->error().databaseText();
                    finalizeMigration3To4();
                    return;
                }

                qCDebug(dcLogEngine()) << "Migrated log entry from version 3 to 4." << (count - 1) << "items left to migrate";
                if (count - 1 > 0) {
                    migrateEntries3to4();
                } else {
                    finalizeMigration3To4();
                }
            });
            enqueJob(deleteJob);
        });
        enqueJob(insertJob);
    });
    enqueJob(job);
}

void LogEngine::finalizeMigration3To4()
{
    qCDebug(dcLogEngine()) << "Finalizing migration of database version 3 to 4.";
    QString selectQuery = QString("DROP TABLE _entries_v3;");
    DatabaseJob *job = new DatabaseJob(m_db, selectQuery);
    enqueJob(job);
    connect(job, &DatabaseJob::finished, this, [job](){

        if (job->error().type() != QSqlError::NoError) {
            qCWarning(dcLogEngine) << "Error finalizing migration from 3 to 4 (drop entries_v3). Driver error:" << job->error().driverText() << "Database error:" << job->error().databaseText();
            return;
        }
    });
}

bool LogEngine::initDB(const QString &username, const QString &password)
{
    m_db.close();
    bool opened = m_db.open(username, password);
    if (!opened) {
        qCWarning(dcLogEngine()) << "Can't open Log DB. Init failed.";
        return false;
    }

    if (!m_db.tables().contains("metadata")) {
        qCDebug(dcLogEngine()) << "Empty Database. Setting up metadata...";
        m_db.exec("CREATE TABLE metadata (`key` VARCHAR(10), data VARCHAR(40));");
        if (m_db.lastError().isValid()) {
            qCWarning(dcLogEngine) << "Error initualizing database. Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
            return false;
        }
        m_db.exec(QString("INSERT INTO metadata (`key`, data) VALUES('version', '%1');").arg(DB_SCHEMA_VERSION));
    }

    QSqlQuery query = m_db.exec("SELECT data FROM metadata WHERE `key` = 'version';");
    if (query.next()) {
        int version = query.value("data").toInt();

        // Migration from 3 -> 4
        if (version == 3) {
            if (!migrateDatabaseVersion3to4()) {
                qCWarning(dcLogEngine()) << "Migration process failed.";
                return false;
            } else {
                // Successfully migrated
                version = 4;
            }
        }

        if (version != DB_SCHEMA_VERSION) {
            qCWarning(dcLogEngine) << "Log schema version not matching! Schema upgrade not implemented for this version change.";
            return false;
        } else {
            qCDebug(dcLogEngine) << QString("Log database schema version \"%1\" matches").arg(DB_SCHEMA_VERSION).toLatin1().data();
            // If there is still a deviceId column, schedule items to be migrated in the
            // background with low priority as this might take hours
            if (m_db.tables().contains("_entries_v3")) {
                migrateEntries3to4();
            }
        }
    } else {
        qCWarning(dcLogEngine) << "Broken log database. Version not found in metadata table.";
        return false;
    }

    if (!m_db.tables().contains("sourceTypes")) {
        m_db.exec("CREATE TABLE sourceTypes (id int, name varchar(20), PRIMARY KEY(id));");
        //qCDebug(dcLogEngine) << m_db.lastError().databaseText();
        QMetaEnum logTypes = Logging::staticMetaObject.enumerator(Logging::staticMetaObject.indexOfEnumerator("LoggingSource"));
        Q_ASSERT_X(logTypes.isValid(), "LogEngine", "Logging has no enum LoggingSource");
        for (int i = 0; i < logTypes.keyCount(); i++) {
            m_db.exec(QString("INSERT INTO sourceTypes (id, name) VALUES(%1, '%2');").arg(i).arg(logTypes.key(i)));
        }
    }

    if (!m_db.tables().contains("loggingEventTypes")) {
        m_db.exec("CREATE TABLE loggingEventTypes (id int, name varchar(40), PRIMARY KEY(id));");
        //qCDebug(dcLogEngine) << m_db.lastError().databaseText();
        QMetaEnum logTypes = Logging::staticMetaObject.enumerator(Logging::staticMetaObject.indexOfEnumerator("LoggingEventType"));
        Q_ASSERT_X(logTypes.isValid(), "LogEngine", "Logging has no enum LoggingEventType");
        for (int i = 0; i < logTypes.keyCount(); i++) {
            m_db.exec(QString("INSERT INTO loggingEventTypes (id, name) VALUES(%1, '%2');").arg(i).arg(logTypes.key(i)));
            if (m_db.lastError().isValid()) {
                qCWarning(dcLogEngine()) << "Failed to insert loggingEventTypes into DB. Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
            }
        }
    }

    if (!m_db.tables().contains("entries")) {
        qCDebug(dcLogEngine()) << "No \"entries\" table in database. Creating it.";
        m_db.exec("CREATE TABLE entries "
                  "("
                  "timestamp BIGINT,"
                  "loggingLevel INT,"
                  "sourceType INT,"
                  "typeId VARCHAR(38),"
                  "thingId VARCHAR(38),"
                  "value VARCHAR(100),"
                  "loggingEventType INT,"
                  "active BOOL,"
                  "errorCode INT,"
                  "FOREIGN KEY(sourceType) REFERENCES sourceTypes(id),"
                  "FOREIGN KEY(loggingEventType) REFERENCES loggingEventTypes(id)"
                  ");");

        if (m_db.lastError().isValid()) {
            qCWarning(dcLogEngine) << "Error creating log table in database. Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
            return false;
        }


    }

    qCDebug(dcLogEngine) << "Initialized logging DB successfully. (maximum DB size:" << m_dbMaxSize << ")";
    m_initialized = true;
    return true;
}

}
