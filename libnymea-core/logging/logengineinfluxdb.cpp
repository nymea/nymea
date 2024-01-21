/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2023, nymea GmbH
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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "logengineinfluxdb.h"

#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QRegularExpression>

LogEngineInfluxDB::LogEngineInfluxDB(const QString &host, const QString &dbName, const QString &username, const QString &password, QObject *parent)
    : LogEngine{parent},
      m_host(host),
      m_dbName(dbName),
      m_username(username),
      m_password(password)
{
    m_nam = new QNetworkAccessManager(this);

    m_reinitTimer.setInterval(5000);
    m_reinitTimer.setSingleShot(true);
    connect(&m_reinitTimer, &QTimer::timeout, this, &LogEngineInfluxDB::initDB);
}

LogEngineInfluxDB::~LogEngineInfluxDB()
{
    if (m_initStatus == InitStatusStarting) {
        m_initStatus = InitStatusFailure;
    }
    if (jobsRunning()) {
        qCInfo(dcLogEngine()) << "Waiting for" << (m_initQueryQueue.count() + m_queryQueue.count() + m_writeQueue.count()) << "jobs to finish... Init status:" << m_initStatus;
    }
    while (jobsRunning()) {
//        qCDebug(dcLogEngine()) << "Waiting for logs to finish processing." << m_writeQueue.count() << "jobs pending...";
        processQueues();
        qApp->processEvents();
    }
}

Logger *LogEngineInfluxDB::registerLogSource(const QString &name, const QStringList &tagNames, Types::LoggingType loggingType, const QString &sampleColumn)
{
    if (m_loggers.contains(name)) {
        qCCritical(dcLogEngine()) << "Log source" << name << "already registerd. Not registering a second time.";
        return nullptr;
    }

//    qCDebug(dcLogEngine()) << "Registering log source" << name << "with tags" << tagNames;

    Logger *logger = createLogger(name, tagNames, loggingType);
    m_loggers.insert(name, logger);

    if (loggingType == Types::LoggingTypeSampled) {
        qCDebug(dcLogEngine()) << "Setting up log sampling on" << sampleColumn;

        if (sampleColumn.isEmpty()) {
            qCCritical(dcLogEngine()) << "Sample type != None but no sample column given. Unable to create samples for" << name;

        } else {
            QStringList columns;
            columns.append(QString("MIN(\"%1\") AS min_%1").arg(sampleColumn));
            columns.append(QString("MAX(\"%1\") AS max_%1").arg(sampleColumn));
            columns.append(QString("MEAN(\"%1\") AS %1").arg(sampleColumn));
            QString target = columns.join(", ");

            QueryJob *minutesJob = query(QString("CREATE CONTINUOUS QUERY \"minutes-%1\" "
                                          "ON \"nymea\" "
                                          "BEGIN "
                                            "SELECT %2 "
                                            "INTO minutes.\"%1\" "
                                            "FROM live.\"%1\" "
                                            "GROUP BY time(1m) "
                                            "fill(previous) "
                                          "END").arg(name).arg(target), true);
            connect(minutesJob, &QueryJob::finished, this, [=](QNetworkReply::NetworkError status, const QVariantList &response){
                if (status == QNetworkReply::NoError) {
                    qCDebug(dcLogEngine()) << "Created minute based continuous query for" << name << qUtf8Printable(QJsonDocument::fromVariant(response).toJson());
                } else {
                    qCWarning(dcLogEngine()) << "Unable to create minute based continuous query for" << name << qUtf8Printable(QJsonDocument::fromVariant(response).toJson());
                }
            });
            QueryJob *hoursJob = query(QString("CREATE CONTINUOUS QUERY \"hours-%1\" "
                                          "ON \"nymea\" "
                                          "BEGIN "
                                            "SELECT %2 "
                                            "INTO hours.\"%1\" "
                                            "FROM minutes.\"%1\" "
                                            "GROUP BY time(1h) "
                                            "fill(previous) "
                                          "END").arg(name).arg(target), true);
            connect(hoursJob, &QueryJob::finished, this, [=](QNetworkReply::NetworkError status, const QVariantList &response){
                if (status == QNetworkReply::NoError) {
                    qCDebug(dcLogEngine()) << "Created hour based continuous query for" << name << qUtf8Printable(QJsonDocument::fromVariant(response).toJson());
                } else {
                    qCWarning(dcLogEngine()) << "Unable to create hour based continuous query for" << name << qUtf8Printable(QJsonDocument::fromVariant(response).toJson());
                }
            });
            QueryJob *daysJob = query(QString("CREATE CONTINUOUS QUERY \"days-%1\" "
                                          "ON \"nymea\" "
                                          "BEGIN "
                                            "SELECT %2 "
                                            "INTO days.\"%1\" "
                                            "FROM hours.\"%1\" "
                                            "GROUP BY time(24h) "
                                            "fill(previous) "
                                          "END").arg(name).arg(target), true);
            connect(daysJob, &QueryJob::finished, this, [=](QNetworkReply::NetworkError status, const QVariantList &response){
                if (status == QNetworkReply::NoError) {
                    qCDebug(dcLogEngine()) << "Created day based continuous query for" << name << qUtf8Printable(QJsonDocument::fromVariant(response).toJson());
                } else {
                    qCWarning(dcLogEngine()) << "Unable to create days based continuous query for" << name << qUtf8Printable(QJsonDocument::fromVariant(response).toJson());
                }
            });

        }
    }

    return logger;
}

