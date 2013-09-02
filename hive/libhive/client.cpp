#include "client.h"
#include <QDebug>
#include <QJsonDocument>

Client::Client(QObject *parent) :
    QObject(parent)
{
    m_tcpSocket = new QTcpSocket(this);
    m_connectionStatus = false;
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(m_tcpSocket, SIGNAL(connected()),this, SLOT(connected()));
    connect(m_tcpSocket, SIGNAL(disconnected()),this, SLOT(disconneted()));
}

bool Client::isConnected()
{
    return m_connectionStatus;
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

void Client::connected()
{
    qDebug() << "connected to hive server";
    m_connectionStatus = true;
    emit connectionChanged();
}

void Client::disconneted()
{
    qDebug() << "disconnect from hive server";
    m_connectionStatus = false;
    emit connectionChanged();
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
