/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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
#include <QTimer>
#include <QFutureWatcher>

namespace nymeaserver {

class DatabaseJob;
class LogEngineFetchJob;

class LogEngine: public QObject
{
    Q_OBJECT
public:
    LogEngine(const QString &driver, const QString &dbName, const QString &hostname = QString("127.0.0.1"), const QString &username = QString(), const QString &password = QString(), int maxDBSize = 50000, QObject *parent = nullptr);
    ~LogEngine();

    LogEngineFetchJob *logEntries(const LogFilter &filter = LogFilter());

    void setMaxLogEntries(int maxLogEntries, int overflow);
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
    void removeDeviceLogs(const DeviceId &deviceId);
    void removeRuleLogs(const RuleId &ruleId);

signals:
    void logEntryAdded(const LogEntry &logEntry);
    void logDatabaseUpdated();

private:
    bool initDB(const QString &username, const QString &password);
    void appendLogEntry(const LogEntry &entry);
    void rotate(const QString &dbName);


    bool migrateDatabaseVersion2to3();

private slots:
    void checkDBSize();

    void enqueJob(DatabaseJob *job);
    void processQueue();
    void handleJobFinished();

private:
    QSqlDatabase m_db;
    QString m_username;
    QString m_password;
    int m_dbMaxSize;
    int m_overflow;
    bool m_trimWarningPrinted = false;
    int m_entryCount = 0;
    bool m_dbMalformed = false;

    QList<DatabaseJob*> m_jobQueue;
    QFutureWatcher<DatabaseJob*> m_jobWatcher;
};

class DatabaseJob: public QObject
{
    Q_OBJECT
public:
    DatabaseJob(const QString &queryString, const QSqlDatabase &db) {
        m_query = QSqlQuery(db);
        m_query.prepare(queryString);
    }

    // IMPORTANT: Make sure it only prepare()d but not executed
    // QSQlQuery(QString, QSqlDatabase) implicitly executes!
    DatabaseJob(const QSqlQuery &query): m_query(query) {}

    QSqlQuery query() const { return m_query; }

signals:
    void finished();

private:
    QSqlQuery m_query;
};

class LogEngineFetchJob: public QObject
{
    Q_OBJECT
public:
    LogEngineFetchJob(QObject *parent): QObject(parent) {}

    QList<LogEntry> results() { return m_results; }

signals:
    void finished();

private:
    void addResult(const LogEntry &entry) { m_results.append(entry); }
    QList<LogEntry> m_results;

    friend class LogEngine;

};
}

#endif