void LogEngineInfluxDB::unregisterLogSource(const QString &name)
{
    Logger *logger = m_loggers.take(name);
    if (!logger) {
        qCWarning(dcLogEngine()) << "Log source" << name << "unknown. Cannot unregister.";
        return;
    }

    QString queryString = QString("DROP MEASUREMENT \"%1\"").arg(name);
    if (m_initStatus == InitStatusOK)
        qCDebug(dcLogEngine()) << "Removing log entries:" << queryString;

    QueryJob *job = query(queryString);
    connect(job, &QueryJob::finished, this, [name](bool success){
        if (success) {
            qCDebug(dcLogEngine()) << "Removed log entries for source" << name;
        } else {
            qCWarning(dcLogEngine()) << "Removing log entries for source" << name << "failed";
        }
    });
}

void LogEngineInfluxDB::logEvent(Logger *logger, const QStringList &tags, const QVariantMap &values)
{
    QString measurement = logger->name();
    QStringList tagsList;
    QStringList fieldsList;
    QDateTime timestamp = QDateTime::currentDateTime();

    QVariantMap combinedValues;

    for (int i = 0; i < qMin(logger->tagNames().count(), tags.count()); i++) {
        tagsList.append(QString("%1=%2").arg(logger->tagNames().at(i)).arg(tags.at(i)));
        combinedValues.insert(logger->tagNames().at(i), tags.at(i));
    }
    foreach (const QString &key, values.keys()) {
        combinedValues.insert(key, values.value(key));
    }

    foreach (const QString &name, values.keys()) {
        QVariant value = values.value(name);
        switch (value.userType()) {
        case QMetaType::QString:
        case QMetaType::QByteArray:
            fieldsList.append(QString("%1=\"%2\"").arg(name).arg(QString(value.toByteArray().toPercentEncoding())));
            break;
        case QMetaType::QUuid:
            fieldsList.append(QString("%1=\"%2\"").arg(name).arg(value.toString()));
            break;
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
            fieldsList.append(QString("%1=%2i").arg(name).arg(value.toString()));
            break;
        default:
            fieldsList.append(QString("%1=%2").arg(name).arg(value.toString()));
        }
    }

    tagsList.prepend(measurement);
    QString measurementAndTags = tagsList.join(',').trimmed();
    QString fieldsString = fieldsList.join(',').trimmed();
    if (!fieldsList.isEmpty()) {
        fieldsString.append(' ');
    }

    QString data = measurementAndTags + " " + fieldsString + QString::number(timestamp.toMSecsSinceEpoch());

    QString retentionPolicy = logger->loggingType() == Types::LoggingTypeSampled ? "live" : "discrete";

    LogEntry entry(timestamp, logger->name(), combinedValues);

    QueueEntry queueEntry;
    queueEntry.request = createWriteRequest(retentionPolicy);
    queueEntry.request.setHeader(QNetworkRequest::ContentTypeHeader, "application/text");
    queueEntry.data = data.toUtf8();
    queueEntry.entry = entry;

    m_writeQueue.append(queueEntry);
    processQueues();
}

