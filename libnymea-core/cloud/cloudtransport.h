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

#ifndef CLOUDTRANSPORT_H
#define CLOUDTRANSPORT_H

#include <QObject>
#include "../transportinterface.h"
#include "remoteproxyconnection.h"

namespace nymeaserver {

class CloudTransport : public TransportInterface
{
    Q_OBJECT
public:
    explicit CloudTransport(const ServerConfiguration &config, QObject *parent = nullptr);

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void sendData(const QList<QUuid> &clientIds, const QByteArray &data) override;

    void terminateClientConnection(const QUuid &clientId) override;

    bool startServer() override;
    bool stopServer() override;

signals:

public slots:
    void connectToCloud(const QString &token, const QString &nonce, const QString &serverUrl);

private slots:
    void remoteConnectionStateChanged(remoteproxyclient::RemoteProxyConnection::State state);
    void transportConnected();
    void transportReady();
    void transportDataReady(const QByteArray &data);
    void transportDisconnected();

private:
    QString m_defaultProxyUrl;

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
