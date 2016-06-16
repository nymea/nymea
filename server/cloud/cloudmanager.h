/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#ifndef CLOUDMANAGER_H
#define CLOUDMANAGER_H

#include <QObject>


#include "transportinterface.h"
#include "cloudinterface.h"
#include "cloudauthenticator.h"
#include "cloudconnection.h"

namespace guhserver {

class CloudManager : public TransportInterface
{
    Q_OBJECT
public:
    friend class CloudInterface;
    friend class CloudConnectionHandler;
    friend class CloudAuthenticationHandler;

    explicit CloudManager(QObject *parent = 0);
    ~CloudManager();

    void connectToCloud(const QString &username, const QString &password) ;

    void sendData(const QUuid &clientId, const QVariantMap &data) override;
    void sendData(const QList<QUuid> &clients, const QVariantMap &data) override;

    bool enabled() const;
    bool connected() const;
    bool connectionAuthenticated() const;
    bool active() const;
    bool authenticated() const;

public slots:
    bool startServer() override;
    bool stopServer() override;

private:
    CloudConnection *m_cloudConnection;
    CloudInterface *m_interface;

    QList<QUuid> m_clients;

    QHash<int, CloudJsonReply *> m_replies;

    QHash<QUuid, QUuid> m_tunnelClients;

    QUuid m_connectionId;
    bool m_enabled;
    bool m_active;
    bool m_authenticated;

protected:
    void sendCloudData(const QVariantMap &data);

    void onConnectionAuthentificationFinished(const bool &status, const QUuid &connectionId);
    void onTunnelAdded(const QUuid &tunnelId, const QUuid &serverId, const QUuid &clientId);
    void onTunnelRemoved(const QUuid &tunnelId);
    void onCloudDataReceived(const QUuid &tunnelId, const QVariantMap &data);

signals:
    void enabledChanged();
    void connectedChanged();
    void activeChanged();
    void authenticatedChanged();

    // Transport interface signals
    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);
    void dataAvailable(const QUuid &clientId, const QString &targetNamespace, const QString &method, const QVariantMap &message);

private slots:
    void onConnectedChanged();
    void onAuthenticatedChanged();

};

}

#endif // CLOUDMANAGER_H
