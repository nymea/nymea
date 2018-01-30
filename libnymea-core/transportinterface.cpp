/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::TransportInterface
    \brief This class provides an interface for the JSON servers.

    \ingroup server
    \inmodule core

    \sa WebSocketServer, TcpServer
*/

/*! \fn void nymeaserver::TransportInterface::clientConnected(const QUuid &clientId);
    This signal is emitted when a new client with the given \a clientId has been connected.

    \sa WebSocketServer, TcpServer
*/

/*! \fn void nymeaserver::TransportInterface::clientDisconnected(const QUuid &clientId);
    This signal is emitted when a new client with the given \a clientId has been disconnected.

    \sa WebSocketServer, TcpServer
*/

/*! \fn bool nymeaserver::TransportInterface::startServer();
    Pure virtual public slot for starting the corresponding \l{TransportInterface}. Returns true
    if started successfully.

    \sa WebSocketServer::startServer(), TcpServer::startServer()
*/

/*! \fn bool nymeaserver::TransportInterface::stopServer();
    Pure virtual public slot for stopping the corresponding \l{TransportInterface}. Returns true
    if stopped successfully.

    \sa WebSocketServer::stopServer(), TcpServer::stopServer()
*/

/*! \fn void nymeaserver::TransportInterface::sendData(const QUuid &clientId, const QVariantMap &data);
    Pure virtual method for sending \a data to the client with the id \a clientId over the corresponding \l{TransportInterface}.
*/

/*! \fn void nymeaserver::TransportInterface::sendData(const QList<QUuid> &clients, const QVariantMap &data);
    Pure virtual method for sending \a data to \a clients over the corresponding \l{TransportInterface}.
*/

/*! \fn void nymeaserver::TransportInterface::dataAvailable(const QUuid &clientId, const QString &targetNamespace, const QString &method, const QVariantMap &message);
    This signal is emitted when valid data from the client with the given \a clientId are available.
    Data are valid if the corresponding \l{TransportInterface} has parsed successfully the given
    \a targetNamespace, \a method and \a message.

    \sa WebSocketServer, TcpServer
*/

#include "transportinterface.h"
#include "loggingcategories.h"

#include <QJsonDocument>

namespace nymeaserver {

/*! Constructs a \l{TransportInterface} with the given \a parent. */
TransportInterface::TransportInterface(const ServerConfiguration &config, QObject *parent) :
    QObject(parent),
    m_config(config)
{
}

void TransportInterface::setConfiguration(const ServerConfiguration &config)
{
    m_config = config;
}

/*! Returns the \{ServerConfiguration}. */
ServerConfiguration TransportInterface::configuration() const
{
    return m_config;
}

void TransportInterface::setServerName(const QString &serverName)
{
    m_serverName = serverName;
}

/*! Virtual destructor for \l{TransportInterface}. */
TransportInterface::~TransportInterface()
{
}

}
