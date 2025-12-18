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

#include "logengine.h"
#include "logger.h"

#include "loggingcategories.h"
NYMEA_LOGGING_CATEGORY(dcLogEngine, "LogEngine")

LogFetchJob::LogFetchJob(QObject *parent)
    : QObject(parent)
{}

LogEntries LogFetchJob::entries() const
{
    return m_entries;
}

void LogFetchJob::finish(const LogEntries &entries)
{
    m_entries = entries;
    emit finished(entries);
    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection, Q_ARG(LogEntries, entries));
}

LogEngine::LogEngine(QObject *parent)
    : QObject{parent}
{
    qRegisterMetaType<LogEntry>();
}

Logger *LogEngine::createLogger(const QString &name, const QStringList &tags, Types::LoggingType loggingType)
{
    return new Logger(this, name, tags, loggingType);
}

void LogEngine::finishFetchJob(LogFetchJob *job, const LogEntries &entries)
{
    job->finish(entries);
}
