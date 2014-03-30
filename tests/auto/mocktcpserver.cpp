#include "mocktcpserver.h"

QList<MockTcpServer*> MockTcpServer::s_allServers;

MockTcpServer::MockTcpServer(QObject *parent):
    QObject(parent)
{
    s_allServers.append(this);
}

MockTcpServer::~MockTcpServer()
{
    s_allServers.removeAll(this);
}

void MockTcpServer::sendData(const QUuid &clientId, const QByteArray &data)
{
    emit outgoingData(clientId, data);
}

QList<MockTcpServer *> MockTcpServer::servers()
{
    return s_allServers;
}

void MockTcpServer::injectData(const QUuid &clientId, const QByteArray &data)
{
    emit dataAvailable(clientId, data);
}

bool MockTcpServer::startServer()
{
    qDebug() << "should start server";
    return true;
}

bool MockTcpServer::stopServer()
{
    qDebug() << "should stop server";
    return true;
}

void MockTcpServer::sendToAll(QByteArray data)
{
     qDebug() << "should send to all clients:" << data;
}
