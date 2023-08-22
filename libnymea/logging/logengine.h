#ifndef LOGENGINE_H
#define LOGENGINE_H

#include "logentry.h"
#include "types/param.h"
#include "typeutils.h"
#include "logger.h"

#include <QObject>
#include <QDateTime>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dcLogEngine)

class LogFetchJob: public QObject
{
    Q_OBJECT
public:
    LogFetchJob(QObject *parent = nullptr);

    LogEntries entries() const;
signals:
    void finished(const LogEntries &entries);

private:
    friend class LogEngine;
    void finish(const LogEntries &entries);

    LogEntries m_entries;
};


class LogEngine : public QObject
{
    Q_OBJECT
public:
    explicit LogEngine(QObject *parent = nullptr);
    virtual ~LogEngine() = default;

    // LogEngine keeps ownership:
    // * name: must be unique (table name)
    // * tags: optional, indexed column names for faster queries
    //   Values for tagged columns must be non-null.
    // * loggingType defines wheter a source will log discrete values or should be resampled
    // * sampleColumn is required for LoggingTypeSampled. Values in the given column will be sampled
    virtual Logger *registerLogSource(const QString &name, const QStringList &tags = QStringList(), Types::LoggingType loggingType = Types::LoggingTypeDiscrete, const QString &sampleColumn = QString()) = 0;

    // Unregistering will discard all related entries from the log database.
    // It is not required to unregister or clean up a log source on application shutdown as the engine keeps ownership.
    virtual void unregisterLogSource(const QString &name) = 0;

    virtual LogFetchJob *fetchLogEntries(const QStringList &sources, const QStringList &columns = QStringList(), const QDateTime &from = QDateTime(), const QDateTime &to = QDateTime(), const QVariantMap &filer = QVariantMap(), Types::SampleRate sampleRate = Types::SampleRateAny, Qt::SortOrder sortOrder = Qt::AscendingOrder, int offset = 0, int limit = 0) = 0;

    virtual bool jobsRunning() const = 0;
    virtual void clear(const QString &source) = 0;

    virtual void enable() = 0;
    virtual void disable() = 0;

signals:
    void logEntryAdded(const LogEntry &entry);

protected:
    Logger *createLogger(const QString &name, const QStringList &tags, Types::LoggingType loggingType);

    void finishFetchJob(LogFetchJob *job, const LogEntries &entries);

private:
    friend class Logger;
    virtual void logEvent(Logger *logger, const QStringList &tags, const QVariantMap &values) = 0;

};

#endif // LOGENGINE_H
