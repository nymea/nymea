#include "client.h"
#include <QDebug>

Client::Client(QObject *parent) :
    QObject(parent)
{
    m_tcpSocket = new QTcpSocket(this);

    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(m_tcpSocket, SIGNAL(connected()),this, SLOT(connectedToHost()));

}

void Client::connectionError(QAbstractSocket::SocketError error)
{
    qDebug() << "---> connection error:" << error;
}

void Client::readData()
{

}

void Client::connectedToHost()
{
    qDebug() << "connected to hive server";
}

void Client::connectToHost(QString ipAddress, QString port)
{
    m_tcpSocket->connectToHost(QHostAddress(ipAddress), port.toInt());
}

void Client::disconnectFromHost()
{
    m_tcpSocket->close();
    qDebug() << "connection to hive server closed";
}

void Client::sendData(QString target, QString command)
{

}
