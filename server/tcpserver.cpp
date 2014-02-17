#include "tcpserver.h"
#include <QDebug>
#include <QJsonDocument>

TcpServer::TcpServer(QObject *parent) :
    QObject(parent)
{       

    qDebug() << "----------------------------";
    qDebug() << "network interfaces:";
    foreach(const QNetworkInterface &interface, QNetworkInterface::allInterfaces()){
        qDebug() << "   -------------------------";
        qDebug() << "   name:" << interface.name();
        qDebug() << "   mac: " << interface.hardwareAddress();
    }
    qDebug() << "----------------------------";

}

void TcpServer::sendResponse(int clientId, const QByteArray &data)
{
    QTcpSocket *client = m_clientList.value(clientId);
    if (client) {
        client->write(data);
    }
}

void TcpServer::newClientConnected()
{
    // got a new client connected
    QTcpServer *server = qobject_cast<QTcpServer*>(sender());
    QTcpSocket *newConnection = server->nextPendingConnection();
    qDebug() << "new client connected:" << newConnection->peerAddress().toString();

    // append the new client to the client list
    m_clientList.insert(m_clientList.count(), newConnection);

    connect(newConnection, SIGNAL(readyRead()),this,SLOT(readPackage()));
    connect(newConnection,SIGNAL(disconnected()),this,SLOT(clientDisconnected()));

    // TODO: properly handle this with jsonrpcserver
    newConnection->write("{\n    \"id\":0,\n    \"status\": \"connected\",\n    \"server\":\"Hive JSONRPC Interface\",\n    \"version\":\"0.0.0\"\n}\n");
}


void TcpServer::readPackage()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    qDebug() << "-----------> data comming from" << client->peerAddress().toString();
    QByteArray message;
    while(client->canReadLine()){
        QByteArray dataLine = client->readLine();
        qDebug() << "line in:" << dataLine;
        message.append(dataLine);
        if(dataLine.endsWith('\n')){
            emit jsonDataAvailable(m_clientList.key(client), message);
            message.clear();
        }
    }
}

void TcpServer::clientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    qDebug() << "client disconnected:" << client->peerAddress().toString();
}

bool TcpServer::startServer()
{
    // Listen on all Networkinterfaces
    foreach(const QHostAddress &address, QNetworkInterface::allAddresses()){
        QTcpServer *server = new QTcpServer(this);
        if(server->listen(address, 1234)) {
            qDebug() << "server listening on" << address.toString();
            connect(server, SIGNAL(newConnection()), SLOT(newClientConnected()));
            m_serverList.insert(m_serverList.count(), server);
        } else {
            qDebug() << "ERROR: can not listening to" << address.toString();
            delete server;
        }
    }
    if(m_serverList.empty()){
        return false;
    }
    return true;
}

bool TcpServer::stopServer()
{
    // Listen on all Networkinterfaces
    foreach(QTcpServer *server, m_serverList){
        qDebug() << "close server " << server->serverAddress().toString();
        server->close();
        delete server;
    }
    if(!m_serverList.empty()){
        return false;
    }
    return true;
}

void TcpServer::sendToAll(QByteArray data)
{
    foreach(QTcpSocket *client,m_clientList){
        client->write(data);
    }
}


