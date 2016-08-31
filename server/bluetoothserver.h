#ifndef BLUETOOTHSERVER_H
#define BLUETOOTHSERVER_H

#include <QObject>
#include <QBluetoothSocket>
#include <QBluetoothServer>

#include "transportinterface.h"

namespace guhserver {

class BluetoothServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit BluetoothServer(QObject *parent = 0);
    ~BluetoothServer();

    static bool hardwareAvailable();

    void sendData(const QUuid &clientId, const QVariantMap &data) override;
    void sendData(const QList<QUuid> &clients, const QVariantMap &data) override;

private:
    QBluetoothServer *m_server;
    QBluetoothServiceInfo m_serviceInfo;
    QHash<QUuid, QBluetoothSocket *> m_clientList;

private slots:
    void onClientConnected();
    void onClientDisconnected();
    void onError(QBluetoothSocket::SocketError error);
    void readData();

public slots:
    bool startServer() override;
    bool stopServer() override;

};

}

#endif // BLUETOOTHSERVER_H