void LogEngineInfluxDB::processQueues()
{
    if (m_initStatus == InitStatusFailure || m_initStatus == InitStatusDisabled) {
        m_writeQueue.clear();
        qDeleteAll(m_queryQueue);
        m_queryQueue.clear();
        qDeleteAll(m_initQueryQueue);
        m_initQueryQueue.clear();
        return;
    }

//    qCDebug(dcLogEngine()) << "Processing queue:" << m_initStatus << "init count:" << m_initQueryQueue.count() << "query count:" << m_queryQueue.count() << "write count:" << m_writeQueue.count();

    if (!m_currentInitQuery && !m_initQueryQueue.isEmpty()) {
        QueryJob *job = m_initQueryQueue.takeFirst();
        QNetworkReply *reply;
        qCDebug(dcLogEngine()) << "Sending init query to influx" << job->m_request.url();

        if (job->m_post) {
            reply = m_nam->post(job->m_request, QByteArray());
        } else {
            reply = m_nam->get(job->m_request);
        }

        m_currentInitQuery = job;

        connect(reply, &QNetworkReply::finished, job, [=](){
            m_currentInitQuery = nullptr;
            qCDebug(dcLogEngine()) << "Init query job finished";
            reply->deleteLater();
            if (reply->error() == QNetworkReply::ProtocolInvalidOperationError) {
                qCWarning(dcLogEngine()) << "Influx DB protocol error:" << reply->readAll();
                job->finish(reply->error());
                return;
            }

            if (reply->error() != QNetworkReply::NoError) {
                qCWarning(dcLogEngine()) << "Error in influxdb communication:" << reply->error() << reply->errorString() << "for query:" << job->m_request.url().toString();
                job->finish(reply->error());
                return;
            }
            QByteArray data = reply->readAll();
            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
            if (error.error != QJsonParseError::NoError) {
                qCWarning(dcLogEngine) << "Unable to process response from influxdb:" << error.errorString() << qUtf8Printable(data);
                job->finish(QNetworkReply::ProtocolFailure);
                return;
            }

            job->finish(QNetworkReply::NoError, jsonDoc.toVariant().toMap().value("results").toList());
        });
    }

    if (m_initStatus != InitStatusOK ) {
        return;
    }

    // process query queue
    if (!m_currentQuery && !m_queryQueue.isEmpty()) {
        QueryJob *job = m_queryQueue.takeFirst();
        QNetworkReply *reply;
        qCDebug(dcLogEngine()) << "Sending query to influx" << job->m_request.url();

        if (job->m_post) {
            reply = m_nam->post(job->m_request, QByteArray());
        } else {
            reply = m_nam->get(job->m_request);
        }

        m_currentQuery = job;

        connect(reply, &QNetworkReply::finished, job, [=](){
            qCDebug(dcLogEngine()) << "Query finished";
            m_currentQuery = nullptr;
            reply->deleteLater();
            if (reply->error() == QNetworkReply::ProtocolInvalidOperationError) {
                qCWarning(dcLogEngine()) << "Influx DB protocol error:" << reply->readAll();
                job->finish(reply->error());
                processQueues();
                return;
            }

            if (reply->error() != QNetworkReply::NoError) {
                qCWarning(dcLogEngine()) << "Error in influxdb communication:" << reply->error() << reply->errorString();
                job->finish(reply->error());
                processQueues();
                return;
            }
            QByteArray data = reply->readAll();
            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
            if (error.error != QJsonParseError::NoError) {
                qCWarning(dcLogEngine) << "Unable to process response from influxdb:" << error.errorString() << qUtf8Printable(data);
                job->finish(QNetworkReply::ProtocolFailure);
                processQueues();
                return;
            }
    //        qCDebug(dcLogEngine()) << "Reply" << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented));

            job->finish(QNetworkReply::NoError, jsonDoc.toVariant().toMap().value("results").toList());

            processQueues();
        });
        return;
    }

    // Process write queue
    if (!m_writeQueue.isEmpty() && !m_currentWriteReply) {
        QueueEntry entry = m_writeQueue.takeFirst();
        QNetworkRequest request = entry.request;
        QByteArray data = entry.data;
        LogEntry logEntry = entry.entry;

        qCDebug(dcLogEngine()) << "Sending log write event to influx" << request.url().toString() << data;
        QNetworkReply *reply = m_nam->post(request, data);
        qCDebug(dcLogEngine()) << "Started:" << reply->isRunning() << reply->isFinished();
        m_currentWriteReply = reply;

        connect(reply, &QNetworkReply::finished, this, [=](){
            m_currentWriteReply = nullptr;
            reply->deleteLater();

            if (reply->error() != QNetworkReply::NoError) {
                qCWarning(dcLogEngine()) << "Unable to connect to influxdb. Cannot log events." << reply->error() << reply->readAll();
                processQueues();
                return;
            }

            emit logEntryAdded(logEntry);

            QByteArray result = reply->readAll();
            if (!result.isEmpty()) {
                qCDebug(dcLogEngine()) << "Log reply:" << result;
            }
            processQueues();
        });
    }
}

