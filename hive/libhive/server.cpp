#include "server.h"
#include <QDebug>
#include <QJsonDocument>

Server::Server(QObject *parent) :
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

void Server::newClientConnected()
{
    // got a new client connected
    QTcpServer *server = qobject_cast<QTcpServer*>(sender());
    QTcpSocket *newConnection = server->nextPendingConnection();
    qDebug() << "new client connected:" << newConnection->peerAddress().toString();

    // append the new client to the client list
    m_clientList.append(newConnection);

    connect(newConnection, SIGNAL(readyRead()),this,SLOT(readPackage()));
    connect(newConnection,SIGNAL(disconnected()),this,SLOT(clientDisconnected()));

}


void Server::readPackage()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    //qDebug() << "-----------> data comming from" << client->peerAddress().toString();
    QByteArray message;
    while(client->canReadLine()){
        QByteArray dataLine = client->readLine();
        //qDebug() << "line in:" << dataLine;
        message.append(dataLine);
        if(dataLine == "}\n"){
            //qDebug() << message;
            emit jsonDataAvailable(message);
            message.clear();
        }
    }

}

void Server::clientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    qDebug() << "client disconnected:" << client->peerAddress().toString();
}

bool Server::startServer()
{
    // Listen on all Networkinterfaces
    foreach(const QHostAddress &address, QNetworkInterface::allAddresses()){
        QTcpServer *server = new QTcpServer(this);
        if(server->listen(address, 1234)) {
            qDebug() << "server listening on" << address.toString();
            connect(server, SIGNAL(newConnection()), SLOT(newClientConnected()));
            m_serverList.append(server);
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

bool Server::stopServer()
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

void Server::sendToAll(QByteArray data)
{
    foreach(QTcpSocket *client,m_clientList){
        client->write(data);
    }
}


