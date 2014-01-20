#ifndef JSONRPCSERVER_H
#define JSONRPCSERVER_H

#include "deviceclass.h"
#include "action.h"
#include "trigger.h"

#include <QObject>
#include <QVariantMap>
#include <QString>

class TcpServer;
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
    TcpServer *m_tcpServer;
    QHash<QString, JsonHandler*> m_handlers;
};

#endif