LogFetchJob *LogEngineInfluxDB::fetchLogEntries(const QStringList &sources, const QStringList &columns, const QDateTime &startTime, const QDateTime &endTime, const QVariantMap &filter, Types::SampleRate sampleRate, Qt::SortOrder sortOrder, int offset, int limit)
{
    LogFetchJob *job = new LogFetchJob(this);

    // FIXME: injection attacks possible?
    QString what = "*";
    if (sampleRate == Types::SampleRateAny) {
        if (!columns.isEmpty()) {
            what = columns.join(", ");
        }
    } else {
        if (!columns.isEmpty()) {
            QStringList meanColumns;
            foreach (const QString &column, columns) {
                meanColumns.append(QString("MEAN(%1)").arg(column));
            }
            what = meanColumns.join(", ");
        } else {
            what = "MEAN(*)";
        }
    }

    QStringList escapedSourced;
    foreach (const QString &source, sources) {

        QString retentionPolicy;
        switch (sampleRate) {
        case Types::SampleRate1Min:
        case Types::SampleRate15Mins:
            retentionPolicy = "minutes";
            break;
        case Types::SampleRate1Hour:
        case Types::SampleRate3Hours:
            retentionPolicy = "hours";
            break;
        case Types::SampleRate1Day:
        case Types::SampleRate1Week:
        case Types::SampleRate1Month:
        case Types::SampleRate1Year:
            retentionPolicy = "days";
            break;
        case Types::SampleRateAny:
            if (m_loggers.contains(source) && m_loggers.value(source)->loggingType() == Types::LoggingTypeSampled) {
                retentionPolicy = "live";
            } else {
                retentionPolicy = "discrete";
            }
            break;
        }

        escapedSourced.append(QString("%1.\"%2\"").arg(retentionPolicy).arg(source));
    }
    QString query = QString("SELECT %1 FROM %2").arg(what).arg(escapedSourced.join(", "));
    QStringList parts;

    if (!startTime.isNull()) {
        parts.append(QString("time >= %1ms").arg(startTime.toMSecsSinceEpoch()));
    }
    if (!endTime.isNull()) {
        parts.append(QString("time <= %1ms").arg(endTime.toMSecsSinceEpoch()));
    }
    if (!filter.isEmpty()) {
        foreach (const QString &column, filter.keys()) {
            parts.append(QString("%1 = '%2'").arg(column).arg(filter.value(column).toString()));
        }
    }
    if (parts.count() > 0) {
        query.append(" WHERE ");
        query.append(parts.join(" AND "));
    }

    if (sampleRate != Types::SampleRateAny) {
        // When resampling, we need to go from oldest to newest to properly "fill(previous)"
        query.append(QString(" GROUP BY time(%1m) fill(previous)").arg(sampleRate));
    }

    if (sortOrder == Qt::AscendingOrder) {
        query.append(" ORDER BY time ASC");
    } else {
        query.append(" ORDER BY time DESC");
    }

    if (limit > 0) {
        query.append(QString(" LIMIT %1").arg(limit));
    }
    if (offset > 0) {
        query.append(QString(" OFFSET %1").arg(offset));
    }

    qCDebug(dcLogEngine()) << "Running query:" << query;
    qCDebug(dcLogEngine()) << "start" << startTime << "end" << endTime;
    QNetworkRequest request = createQueryRequest(query);
    qCDebug(dcLogEngine()) << "Request:" << request.url() << filter;
    QNetworkReply *reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, this, [=](){
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(dcLogEngine()) << "Unable to obtain entries from influxdb" << reply->error() << reply->readAll();
            finishFetchJob(job, QList<LogEntry>());
            return;
        }
        QByteArray data = reply->readAll();
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcLogEngine) << "Unable to process response from influxdb:" << error.errorString() << qUtf8Printable(data);
            finishFetchJob(job, QList<LogEntry>());
            return;
        }
        qCDebug(dcLogEngine()) << "Reply" << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented));
        QList<LogEntry> entries;
        foreach (const QVariant &resultsVariant, jsonDoc.toVariant().toMap().value("results").toList()) {
            QVariantMap resultMap = resultsVariant.toMap();
            foreach (const QVariant &seriesVariant, resultMap.value("series").toList()) {
                QVariantMap seriesMap = seriesVariant.toMap();
                QStringList columns = seriesMap.value("columns").toStringList();
                foreach (const QVariant &valueVariant, seriesMap.value("values").toList()) {
                    QVariantMap valuesMap;
                    QVariantList values = valueVariant.toList();
                    QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(values.first().toULongLong());
                    for (int i = 1; i < columns.count(); i++) {
                        QString column = columns.at(i);
                        if (sampleRate != Types::SampleRateAny) {
                            column.remove(QRegularExpression("^mean_"));
                        }
                        QVariant value = values.at(i);
                        if (value.userType() == QMetaType::QString || value.userType() == QMetaType::QByteArray) {
                            valuesMap.insert(column, QByteArray::fromPercentEncoding(value.toByteArray()));
                        } else {
                            valuesMap.insert(column, values.at(i));
                        }
                    }
                    LogEntry entry(timestamp, seriesMap.value("name").toString(), valuesMap);
                    entries.append(entry);
                }
            }
        }
        finishFetchJob(job, entries);
    });

    return job;
}

