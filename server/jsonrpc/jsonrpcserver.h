#ifndef JSONRPCSERVER_H
#define JSONRPCSERVER_H

#include "deviceclass.h"
#include "action.h"
#include "trigger.h"

#include <QObject>
#include <QVariantMap>
#include <QString>

#ifdef TESTING_ENABLED
class MockTcpServer;
#else
class TcpServer;
#endif
class Device;
class JsonHandler;

class JsonRPCServer: public QObject
{
    Q_OBJECT
public:
    JsonRPCServer(QObject *parent = 0);

signals:
    void commandReceived(const QString &targetNamespace, const QString &command, const QVariantMap &params);

private slots:
    void processData(int clientId, const QByteArray &jsonData);

private:
    void registerHandler(JsonHandler *handler);

    void sendResponse(int clientId, int commandId, const QVariantMap &params = QVariantMap());
    void sendErrorResponse(int clientId, int commandId, const QString &error);

private:
#ifdef TESTING_ENABLED
    MockTcpServer *m_tcpServer;
#else
    TcpServer *m_tcpServer;
#endif
    QHash<QString, JsonHandler*> m_handlers;
};

#endif
