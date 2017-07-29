/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

/*!
    \class guhserver::TransportInterface
    \brief This class provides an interface for the JSON servers.

    \ingroup server
    \inmodule core

    \sa WebSocketServer, TcpServer
*/

/*! \fn void guhserver::TransportInterface::clientConnected(const QUuid &clientId);
    This signal is emitted when a new client with the given \a clientId has been connected.

    \sa WebSocketServer, TcpServer
*/

/*! \fn void guhserver::TransportInterface::clientDisconnected(const QUuid &clientId);
    This signal is emitted when a new client with the given \a clientId has been disconnected.

    \sa WebSocketServer, TcpServer
*/

/*! \fn bool guhserver::TransportInterface::startServer();
    Pure virtual public slot for starting the corresponding \l{TransportInterface}. Returns true
    if started successfully.

    \sa WebSocketServer::startServer(), TcpServer::startServer()
*/

/*! \fn bool guhserver::TransportInterface::stopServer();
    Pure virtual public slot for stopping the corresponding \l{TransportInterface}. Returns true
    if stopped successfully.

    \sa WebSocketServer::stopServer(), TcpServer::stopServer()
*/

/*! \fn void guhserver::TransportInterface::sendData(const QUuid &clientId, const QVariantMap &data);
    Pure virtual method for sending \a data to the client with the id \a clientId over the corresponding \l{TransportInterface}.
*/

/*! \fn void guhserver::TransportInterface::sendData(const QList<QUuid> &clients, const QVariantMap &data);
    Pure virtual method for sending \a data to \a clients over the corresponding \l{TransportInterface}.
*/

/*! \fn void guhserver::TransportInterface::dataAvailable(const QUuid &clientId, const QString &targetNamespace, const QString &method, const QVariantMap &message);
    This signal is emitted when valid data from the client with the given \a clientId are available.
    Data are valid if the corresponding \l{TransportInterface} has parsed successfully the given
    \a targetNamespace, \a method and \a message.

    \sa WebSocketServer, TcpServer
*/

#include "transportinterface.h"
#include "loggingcategories.h"

#include <QJsonDocument>

namespace guhserver {

/*! Constructs a \l{TransportInterface} with the given \a parent. */
TransportInterface::TransportInterface(QObject *parent) :
    QObject(parent)
{
}

/*! Pure virtual destructor for \l{TransportInterface}. */
TransportInterface::~TransportInterface()
{
}

/*! Send a JSON success response to the client with the given \a clientId,
 * \a commandId and \a params to the inerted \l{TransportInterface}.
 */
void TransportInterface::sendResponse(const QUuid &clientId, int commandId, const QVariantMap &params)
{
    QVariantMap response;
    response.insert("id", commandId);
    response.insert("status", "success");
    response.insert("params", params);

    sendData(clientId, response);
}

/*! Send a JSON error response to the client with the given \a clientId,
 * \a commandId and \a error to the inerted \l{TransportInterface}.
 */
void TransportInterface::sendErrorResponse(const QUuid &clientId, int commandId, const QString &error)
{
    QVariantMap errorResponse;
    errorResponse.insert("id", commandId);
    errorResponse.insert("status", "error");
    errorResponse.insert("error", error);

    sendData(clientId, errorResponse);
}

}
