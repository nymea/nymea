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

}

Logger *LogEngine::createLogger(const QString &name, const QStringList &tags, Types::LoggingType loggingType)
{
    return new Logger(this, name, tags, loggingType);
}

void LogEngine::finishFetchJob(LogFetchJob *job, const LogEntries &entries)
{
    job->finish(entries);
}



