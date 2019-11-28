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
#include <QSqlError>
#include <QSqlRecord>
#include <QTimer>
#include <QFutureWatcher>

namespace nymeaserver {

class DatabaseJob;
class LogEntriesFetchJob;
class DevicesFetchJob;

class LogEngine: public QObject
{
    Q_OBJECT
public:
    LogEngine(const QString &driver, const QString &dbName, const QString &hostname = QString("127.0.0.1"), const QString &username = QString(), const QString &password = QString(), int maxDBSize = 50000, QObject *parent = nullptr);
    ~LogEngine();

    LogEntriesFetchJob *fetchLogEntries(const LogFilter &filter = LogFilter());
    DevicesFetchJob *fetchDevices();

    bool jobsRunning() const;

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

    void jobsRunningChanged();

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
    DatabaseJob *m_currentJob = nullptr;
    QFutureWatcher<DatabaseJob*> m_jobWatcher;
};

class DatabaseJob: public QObject
{
    Q_OBJECT
public:
    DatabaseJob(const QSqlDatabase &db, const QString &queryString, const QStringList &bindValues = QStringList()):
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
    QStringList m_bindValues;

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

class DevicesFetchJob: public QObject
{
    Q_OBJECT
public:
    DevicesFetchJob(QObject *parent): QObject(parent) {}
    QList<DeviceId> results() { return m_results; }
signals:
    void finished();
private:
    QList<DeviceId> m_results;
    friend class LogEngine;
};

}

#endif
