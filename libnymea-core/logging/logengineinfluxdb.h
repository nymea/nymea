#ifndef LOGENGINEINFLUXDB_H
#define LOGENGINEINFLUXDB_H

#include "logging/logengine.h"
#include <QObject>
#include <QQueue>
#include <QHostAddress>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class QueryJob: public QObject {
    Q_OBJECT
    explicit QueryJob(const QNetworkRequest &request, bool post, bool isInit, QObject *parent = nullptr);

signals:
    void finished(QNetworkReply::NetworkError status, const QVariantList &response);

private:
    friend class LogEngineInfluxDB;
    QNetworkRequest m_request;
    bool m_post = false;
    bool m_isInit = false;

    void finish(QNetworkReply::NetworkError status, const QVariantList &results = QVariantList());
};

class LogEngineInfluxDB : public LogEngine
{
    Q_OBJECT
public:
    enum InitStatus {
        InitStatusNone,
        InitStatusStarting,
        InitStatusOK,
        InitStatusFailure
    };
    Q_ENUM(InitStatus)

    explicit LogEngineInfluxDB(const QString &host, const QString &dbName, const QString &username = QString(), const QString &password = QString(), QObject *parent = nullptr);
    ~LogEngineInfluxDB();

    Logger *registerLogSource(const QString &name, const QStringList &tagNames, Types::LoggingType loggingType = Types::LoggingTypeDiscrete, const QString &sampleColumn = QString()) override;

    void unregisterLogSource(const QString &name) override;

    void logEvent(Logger *logger, const QStringList &tags, const QVariantMap &values) override;

    LogFetchJob *fetchLogEntries(const QStringList &sources, const QStringList &columns, const QDateTime &startTime = QDateTime(), const QDateTime &endTime = QDateTime(), const QVariantMap &filter = QVariantMap(), Types::SampleRate sampleRate = Types::SampleRateAny, Qt::SortOrder sortOrder = Qt::AscendingOrder, int offset = 0, int limit = 0) override;

    bool jobsRunning() const override;
    void clear(const QString &source) override;

private:
    void initDB();
    void createRetentionPolicies();
    void createDB();

    QNetworkRequest createQueryRequest(const QString &quer);
    QNetworkRequest createWriteRequest(const QString &retentionPolicy);

    QueryJob *query(const QString &query, bool post = false, bool isInit = false);

private slots:
    void processQueues();

private:
    struct QueueEntry {
        QNetworkRequest request;
        QByteArray data;
        LogEntry entry;
    };

    InitStatus m_initStatus = InitStatusNone;

    QNetworkAccessManager *m_nam = nullptr;

    QString m_host;
    QString m_dbName;
    QString m_username;
    QString m_password;

    QHash<QString, Logger*> m_loggers;

    QQueue<QueryJob*> m_initQueryQueue;
    QueryJob *m_currentInitQuery = nullptr;
    QQueue<QueryJob*> m_queryQueue;
    QueryJob *m_currentQuery = nullptr;
    QQueue<QueueEntry> m_writeQueue;
    QNetworkReply* m_currentWriteReply = nullptr;
};

#endif // LOGENGINEINFLUXDB_H
