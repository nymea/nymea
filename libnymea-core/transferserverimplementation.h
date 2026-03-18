// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TRANSFERSERVERIMPLEMENTATION_H
#define TRANSFERSERVERIMPLEMENTATION_H

#include <QObject>
#include <QHash>
#include <QUuid>

#include "transportinterface.h"
#include "transfermanager.h"

namespace nymeaserver {

class TransferServerImplementation : public QObject
{
    Q_OBJECT
public:
    explicit TransferServerImplementation(TransferManager *transferManager, QObject *parent = nullptr);

    void registerTransportInterface(TransportInterface *interface);
    void unregisterTransportInterface(TransportInterface *interface);

private slots:
    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);
    void processData(const QUuid &clientId, const QByteArray &data);

private:
    struct ClientState {
        TransportInterface *transport = nullptr;
        QByteArray buffer;
        QString transferId;
        bool connected = false;
    };

    void processPacket(TransportInterface *interface, const QUuid &clientId, const QByteArray &data);
    void sendResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QVariantMap &params = QVariantMap());
    void sendErrorResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QString &error);

    TransferManager *m_transferManager = nullptr;
    QHash<QUuid, ClientState> m_clients;
};

}

#endif // TRANSFERSERVERIMPLEMENTATION_H
