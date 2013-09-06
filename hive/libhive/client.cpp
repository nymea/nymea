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
            emit dataAvailable(message);
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

void Client::processData(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qDebug() << "failed to parse data" << data << ":" << error.errorString();
    }
    qDebug() << "-------------------------\n" << jsonDoc.toJson();

    QVariantMap command = jsonDoc.toVariant().toMap();
    QVariantMap params = jsonDoc.toVariant().toMap().value("params").toMap();

    if(command.contains("signal")){
        handleSignal(params);
    }else{
        handleResponse(params);
    }

}

void Client::handleResponse(const QVariantMap &rsp)
{
    qDebug() << "handling response" << rsp;
    if(!rsp.contains("id")) {
        qDebug() << "packet does not contain an id. discarding...";
        return;
    }
    int id = rsp.value("id").toInt();
    if(m_requestMap.contains(id)) {
        switch(m_requestMap.value(id)){
        case RequestAddDevice:
            qDebug() << "add device response received";
            break;
        case RequestEditDevice:
            qDebug() << "edit device response received";
            break;
        case RequestRemoveDevice:
            qDebug() << "add device response received";
            break;

        }
        m_requestMap.remove(id);
    }




}

void Client::handleSignal(const QVariantMap &signal)
{

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
