/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

#include "tunemanager.h"

TuneManager::TuneManager(int port, QObject *parent) :
    QObject(parent),
    m_server(0),
    m_tune(0),
    m_port(port),
    m_connected(false)
{
}

bool TuneManager::tuneAvailable()
{
    return m_connected;
}

bool TuneManager::sendData(const QByteArray &data)
{
    if (m_connected) {
        m_tune->write(data + '\n');
        return true;
    }
    return false;
}

void TuneManager::tuneConnected()
{
    QTcpSocket *socket = m_server->nextPendingConnection();

    if (m_tune) {
        qWarning() << "--> ATTENTION: tune allready connected! connection refused.";
        socket->disconnect();
        delete socket;
        return;
    }

    m_tune = socket;

    connect(m_tune, &QTcpSocket::readyRead, this, &TuneManager::readData);
    connect(m_tune, &QTcpSocket::disconnected, this, &TuneManager::tuneDisconnected);

    qDebug() << " --> tune connected:" << m_tune->peerAddress().toString() << m_port;
    m_connected = true;
    emit tuneConnectionStatusChanged(true);
}

void TuneManager::tuneDisconnected()
{
    qWarning() << " --> tune disconnected:" << m_tune->peerAddress().toString();
    m_connected = false;
    emit tuneConnectionStatusChanged(false);
    delete m_tune;
    m_tune = 0;
}

void TuneManager::readData()
{
    QByteArray message;
    while (m_tune->canReadLine()) {
        QByteArray dataLine = m_tune->readLine();
        //qDebug() << " --> tune line in:" << dataLine;
        message.append(dataLine);
        if (dataLine.endsWith('\n')) {
            emit dataReady(message);
            message.clear();
        }
    }
}

bool TuneManager::start()
{
    if(!m_server) {
        m_server = new QTcpServer(this);
    }

    m_server->setMaxPendingConnections(1);

    QHostAddress localhost = QHostAddress(QHostAddress::LocalHost);
    if(!m_server->listen(localhost, m_port)) {
        qWarning() << "ERROR: Tune server can not listen on" << localhost << m_port;
        delete m_server;
        return false;
    }
    qDebug() << " --> Tune server started" << localhost << m_port;
    connect(m_server, &QTcpServer::newConnection, this, &TuneManager::tuneConnected);
    return true;
}

void TuneManager::stop()
{
    qDebug() << " --> close Tune server" << m_server->serverAddress().toString();
    m_server->close();
    delete m_server;
    m_server = 0;
    m_connected = false;
    emit tuneConnectionStatusChanged(false);
}
