#include "kodiconnection.h"
#include "loggingcategories.h"
#include "jsonhandler.h"

#include <QPixmap>

KodiConnection::KodiConnection(const QHostAddress &hostAddress, const int &port, QObject *parent) :
    QObject(parent),
    m_hostAddress(hostAddress),
    m_port(port),
    m_connected(false)
{
    m_socket = new QTcpSocket(this);

    connect(m_socket, &QTcpSocket::connected, this, &KodiConnection::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &KodiConnection::onDisconnected);
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(m_socket, &QTcpSocket::readyRead, this, &KodiConnection::readData);
}

void KodiConnection::connectToKodi()
{
    if (m_socket->state() == QAbstractSocket::ConnectingState) {
        return;
    }
    m_socket->connectToHost(m_hostAddress, m_port);
}

void KodiConnection::disconnectFromKodi()
{
    m_socket->close();
}

QHostAddress KodiConnection::hostAddress() const
{
    return m_hostAddress;
}

int KodiConnection::port() const
{
    return m_port;
}

bool KodiConnection::connected()
{
    return m_connected;
}

void KodiConnection::onConnected()
{
    qCDebug(dcKodi) << "connected successfully to" << hostAddress().toString() << port();
    m_connected = true;
//    QPixmap logo = QPixmap(":/images/guh-logo.png");
//    qCDebug(dcKodi) << "image size" << logo.size();
    emit connectionStateChanged(true);
}

void KodiConnection::onDisconnected()
{
    qCDebug(dcKodi) << "disconnected from" << hostAddress().toString() << port();
    m_connected = false;
    emit connectionStateChanged(false);
}

void KodiConnection::onError(QAbstractSocket::SocketError socketError)
{
    qCWarning(dcKodi) << "socket error:" << socketError << m_socket->errorString();
}

void KodiConnection::readData()
{
    QByteArray data = m_socket->readAll();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcKodi) << "failed to parse JSON data:" << data << ":" << error.errorString();
        return;
    }
    qCDebug(dcKodi) << "data received:" << jsonDoc.toJson();

    emit dataReady(data);
}

void KodiConnection::sendData(const QString &method, const QVariantMap &params)
{
    QVariantMap package;
    package.insert("id", m_id);
    package.insert("method", method);
    package.insert("params", params);
    package.insert("jsonrpc", "2.0");
    m_id++;

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(package);
    qCDebug(dcKodi) << "sending data" << jsonDoc.toJson();

    m_socket->write(jsonDoc.toJson());
}

