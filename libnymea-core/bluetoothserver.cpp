/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016-2018 Simon Stürz <simon.stuerz@guh.io>              *
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

    client->write(data + '\n');
}

/*! Send the given \a data to the \a clients. */
void BluetoothServer::sendData(const QList<QUuid> &clients, const QByteArray &data)
{
    foreach (const QUuid &client, clients)
        sendData(client, data);
}

void BluetoothServer::onHostModeChanged(const QBluetoothLocalDevice::HostMode &mode)
{
    if (!m_server || !m_localDevice)
        return;

    if (mode != QBluetoothLocalDevice::HostDiscoverable || QBluetoothLocalDevice::HostDiscoverableLimitedInquiry) {
        m_localDevice->powerOn();
        m_localDevice->setHostMode(QBluetoothLocalDevice::HostDiscoverableLimitedInquiry);
    }
}

void BluetoothServer::onClientConnected()
{
    // Got a new client connected
    QBluetoothSocket *client = m_server->nextPendingConnection();
    if (!client)
        return;

    qCDebug(dcConnection) << "BluetoothServer: New client connected:" << client->localName() << client->localAddress().toString();

    QUuid clientId = QUuid::createUuid();
    m_clientList.insert(clientId, client);

    connect(client, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(client, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(onError(QBluetoothSocket::SocketError)));
    connect(client, SIGNAL(disconnected()), this, SLOT(onClientDisconnected()));

    emit clientConnected(clientId);
}

void BluetoothServer::onClientDisconnected()
{
    QBluetoothSocket *client = qobject_cast<QBluetoothSocket *>(sender());
    if (!client)
        return;

    qCDebug(dcConnection) << "BluetoothServer: Client disconnected:" << client->localName() << client->localAddress().toString();
    QUuid clientId = m_clientList.key(client);
    m_clientList.take(clientId)->deleteLater();
}

void BluetoothServer::onError(QBluetoothSocket::SocketError error)
{
    qCWarning(dcConnection) << "BluetoothServer: Error occured:" << error;
}

void BluetoothServer::readData()
{
    QBluetoothSocket *client = qobject_cast<QBluetoothSocket *>(sender());
    if (!client)
        return;

    m_receiveBuffer.append(client->readAll());
    int splitIndex = m_receiveBuffer.indexOf("}\n{");
    while (splitIndex > -1) {
        emit dataAvailable(m_clientList.key(client), m_receiveBuffer.left(splitIndex + 1));
        m_receiveBuffer = m_receiveBuffer.right(m_receiveBuffer.length() - splitIndex - 2);
        splitIndex = m_receiveBuffer.indexOf("}\n{");
    }
    if (m_receiveBuffer.endsWith("}\n")) {
        emit dataAvailable(m_clientList.key(client), m_receiveBuffer);
        m_receiveBuffer.clear();
    }
}

bool BluetoothServer::startServer()
{
    if (m_server && m_localDevice)
        return true;

    m_localDevice = new QBluetoothLocalDevice(this);
    if (!m_localDevice->isValid()) {
        qCWarning(dcConnection()) << "BluetoothServer: could find any bluetooth hardware";
        delete m_localDevice;
        m_localDevice = nullptr;
        return false;
    }

    // Init adapter
    qCDebug(dcConnection()) << "BluetoothServer: Using adapter" << m_localDevice->name() << m_localDevice->address().toString();
    m_localDevice->powerOn();
    m_localDevice->setHostMode(QBluetoothLocalDevice::HostDiscoverableLimitedInquiry);
    connect(m_localDevice, &QBluetoothLocalDevice::hostModeStateChanged, this, &BluetoothServer::onHostModeChanged);

    // Init bluetooth server
    m_server = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(onClientConnected()));
    if (!m_server->listen(m_localDevice->address())) {
        qCWarning(dcConnection()) << "BluetoothServer: Could not listen on local device." << m_localDevice->name();
        delete m_localDevice;
        delete m_server;
        m_localDevice = nullptr;
        m_server = nullptr;
        return false;
    }

    qCDebug(dcConnection) << "BluetoothServer: Started bluetooth server" << m_server->serverAddress().toString();

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
        qCWarning(dcConnection()) << "BluetoothServer: Could not register service" << m_serviceInfo.serviceName() << nymeaServiceUuid.toString();
        delete m_localDevice;
        delete m_server;
        m_localDevice = nullptr;
        m_server = nullptr;
        return false;
    }

    qCDebug(dcConnection()) << "BluetoothServer: Registered successfully service" << m_serviceInfo.serviceName() << nymeaServiceUuid.toString();
    return true;
}

bool BluetoothServer::stopServer()
{
    foreach (QBluetoothSocket *client, m_clientList.values()) {
        client->close();
    }

    if (m_server) {
        qCDebug(dcBluetooth()) << "Shutting down \"Bluetooth server\"";
        m_serviceInfo.unregisterService();
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }

    if (m_localDevice) {
        m_localDevice->deleteLater();
        m_localDevice = nullptr;
    }

    m_receiveBuffer.clear();
    return true;
}


}