bool LogEngineInfluxDB::jobsRunning() const
{
//    qCDebug(dcLogEngine()) << "Jobs running:" << m_initStatus << m_writeQueue.count() << m_initQueryQueue.count() << m_queryQueue.count() << m_currentWriteReply;
    return m_currentInitQuery
            || !m_initQueryQueue.isEmpty()
            || m_currentQuery
            || !m_queryQueue.isEmpty()
            || m_currentWriteReply
            || !m_writeQueue.isEmpty();
}

void LogEngineInfluxDB::clear(const QString &source)
{
    qCDebug(dcLogEngine()) << "Clearing entries for source:" << source;
    QueryJob *job = query(QString("DROP MEASUREMENT \"%1\"").arg(source));
    connect(job, &QueryJob::finished, this, [=](QNetworkReply::NetworkError status, const QVariantList &results){
        if (status != QNetworkReply::NoError) {
            qCWarning(dcLogEngine()) << "Unable to clear log entries for" << source << ":" << qUtf8Printable(QJsonDocument::fromVariant(results).toJson());
        }
    });
}

void LogEngineInfluxDB::enable()
{
    qCInfo(dcLogEngine()) << "Enabling influx DB log engine";
    initDB();
}

void LogEngineInfluxDB::disable()
{
    qCInfo(dcLogEngine()) << "Disabling influx DB log engine";
    m_initStatus = InitStatusDisabled;
    m_reinitTimer.stop();

    // Cleanup queues
    processQueues();
}

