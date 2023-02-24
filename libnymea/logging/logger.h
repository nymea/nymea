#ifndef LOGGER_H
#define LOGGER_H

#include <QStringList>
#include <QVariant>
#include "typeutils.h"

class LogEngine;

class Logger
{
public:

    QString name() const;
    QStringList tagNames() const;
    Types::LoggingType loggingType() const;

    void log(const QStringList &tags, const QVariantMap &values);

private:
    friend class LogEngine;
    Logger(LogEngine *engine, const QString &name, const QStringList &tagNames, Types::LoggingType loggingType);
    LogEngine *m_engine = nullptr;
    QString m_name;
    QStringList m_tagNames;
    Types::LoggingType m_loggingType = Types::LoggingTypeDiscrete;
};

Q_DECLARE_METATYPE(Logger*)

#endif // LOGGER_H
