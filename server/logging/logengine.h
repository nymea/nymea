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

#ifndef LOGENGINE_H
#define LOGENGINE_H

#include "logentry.h"
#include "logfilter.h"
#include "types/event.h"
#include "types/action.h"
#include "rule.h"

#include <QObject>
#include <QSqlDatabase>

namespace guhserver {

class LogEngine: public QObject
{
    Q_OBJECT
public:
    LogEngine(QObject *parent = 0);
    ~LogEngine();

    QList<LogEntry> logEntries(const LogFilter &filter = LogFilter()) const;

    void clearDatabase();

signals:
    void logEntryAdded(const LogEntry &logEntry);
    void logDatabaseUpdated();

private:
    QSqlDatabase m_db;
    int m_dbMaxSize;

    void initDB();
    void appendLogEntry(const LogEntry &entry);
    void checkDBSize();

private:
    // Only GuhCore is allowed to log events.
    friend class GuhCore;
    void logSystemEvent(bool active, Logging::LoggingLevel level = Logging::LoggingLevelInfo);
    void logEvent(const Event &event);
    void logAction(const Action &action, Logging::LoggingLevel level = Logging::LoggingLevelInfo, int errorCode = 0);
    void logRuleTriggered(const Rule &rule);
    void logRuleActiveChanged(const Rule &rule);
    void removeDeviceLogs(const DeviceId &deviceId);
    void removeRuleLogs(const RuleId &ruleId);

};

}

#endif
