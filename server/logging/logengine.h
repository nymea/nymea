/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
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

#ifndef LOGENGINE_H
#define LOGENGINE_H

#include "logentry.h"
#include "types/event.h"
#include "types/action.h"
#include "rule.h"

#include <QObject>
#include <QSqlDatabase>

class LogEngine: public QObject
{
    Q_OBJECT
public:
    LogEngine(QObject *parent = 0);

    QList<LogEntry> logEntries() const;

signals:
    void logEntryAdded(const LogEntry &logEntry);

private:
    // Only GuhCore is allowed to log events.
    friend class GuhCore;
    void logSystemEvent(bool active, Logging::LoggingLevel level = Logging::LoggingLevelInfo);
    void logEvent(const Event &event);
    void logAction(const Action &action, Logging::LoggingLevel level = Logging::LoggingLevelInfo, int errorCode = 0);
    void logRuleTriggered(const Rule &rule);
    void logRuleActiveChanged(const Rule &rule);

private:
    void initDB();
    void appendLogEntry(const LogEntry &entry);

    QSqlDatabase m_db;
};

#endif
