#include "logger.h"
#include "logengine.h"
#include <QLoggingCategory>

Logger::Logger(LogEngine *engine, const QString &name, const QStringList &tagNames, Types::LoggingType loggingType):
    m_engine(engine),
    m_name(name),
    m_tagNames(tagNames),
    m_loggingType(loggingType)
{

}

QString Logger::name() const
{
    return m_name;
}

QStringList Logger::tagNames() const
{
    return m_tagNames;
}

Types::LoggingType Logger::loggingType() const
{
    return m_loggingType;
}

void Logger::log(const QStringList &tags, const QVariantMap &values)
{
    m_engine->logEvent(this, tags, values);
}