void LogEngineInfluxDB::initDB()
{
    m_initStatus = InitStatusStarting;
    qCInfo(dcLogEngine()) << "Initializing influx DB connection";
    createDB();
}

void LogEngineInfluxDB::createDB()
{
    QueryJob *job = query("SHOW DATABASES", false, true);
    connect(job, &QueryJob::finished, this, [=](QNetworkReply::NetworkError status, const QVariantList &results){
        if (status != QNetworkReply::NoError) {
            if (status == QNetworkReply::ConnectionRefusedError) {
                // Influx not up yet? trying again in 5 secs...
                if (m_initStatus != InitStatusDisabled) {
                    qCInfo(dcLogEngine) << "Failed to connect to influx... retrying in 5 seconds...";
                    m_reinitTimer.start();
                }
                return;
            }
            qCCritical(dcLogEngine()) << "Unable to connect to InfluxDB";
            m_initStatus = InitStatusFailure;
            return;
        }

        if (results.count() != 1) {
            qCWarning(dcLogEngine()) << "Unable to read databases from influxdb. No result set.";
            m_initStatus = InitStatusFailure;
            return;
        }

        QVariantList series = results.first().toMap().value("series").toList();
        if (series.count() != 1) {
            qCWarning(dcLogEngine()) << "Unable to read databases from influxdb. No series set.";
            m_initStatus = InitStatusFailure;
            return;
        }

        QVariantList values = series.first().toMap().value("values").toList();

        qCDebug(dcLogEngine()) << "Databases in influx:" << values;
        bool nymeaDBfound = false;
        foreach (const QVariant &value, values) {
            QVariantList valueList = value.toList();
            if (valueList.count() > 0) {
                if (valueList.first().toString() == m_dbName) {
                    nymeaDBfound = true;
                    break;
                }
            }
        }

        if (nymeaDBfound) {
            qCDebug(dcLogEngine()) << "influxdb database already set up.";
            createRetentionPolicies();
            return;
        }
        qCInfo(dcLogEngine()) << "Creating" << m_dbName << "database in influxdb";

        QueryJob *job = query(QString("CREATE DATABASE %1").arg(m_dbName), true, true);
        connect(job, &QueryJob::finished, this, [=](QNetworkReply::NetworkError status, const QVariantList &result) {
            if (status != QNetworkReply::NoError) {
                qCCritical(dcLogEngine()) << "Unable to create" << m_dbName << "database in influxdb:" << QJsonDocument::fromVariant(result).toJson();
                m_initStatus = InitStatusFailure;
                return;
            }
            qCInfo(dcLogEngine()) << m_dbName << "database created in influxdb.";
            createRetentionPolicies();
        });
    });
}

