// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2026, chargebyte austria GmbH
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
