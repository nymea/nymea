#include "bluetoothserver.h"
#include "loggingcategories.h"

#include <QJsonDocument>
#include <QBluetoothLocalDevice>

namespace guhserver {

static const QLatin1String serviceUuid("81679f09-1404-4242-b685-a7f7e23df8cf");

BluetoothServer::BluetoothServer(QObject *parent) :
    TransportInterface(parent),
    m_server(0)
{

}

BluetoothServer::~BluetoothServer()
{
    stopServer();
}

bool BluetoothServer::hardwareAvailable()
{
    QBluetoothLocalDevice localDevice;
    return localDevice.isValid();
}

void BluetoothServer::sendData(const QUuid &clientId, const QVariantMap &data)
{
    QBluetoothSocket *client = 0;
    client = m_clientList.value(clientId);
    if (client)
        client->write(QJsonDocument::fromVariant(data).toJson() + "\n");
}

void BluetoothServer::sendData(const QList<QUuid> &clients, const QVariantMap &data)
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

    while (client->canReadLine()) {
        QByteArray line = client->readLine().trimmed();
        qCDebug(dcConnection()) << "Bluetooth server: line in:" << QString::fromUtf8(line.constData(), line.length());
    }
}

bool BluetoothServer::startServer()
{
    if (m_server)
        return true;

    // Check if Bluetooth is available on this device
    QBluetoothLocalDevice localDevice;
    if (localDevice.isValid()) {
        // Turn Bluetooth on
        localDevice.powerOn();
        // Make it visible to others
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

    QBluetoothServiceInfo::Sequence classId;
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList, classId);

    classId.prepend(QVariant::fromValue(QBluetoothUuid(serviceUuid)));

    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,classId);
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, tr("guhIO JSON-RPC"));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription, tr("The JSON-RPC interface for guhIO."));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider, "guh.io");
    m_serviceInfo.setServiceUuid(QBluetoothUuid(serviceUuid));

    QBluetoothServiceInfo::Sequence publicBrowse;
    publicBrowse << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList, publicBrowse);

    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    protocol.clear();
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
             << QVariant::fromValue(quint8(m_server->serverPort()));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);
    m_serviceInfo.registerService(localDevice.address());
    return true;
}

bool BluetoothServer::stopServer()
{
    foreach (QBluetoothSocket *client, m_clientList.values()) {
        client->close();
    }

    m_server->close();
    m_server->deleteLater();
    m_server = 0;
    return true;
}

}
