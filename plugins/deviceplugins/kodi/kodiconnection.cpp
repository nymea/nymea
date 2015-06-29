/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "kodiconnection.h"
#include "loggingcategories.h"
#include "jsonhandler.h"
#include "extern-plugininfo.h"

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

void KodiConnection::connectKodi()
{
    if (m_socket->state() == QAbstractSocket::ConnectingState) {
        return;
    }
    m_socket->connectToHost(m_hostAddress, m_port);
}

void KodiConnection::disconnectKodi()
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
    emit connectionStatusChanged();
}

void KodiConnection::onDisconnected()
{
    qCDebug(dcKodi) << "disconnected from" << hostAddress().toString() << port();
    m_connected = false;
    emit connectionStatusChanged();
}

void KodiConnection::onError(QAbstractSocket::SocketError socketError)
{
    if (connected()) {
        qCWarning(dcKodi) << "socket error:" << socketError << m_socket->errorString();
    }
}

void KodiConnection::readData()
{
    QByteArray data = m_socket->readAll();
    emit dataReady(data);
}

void KodiConnection::sendData(const QByteArray &message)
{
    m_socket->write(message);
}

