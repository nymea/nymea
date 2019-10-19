#ifndef JSONREPLY_H
#define JSONREPLY_H

#include <QObject>
#include <QVariantMap>
#include <QUuid>
#include <QTimer>

class JsonHandler;

class JsonReply: public QObject
{
    Q_OBJECT
public:
    enum Type {
        TypeSync,
        TypeAsync
    };

    static JsonReply *createReply(JsonHandler *handler, const QVariantMap &data);
    static JsonReply *createAsyncReply(JsonHandler *handler, const QString &method);

    Type type() const;
    QVariantMap data() const;
    void setData(const QVariantMap &data);

    JsonHandler *handler() const;
    QString method() const;

    QUuid clientId() const;
    void setClientId(const QUuid &clientId);

    int commandId() const;
    void setCommandId(int commandId);

    bool timedOut() const;

public slots:
    void startWait();

signals:
    void finished();

private slots:
    void timeout();

private:
    JsonReply(Type type, JsonHandler *handler, const QString &method, const QVariantMap &data = QVariantMap());
    Type m_type;
    QVariantMap m_data;

    JsonHandler *m_handler;
    QString m_method;
    QUuid m_clientId;
    int m_commandId;
    bool m_timedOut;

    QTimer m_timeout;

};

#endif // JSONREPLY_H
