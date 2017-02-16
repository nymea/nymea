/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "kodiconnection.h"
#include "kodijsonhandler.h"
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

    QStringList commandList = QString(data).split("}{");
    for(int i = 0; i < commandList.count(); ++i) {
        QString command = commandList.at(i);
        if(command.isEmpty()) {
            continue;
        }
        if(i < commandList.count() - 1) {
            command.append("}");
        }
        if(i > 0) {
            command.prepend("{");
        }
        emit dataReady(command.toUtf8());
    }
}

void KodiConnection::sendData(const QByteArray &message)
{
    m_socket->write(message);
}

