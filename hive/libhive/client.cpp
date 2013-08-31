#include "client.h"
#include <QDebug>
#include <QJsonDocument>

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
    QByteArray message;
    while(m_tcpSocket->canReadLine()){
        QByteArray dataLine = m_tcpSocket->readLine();
        message.append(dataLine);
        if(dataLine == "}\n"){
            emit jsonDataAvailable(message);
            message.clear();
        }
    }
}

void Client::connectedToHost()
{
    qDebug() << "connected to hive server";
    emit connected();
}

void Client::connectToHost(QString ipAddress, QString port)
{
    qDebug() << "connecting to" << ipAddress << ":" << port;
    m_tcpSocket->connectToHost(QHostAddress(ipAddress), port.toInt());
}

void Client::disconnectFromHost()
{
    m_tcpSocket->close();
    qDebug() << "connection to hive server closed";
}

void Client::sendData(QByteArray data)
{
    m_tcpSocket->write(data);
}