void LogEngineInfluxDB::createRetentionPolicies()
{
    QueryJob *job = query("SHOW RETENTION POLICIES", false, true);
    connect(job, &QueryJob::finished, this, [=](QNetworkReply::NetworkError status, const QVariantList &results){
        if (status != QNetworkReply::NoError) {
            qCCritical(dcLogEngine()) << "Unable to query retention policies.";
            m_initStatus = InitStatusFailure;
            return;
        }

        if (results.count() != 1) {
            qCWarning(dcLogEngine()) << "Unable to read retention policies from influxdb. No result set.";
            m_initStatus = InitStatusFailure;
            return;
        }

        QVariantList series = results.first().toMap().value("series").toList();
        if (series.count() != 1) {
            qCWarning(dcLogEngine()) << "Unable to read retention policies from influxdb. No series set.";
            m_initStatus = InitStatusFailure;
            return;
        }

        QVariantList values = series.first().toMap().value("values").toList();

        qCDebug(dcLogEngine()) << "Retention policies in influx:" << values;
        bool discreteRPFound = false, liveRPFound = false, minutesRPFound = false, hoursRPFound = false, daysRPFound = false;
        foreach (const QVariant &value, values) {
            QVariantList valueList = value.toList();
            if (valueList.count() > 0) {
                if (valueList.first().toString() == "discrete") {
                    discreteRPFound = true;
                    continue;
                }
            }
            if (valueList.count() > 0) {
                if (valueList.first().toString() == "live") {
                    liveRPFound = true;
                    continue;
                }
            }
            if (valueList.count() > 0) {
                if (valueList.first().toString() == "minutes") {
                    minutesRPFound = true;
                    continue;
                }
            }
            if (valueList.count() > 0) {
                if (valueList.first().toString() == "hours") {
                    hoursRPFound = true;
                    continue;
                }
            }
            if (valueList.count() > 0) {
                if (valueList.first().toString() == "days") {
                    daysRPFound = true;
                    continue;
                }
            }
        }

        if (!discreteRPFound) {
            qCInfo(dcLogEngine()) << "Creating discrete nymea retention policy in influxdb";
            QueryJob *job = query(QString("CREATE RETENTION POLICY discrete ON %1 DURATION 8760h REPLICATION 1").arg(m_dbName), true, true);
            connect(job, &QueryJob::finished, this, [=](QNetworkReply::NetworkError status){
                if (status != QNetworkReply::NoError) {
                    qCWarning(dcLogEngine()) << "Unable to create discrete retention policy in influxdb.";
                    m_initStatus = InitStatusFailure;
                    return;
                }
                createRetentionPolicies();
            });
            return;
        }

        if (!liveRPFound) {
            qCInfo(dcLogEngine()) << "Creating live nymea retention policy in influxdb";
            QueryJob *job = query(QString("CREATE RETENTION POLICY live ON %1 DURATION 24h REPLICATION 1").arg(m_dbName), true, true);
            connect(job, &QueryJob::finished, this, [=](QNetworkReply::NetworkError status){
                if (status != QNetworkReply::NoError) {
                    qCWarning(dcLogEngine()) << "Unable to create live retention policy in influxdb.";
                    m_initStatus = InitStatusFailure;
                    return;
                }
                createRetentionPolicies();
            });
            return;
        }

        if (!minutesRPFound) {
            qCInfo(dcLogEngine()) << "Creating minutes nymea retention policy in influxdb";
            QueryJob *job = query(QString("CREATE RETENTION POLICY minutes ON %1 DURATION 168h REPLICATION 1").arg(m_dbName), true, true);
            connect(job, &QueryJob::finished, this, [=](QNetworkReply::NetworkError status){
                if (status != QNetworkReply::NoError) {
                    qCWarning(dcLogEngine()) << "Unable to create minutes retention policy in influxdb.";
                    m_initStatus = InitStatusFailure;
                    return;
                }
                createRetentionPolicies();
            });
            return;
        }

        if (!hoursRPFound) {
            qCInfo(dcLogEngine()) << "Creating hours nymea retention policy in influxdb";
            QueryJob *job = query(QString("CREATE RETENTION POLICY hours ON %1 DURATION 26280h REPLICATION 1").arg(m_dbName), true, true);
            connect(job, &QueryJob::finished, this, [=](QNetworkReply::NetworkError status){
                if (status != QNetworkReply::NoError) {
                    qCWarning(dcLogEngine()) << "Unable to create hours retention policy in influxdb.";
                    m_initStatus = InitStatusFailure;
                    return;
                }
                createRetentionPolicies();
            });
            return;
        }

        if (!daysRPFound) {
            qCInfo(dcLogEngine()) << "Creating days nymea retention policy in influxdb";
            QueryJob *job = query(QString("CREATE RETENTION POLICY days ON %1 DURATION 175200h REPLICATION 1").arg(m_dbName), true, true);
            connect(job, &QueryJob::finished, this, [=](QNetworkReply::NetworkError status){
                if (status != QNetworkReply::NoError) {
                    qCWarning(dcLogEngine()) << "Unable to create days retention policy in influxdb.";
                    m_initStatus = InitStatusFailure;
                    return;
                }
                createRetentionPolicies();
            });
            return;
        }

        m_initStatus = InitStatusOK;

        qCDebug(dcLogEngine()) << "Influx initialized. Starting to process log entries (" << m_initQueryQueue.count() << m_queryQueue.count() << m_writeQueue.count() << "in queue)";
        processQueues();
    });
}

