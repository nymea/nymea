#include "logentry.h"

LogEntry::LogEntry()
{

}

LogEntry::LogEntry(const QDateTime &timestamp, const QString &source, const QVariantMap &values):
    m_timestamp(timestamp),
    m_source(source),
    m_values(values)
{

}

QDateTime LogEntry::timestamp() const
{
    return m_timestamp;
}

QString LogEntry::source() const
{
    return m_source;
}

QVariantMap LogEntry::values() const
{
    return m_values;
}

LogEntries::LogEntries(): QList<LogEntry>()
{

}

LogEntries::LogEntries(const QList<LogEntry> &other):
    QList<LogEntry>(other)
{

}

QVariant LogEntries::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void LogEntries::put(const QVariant &variant)
{
    append(variant.value<LogEntry>());
}
