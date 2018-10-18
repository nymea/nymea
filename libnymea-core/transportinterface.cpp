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

/*! \fn void nymeaserver::TransportInterface::sendData(const QUuid &clientId, const QByteArray &data);
    Pure virtual method for sending \a data to the client with the id \a clientId over the corresponding \l{TransportInterface}.
*/

/*! \fn void nymeaserver::TransportInterface::sendData(const QList<QUuid> &clients, const QByteArray &data);
    Pure virtual method for sending \a data to \a clients over the corresponding \l{TransportInterface}.
*/

/*! \fn void nymeaserver::TransportInterface::terminateClientConnection(const QUuid &clientId);
    Pure virtual method for terminating \a clients connection. The JSON RPC server might call this when a
    client violates the protocol. Transports should immediately abort the connection to the client.
*/

/*! \fn void nymeaserver::TransportInterface::dataAvailable(const QUuid &clientId, const QByteArray &data);
    This signal is emitted when valid \a data from the client with the given \a clientId are available.

    \sa WebSocketServer, TcpServer, BluetoothServer
*/

#include "transportinterface.h"
#include "loggingcategories.h"

#include <QJsonDocument>

namespace nymeaserver {

/*! Constructs a \l{TransportInterface} with the given \a config and \a parent. */
TransportInterface::TransportInterface(const ServerConfiguration &config, QObject *parent) :
    QObject(parent),
    m_config(config)
{
}

/*! Set the ServerConfiguration of this TransportInterface to the given \a config. */
void TransportInterface::setConfiguration(const ServerConfiguration &config)
{
    m_config = config;
}

/*! Returns the \{ServerConfiguration}. */
ServerConfiguration TransportInterface::configuration() const
{
    return m_config;
}

/*! Set the name of this TransportInterface to the given \a serverName. */
void TransportInterface::setServerName(const QString &serverName)
{
    m_serverName = serverName;
}

/*! Virtual destructor for \l{TransportInterface}. */
TransportInterface::~TransportInterface()
{
}

}
