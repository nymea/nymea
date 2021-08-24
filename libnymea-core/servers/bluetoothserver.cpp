/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::BluetoothServer
    \brief This class represents the bluetooth server for nymead.

    \ingroup server
    \inmodule core

    \inherits TransportInterface

    The bluetooth server allows clients to connect to the JSON-RPC API using an RFCOMM bluetooth connection. If the server is enabled, a client
    can discover the services running on this host. The service for the JSON-RPC api is called \tt nymea and has the uuid \tt 997936b5-d2cd-4c57-b41b-c6048320cd2b .

    \sa TransportInterface
*/

#ifdef WITH_BLUETOOTH

#include "bluetoothserver.h"
#include "loggingcategories.h"

#include <QJsonDocument>

namespace nymeaserver {

static const QBluetoothUuid nymeaServiceUuid(QUuid("997936b5-d2cd-4c57-b41b-c6048320cd2b"));

/*! Constructs a \l{BluetoothServer} with the given \a parent. */
BluetoothServer::BluetoothServer(QObject *parent) :
    TransportInterface(ServerConfiguration(), parent)
{

}

/*! Destructs this \l{BluetoothServer}. */
BluetoothServer::~BluetoothServer()
{
    stopServer();
}

/*! Returns true if a Bleutooth hardware is available. */
bool BluetoothServer::hardwareAvailable()
{
    QBluetoothLocalDevice localDevice;
    return localDevice.isValid();
}

/*! Send \a data to the client with the given \a clientId.*/
void BluetoothServer::sendData(const QUuid &clientId, const QByteArray &data)
{
    QBluetoothSocket *client = 0;
    client = m_clientList.value(clientId);
    if (!client)
        return;

    qCDebug(dcBluetoothServerTraffic()) << "Send data:" << qUtf8Printable(data);
    client->write(data + '\n');
}

/*! Send the given \a data to the \a clients. */
void BluetoothServer::sendData(const QList<QUuid> &clients, const QByteArray &data)
{
    foreach (const QUuid &client, clients)
        sendData(client, data);
}

void BluetoothServer::terminateClientConnection(const QUuid &clientId)
{
    QBluetoothSocket *client = m_clientList.value(clientId);
    if (client) {
        client->close();
    }
}

void BluetoothServer::onHostModeChanged(const QBluetoothLocalDevice::HostMode &mode)
{
    if (!m_server || !m_localDevice)
        return;

    if (mode != QBluetoothLocalDevice::HostDiscoverable) {
        m_localDevice->setHostMode(QBluetoothLocalDevice::HostDiscoverable);
    }
}

void BluetoothServer::onClientConnected()
{
    // Got a new client connected
    QBluetoothSocket *client = m_server->nextPendingConnection();
    if (!client)
        return;

    qCDebug(dcBluetoothServer()) << "New client connected:" << client->peerName() << client->peerAddress().toString();

    QUuid clientId = QUuid::createUuid();
    m_clientList.insert(clientId, client);

    connect(client, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(client, SIGNAL(disconnected()), this, SLOT(onClientDisconnected()));
    connect(client, SIGNAL(stateChanged(QBluetoothSocket::SocketState)), this, SLOT(onClientStateChanged(QBluetoothSocket::SocketState)));
    connect(client, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(onClientError(QBluetoothSocket::SocketError)));

    emit clientConnected(clientId);
}

void BluetoothServer::onClientDisconnected()
{
    QBluetoothSocket *client = qobject_cast<QBluetoothSocket *>(sender());
    if (!client)
        return;

    qCDebug(dcBluetoothServer()) << "Client disconnected:" << client->peerName() << client->peerAddress().toString();
    QUuid clientId = m_clientList.key(client);
    m_clientList.take(clientId)->deleteLater();
    emit clientDisconnected(clientId);
}

void BluetoothServer::onClientError(QBluetoothSocket::SocketError error)
{
    qCWarning(dcBluetoothServer()) << "BluetoothServer: Error occurred:" << error;
}

void BluetoothServer::onClientStateChanged(QBluetoothSocket::SocketState state)
{
    qCDebug(dcBluetoothServer()) << "Client socket state changed:" << state;
}

void BluetoothServer::readData()
{
    QBluetoothSocket *client = qobject_cast<QBluetoothSocket *>(sender());
    if (!client)
        return;

    emit dataAvailable(m_clientList.key(client), client->readAll());
}

bool BluetoothServer::startServer()
{
    if (m_server && m_localDevice)
        return true;

    m_localDevice = new QBluetoothLocalDevice(this);
    if (!m_localDevice->isValid()) {
        qCWarning(dcBluetoothServer()) << "BluetoothServer: could find any bluetooth hardware";
        delete m_localDevice;
        m_localDevice = nullptr;
        return false;
    }

    // Init adapter
    qCDebug(dcBluetoothServer()) << "Using adapter" << m_localDevice->name() << m_localDevice->address().toString();
    m_localDevice->powerOn();
    m_localDevice->setHostMode(QBluetoothLocalDevice::HostDiscoverable);
    connect(m_localDevice, &QBluetoothLocalDevice::hostModeStateChanged, this, &BluetoothServer::onHostModeChanged);

    // Init bluetooth server
    m_server = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(onClientConnected()));
    if (!m_server->listen(m_localDevice->address())) {
        qCWarning(dcBluetoothServer()) << "Could not listen on local device." << m_localDevice->name();
        delete m_localDevice;
        delete m_server;
        m_localDevice = nullptr;
        m_server = nullptr;
        return false;
    }

    // Set service attributes
    QBluetoothServiceInfo::Sequence browseSequence;
    browseSequence << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList, browseSequence);

    QBluetoothServiceInfo::Sequence classId;
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList, classId);
    classId.prepend(QVariant::fromValue(nymeaServiceUuid));

    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList, classId);
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, QVariant("nymea"));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription, QVariant("The JSON-RPC interface for nymea."));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider, QVariant("https://nymea.io"));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::DocumentationUrl, QVariant("https://doc.nymea.io/jsonrpc.html"));

    // Define protocol
    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
             << QVariant::fromValue(quint8(m_server->serverPort()));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);

    // Set UUID
    m_serviceInfo.setServiceUuid(nymeaServiceUuid);

    // Register the service in the local device
    if (!m_serviceInfo.registerService(m_localDevice->address())) {
        qCWarning(dcBluetoothServer()) << "Could not register service" << m_serviceInfo.serviceName() << nymeaServiceUuid.toString();
        delete m_localDevice;
        delete m_server;
        m_localDevice = nullptr;
        m_server = nullptr;
        return false;
    }
    qCDebug(dcBluetoothServer()) << "Started bluetooth server" << m_localDevice->name() << m_localDevice->address().toString() << "Serivce:" << m_serviceInfo.serviceName() << nymeaServiceUuid.toString();

    return true;
}

bool BluetoothServer::stopServer()
{
    foreach (QBluetoothSocket *client, m_clientList.values()) {
        client->close();
    }

    if (m_server) {
        qCDebug(dcBluetoothServer()) << "Shutting down \"Bluetooth server\"";
        m_serviceInfo.unregisterService();
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }

    if (m_localDevice) {
        m_localDevice->deleteLater();
        m_localDevice = nullptr;
    }

    return true;
}


}

#endif // WITH_BLUETOOTH
