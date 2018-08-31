/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Michael Zanetti <michael.zanetti@guh.io>            *
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

#ifndef CLOUDTRANSPORT_H
#define CLOUDTRANSPORT_H

#include <QObject>
#include "../transportinterface.h"
#include "nymea-remoteproxyclient/remoteproxyconnection.h"

namespace nymeaserver {

class CloudTransport : public TransportInterface
{
    Q_OBJECT
public:
    explicit CloudTransport(const ServerConfiguration &config, QObject *parent = nullptr);

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void sendData(const QList<QUuid> &clientIds, const QByteArray &data) override;

    bool startServer() override;
    bool stopServer() override;

signals:

public slots:
    void connectToCloud(const QString &token, const QString &nonce);
    void remoteConnectionStateChanged(remoteproxyclient::RemoteProxyConnection::State state);

private slots:
    void transportReady();
    void transportDataReady(const QByteArray &data);

private:
    QUrl m_proxyUrl;

    class ConnectionContext {
    public:
        QUuid clientId;
        QString token;
        QString nonce;
        remoteproxyclient::RemoteProxyConnection* proxyConnection;
    };
    QHash<remoteproxyclient::RemoteProxyConnection*, ConnectionContext> m_connections;

};

}

#endif // CLOUDTRANSPORT_H
