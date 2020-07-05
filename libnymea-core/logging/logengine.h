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

#ifndef LOGENGINE_H
#define LOGENGINE_H

#include "logentry.h"
#include "logfilter.h"
#include "types/event.h"
#include "types/action.h"
#include "types/browseritemaction.h"
#include "types/browseraction.h"
#include "ruleengine/rule.h"

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QTimer>
#include <QFutureWatcher>

namespace nymeaserver {

class DatabaseJob;
class LogEntriesFetchJob;
class ThingsFetchJob;

class LogEngine: public QObject
{
    Q_OBJECT
public:
    LogEngine(const QString &driver, const QString &dbName, const QString &hostname = QString("127.0.0.1"), const QString &username = QString(), const QString &password = QString(), int maxDBSize = 50000, QObject *parent = nullptr);
    ~LogEngine();

    LogEntriesFetchJob *fetchLogEntries(const LogFilter &filter = LogFilter());
    ThingsFetchJob *fetchThings();

    bool jobsRunning() const;

    void setMaxLogEntries(int maxLogEntries, int trimSize);
    void clearDatabase();

    void logSystemEvent(const QDateTime &dateTime, bool active, Logging::LoggingLevel level = Logging::LoggingLevelInfo);
    void logEvent(const Event &event);
    void logAction(const Action &action, Logging::LoggingLevel level = Logging::LoggingLevelInfo, int errorCode = 0);
    void logBrowserAction(const BrowserAction &browserAction, Logging::LoggingLevel level = Logging::LoggingLevelInfo, int errorCode = 0);
    void logBrowserItemAction(const BrowserItemAction &browserItemAction, Logging::LoggingLevel level = Logging::LoggingLevelInfo, int errorCode = 0);
    void logRuleTriggered(const Rule &rule);
    void logRuleActiveChanged(const Rule &rule);
    void logRuleEnabledChanged(const Rule &rule, const bool &enabled);
    void logRuleActionsExecuted(const Rule &rule);
    void logRuleExitActionsExecuted(const Rule &rule);
    void removeThingLogs(const ThingId &thingId);
    void removeRuleLogs(const RuleId &ruleId);

signals:
    void logEntryAdded(const LogEntry &logEntry);
    void logDatabaseUpdated();

    void jobsRunningChanged();

private:
    bool initDB(const QString &username, const QString &password);
    void appendLogEntry(const LogEntry &entry);
    void rotate(const QString &dbName);

    bool migrateDatabaseVersion3to4();
    void migrateEntries3to4();
    void finalizeMigration3To4();

private slots:
    void checkDBSize();
    void trim();

    void enqueJob(DatabaseJob *job, bool priority = false);
    void processQueue();
    void handleJobFinished();

private:
    QSqlDatabase m_db;
    QString m_username;
    QString m_password;
    int m_dbMaxSize;
    int m_trimSize;
    int m_entryCount = 0;
    bool m_initialized = false;
    bool m_dbMalformed = false;

    // When maxQueueLength is exceeded, jobs will be flagged and discarded if this source logs more events
    int m_maxQueueLength;
    QHash<QString, QList<DatabaseJob*>> m_flaggedJobs;

    QList<DatabaseJob*> m_jobQueue;
    DatabaseJob *m_currentJob = nullptr;
    QFutureWatcher<DatabaseJob*> m_jobWatcher;
};

class DatabaseJob: public QObject
{
    Q_OBJECT
public:
    DatabaseJob(const QSqlDatabase &db, const QString &queryString, const QVariantList &bindValues = QVariantList()):
        m_db(db),
        m_queryString(queryString),
        m_bindValues(bindValues)
    {
    }

    QString executedQuery() const { return m_executedQuery; }
    QSqlError error() const { return m_error; }
    QList<QSqlRecord> results() const { return m_results; }

signals:
    void finished();

private:
    QSqlDatabase m_db;
    QString m_queryString;
    QVariantList m_bindValues;

    QString m_executedQuery;
    QSqlError m_error;
    QList<QSqlRecord> m_results;

    friend class LogEngine;
};

class LogEntriesFetchJob: public QObject
{
    Q_OBJECT
public:
    LogEntriesFetchJob(QObject *parent): QObject(parent) {}
    QList<LogEntry> results() { return m_results; }
signals:
    void finished();
private:
    QList<LogEntry> m_results;
    friend class LogEngine;
};

class ThingsFetchJob: public QObject
{
    Q_OBJECT
public:
    ThingsFetchJob(QObject *parent): QObject(parent) {}
    QList<ThingId> results() { return m_results; }
signals:
    void finished();
private:
    QList<ThingId> m_results;
    friend class LogEngine;
};

}

#endif
