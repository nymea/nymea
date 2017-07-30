/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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
    \class guhserver::BluetoothServer
    \brief This class represents the bluetooth server for guhd.

    \ingroup server
    \inmodule core

    \inherits TransportInterface

    The bluetooth server allows clients to connect to the JSON-RPC API using an RFCOMM bluetooth connection. If the server is enabled, a client
    can discover the services running on this host. The service for the JSON-RPC api is called \tt guhIO and has the uuid \tt 997936b5-d2cd-4c57-b41b-c6048320cd2b .

    \sa TransportInterface
*/


#include "bluetoothserver.h"
#include "loggingcategories.h"

#include <QJsonDocument>
#include <QBluetoothLocalDevice>

namespace guhserver {

static const QBluetoothUuid serviceUuid(QUuid("997936b5-d2cd-4c57-b41b-c6048320cd2b"));
/*! Constructs a \l{BluetoothServer} with the given \a parent. */
BluetoothServer::BluetoothServer(QObject *parent) :
    TransportInterface(parent),
    m_server(0)
{

}

/*! Destructs this \l{BluetoothServer}. */
BluetoothServer::~BluetoothServer()
{
    if (m_server && m_server->isListening())
        qCDebug(dcApplication) << "Shutting down \"Bluetooth server\"";

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
    if (client)
        client->write(QJsonDocument::fromVariant(data).toJson(QJsonDocument::Compact) + '\n');
}

/*! Send the given \a data to the \a clients. */
void BluetoothServer::sendData(const QList<QUuid> &clients, const QByteArray &data)
{
    foreach (const QUuid &client, clients)
        sendData(client, data);
}

void BluetoothServer::onClientConnected()
{
    // Got a new client connected
    QBluetoothSocket *client = m_server->nextPendingConnection();
    if (!client)
        return;

    qCDebug(dcConnection) << "Bluetooth server: new client connected:" << client->localName() << client->localAddress().toString();

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

    qCDebug(dcConnection) << "Bluetooth server: client disconnected:" << client->localName() << client->localAddress().toString();
    QUuid clientId = m_clientList.key(client);
    m_clientList.take(clientId)->deleteLater();
}

void BluetoothServer::onError(QBluetoothSocket::SocketError error)
{
    qCWarning(dcConnection) << "Bluetooth server error:" << error;
}

void BluetoothServer::readData()
{
    QBluetoothSocket *client = qobject_cast<QBluetoothSocket *>(sender());
    if (!client)
        return;

    QByteArray message;
    while (client->canReadLine()) {
        QByteArray dataLine = client->readLine();
        message.append(dataLine);
        if (dataLine.endsWith('\n')) {
            qCDebug(dcConnection()) << "Bluetooth data received:" << message;
            emit dataAvailable(m_clientList.key(client), message);
            message.clear();
        }
    }
}

bool BluetoothServer::startServer()
{
    if (m_server)
        return true;

    // Check if Bluetooth is available on this device
    QBluetoothLocalDevice localDevice;
    if (localDevice.isValid()) {
        qCDebug(dcConnection()) << "Bluetooth: using adapter" << localDevice.name() << localDevice.address().toString();
        localDevice.powerOn();
        localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
    } else {
        qCWarning(dcConnection()) << "Bluetooth server: could find any bluetooth hardware";
        return false;
    }

    m_server = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(onClientConnected()));
    if (!m_server->listen(localDevice.address())) {
        qCWarning(dcConnection()) << "Bluetooth server: could not listen on local device." << localDevice.name();
        return false;
    }

    qCDebug(dcConnection) << "Started bluetooth server" << m_server->serverAddress().toString();

    // Set service attributes
    QBluetoothServiceInfo::Sequence browseSequence;
    browseSequence << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList, browseSequence);

    QBluetoothServiceInfo::Sequence classId;
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList, classId);
    classId.prepend(QVariant::fromValue(serviceUuid));

    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);

    m_serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,classId);
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, QVariant("guhIO"));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription, QVariant("The JSON-RPC interface for guhIO."));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider, QVariant("https://guh.io"));

    // Define protocol
    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
             << QVariant::fromValue(quint8(m_server->serverPort()));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);

    // Set UUID
    m_serviceInfo.setServiceUuid(serviceUuid);

    // Register the service in the local device
    if (!m_serviceInfo.registerService(localDevice.address())) {
        qCWarning(dcConnection()) << "Bluetooth: Could not register service" << m_serviceInfo.serviceName() << serviceUuid.toString();
        return false;
    }

    qCDebug(dcConnection()) << "Bluetooth: Registered successfully service" << m_serviceInfo.serviceName() << serviceUuid.toString();
    return true;
}

bool BluetoothServer::stopServer()
{
    foreach (QBluetoothSocket *client, m_clientList.values()) {
        client->close();
    }

    if (!m_server)
        return true;

    m_serviceInfo.unregisterService();

    m_server->close();
    m_server->deleteLater();
    m_server = 0;
    return true;
}


}
