#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <QDateTime>
#include <QVariant>
#include <QMetaObject>

class LogEntry
{
    Q_GADGET
    Q_PROPERTY(QDateTime timestamp READ timestamp)
    Q_PROPERTY(QString source READ source )
    Q_PROPERTY(QVariantMap values READ values)
public:
    LogEntry();
    LogEntry(const QDateTime &timestamp, const QString &source, const QVariantMap &values);

    QDateTime timestamp() const;
    QString source() const;
    QVariantMap values() const;

private:
    QDateTime m_timestamp;
    QString m_source;
    QVariantMap m_values;
};
Q_DECLARE_METATYPE(LogEntry)

class LogEntries: public QList<LogEntry>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    LogEntries();
    LogEntries(const QList<LogEntry> &other);
    LogEntries(std::initializer_list<LogEntry> args):QList(args) {}
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
Q_DECLARE_METATYPE(LogEntries)

#endif // LOGENTRY_H
