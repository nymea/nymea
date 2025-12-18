// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
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

#include "bluetoothserver.h"
#include "loggingcategories.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QJsonDocument>

namespace nymeaserver {

static const QBluetoothUuid nymeaServiceUuid(QUuid("997936b5-d2cd-4c57-b41b-c6048320cd2b"));

/*! Constructs a \l{BluetoothServer} with the given \a parent. */
BluetoothServer::BluetoothServer(QObject *parent)
    : TransportInterface(ServerConfiguration(), parent)
{}

/*! Destructs this \l{BluetoothServer}. */
BluetoothServer::~BluetoothServer()
{
    stopServer();
}

/*! Returns true if a Bleutooth hardware is available. */
bool BluetoothServer::hardwareAvailable()
{
    // QBluetooth hangs for the D-Bus timeout if BlueZ is not available. In order to avoid that, let's first check
    // ourselves if bluez is registered on D-Bus.
    QDBusReply<QStringList> reply = QDBusConnection::systemBus().interface()->registeredServiceNames();
    if (!reply.isValid()) {
        qCWarning(dcBluetoothServer()) << "Unable to query D-Bus for bluez:" << reply.error().message();
        return false;
    }
    const QStringList services = reply.value();
    if (!services.contains("org.bluez")) {
        qCWarning(dcBluetoothServer()) << "BlueZ not found on D-Bus. Skipping Bluetooth initialisation.";
        return false;
    }

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

    connect(client, &QBluetoothSocket::readyRead, this, &BluetoothServer::readData);
    connect(client, &QBluetoothSocket::disconnected, this, &BluetoothServer::onClientDisconnected);
    connect(client, &QBluetoothSocket::stateChanged, this, &BluetoothServer::onClientStateChanged);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(client, &QBluetoothSocket::errorOccurred, this, &BluetoothServer::onClientError);
#else
    connect(client, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(onClientError(QBluetoothSocket::SocketError)));
#endif

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
    connect(m_server, &QBluetoothServer::newConnection, this, &BluetoothServer::onClientConnected);
    if (!m_server->listen(m_localDevice->address())) {
        qCWarning(dcBluetoothServer()) << "Could not listen on local device." << m_localDevice->name();
        delete m_localDevice;
        delete m_server;
        m_localDevice = nullptr;
        m_server = nullptr;
        return false;
    }

    // Set service attributes
    m_serviceInfo = new QBluetoothServiceInfo();

    QBluetoothUuid publicBrowseGroupUuid;
    QBluetoothUuid serialPortUuid;
    QBluetoothUuid rfCommUuid;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    publicBrowseGroupUuid = QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::PublicBrowseGroup);
    serialPortUuid = QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort);
    rfCommUuid = QBluetoothUuid(QBluetoothUuid::ProtocolUuid::Rfcomm);
#else
    publicBrowseGroupUuid = QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup);
    serialPortUuid = QBluetoothUuid(QBluetoothUuid::SerialPort);
    rfCommUuid = QBluetoothUuid(QBluetoothUuid::Rfcomm);
#endif

    QBluetoothServiceInfo::Sequence browseSequence;
    browseSequence << QVariant::fromValue(publicBrowseGroupUuid);
    m_serviceInfo->setAttribute(QBluetoothServiceInfo::BrowseGroupList, browseSequence);

    QBluetoothServiceInfo::Sequence classId;
    classId << QVariant::fromValue(serialPortUuid);
    m_serviceInfo->setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList, classId);
    classId.prepend(QVariant::fromValue(nymeaServiceUuid));

    m_serviceInfo->setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);
    m_serviceInfo->setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList, classId);
    m_serviceInfo->setAttribute(QBluetoothServiceInfo::ServiceName, QVariant("nymea"));
    m_serviceInfo->setAttribute(QBluetoothServiceInfo::ServiceDescription, QVariant("The JSON-RPC interface for nymea."));
    m_serviceInfo->setAttribute(QBluetoothServiceInfo::ServiceProvider, QVariant("https://nymea.io"));
    m_serviceInfo->setAttribute(QBluetoothServiceInfo::DocumentationUrl, QVariant("https://doc.nymea.io/jsonrpc.html"));

    // Define protocol
    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
    protocol << QVariant::fromValue(rfCommUuid) << QVariant::fromValue(quint8(m_server->serverPort()));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    m_serviceInfo->setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);

    // Set UUID
    m_serviceInfo->setServiceUuid(nymeaServiceUuid);

    // Register the service in the local device
    if (!m_serviceInfo->registerService(m_localDevice->address())) {
        qCWarning(dcBluetoothServer()) << "Could not register service" << m_serviceInfo->serviceName() << nymeaServiceUuid.toString();
        delete m_serviceInfo;
        delete m_localDevice;
        delete m_server;
        m_localDevice = nullptr;
        m_server = nullptr;
        return false;
    }
    qCDebug(dcBluetoothServer()) << "Started bluetooth server" << m_localDevice->name() << m_localDevice->address().toString() << "Serivce:" << m_serviceInfo->serviceName()
                                 << nymeaServiceUuid.toString();

    return true;
}

bool BluetoothServer::stopServer()
{
    foreach (QBluetoothSocket *client, m_clientList.values()) {
        client->close();
    }

    if (m_serviceInfo) {
        m_serviceInfo->unregisterService();
        delete m_serviceInfo;
    }

    if (m_server) {
        qCDebug(dcBluetoothServer()) << "Shutting down \"Bluetooth server\"";
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

} // namespace nymeaserver
