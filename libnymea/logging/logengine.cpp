/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2024, nymea GmbH
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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "logengine.h"
#include "logger.h"

#include "loggingcategories.h"
NYMEA_LOGGING_CATEGORY(dcLogEngine, "LogEngine")

LogFetchJob::LogFetchJob(QObject *parent): QObject(parent)
{

}

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