QNetworkRequest LogEngineInfluxDB::createQueryRequest(const QString &quer)
{
    QUrl url;
    url.setScheme("http");
    url.setHost(m_host);
    url.setPort(8086);
    url.setPath("/query");

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("db", m_dbName);
    urlQuery.addQueryItem("q", quer);
    urlQuery.addQueryItem("epoch", "ms");
    url.setQuery(urlQuery);

    QNetworkRequest request(url);

    if (!m_username.isEmpty() || !m_password.isEmpty()) {
        QByteArray auth = QByteArray(m_username.toLatin1() + ':' + m_password.toLatin1()).toBase64(QByteArray::Base64Encoding | QByteArray::KeepTrailingEquals);
        request.setRawHeader("Authorization", QString("Basic %1").arg(QString(auth)).toUtf8());
    }

    return request;
}

QNetworkRequest LogEngineInfluxDB::createWriteRequest(const QString &retentionPolicy)
{
    QUrl url;
    url.setScheme("http");
    url.setHost(m_host);
    url.setPort(8086);
    url.setPath("/write");

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("db", m_dbName);
    urlQuery.addQueryItem("precision", "ms");
    urlQuery.addQueryItem("rp", retentionPolicy);
    url.setQuery(urlQuery);

    QNetworkRequest request(url);

    if (!m_username.isEmpty() || !m_password.isEmpty()) {
        QByteArray auth = QByteArray(m_username.toLatin1() + ':' + m_password.toLatin1()).toBase64(QByteArray::Base64Encoding | QByteArray::KeepTrailingEquals);
        request.setRawHeader("Authorization", QString("Basic %1").arg(QString(auth)).toUtf8());
    }
    return request;
}

QueryJob *LogEngineInfluxDB::query(const QString &query, bool post, bool isInit)
{
    QNetworkRequest request = createQueryRequest(query);
    if (post) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    }

    QueryJob *job = new QueryJob(request, post, isInit, this);

    if (isInit) {
        m_initQueryQueue.append(job);
    } else {
        m_queryQueue.append(job);
    }

    QMetaObject::invokeMethod(this, "processQueues", Qt::QueuedConnection);
    return job;
}

QueryJob::QueryJob(const QNetworkRequest &request, bool post, bool isInit, QObject *parent):
    QObject(parent),
    m_request(request),
    m_post(post),
    m_isInit(isInit)
{

}

void QueryJob::finish(QNetworkReply::NetworkError status, const QVariantList &results)
{
    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection, Q_ARG(QNetworkReply::NetworkError, status), Q_ARG(QVariantList, results));
    QMetaObject::invokeMethod(this, "deleteLater", Qt::QueuedConnection);
}

