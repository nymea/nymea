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

#ifndef CLOUDMANAGER_H
#define CLOUDMANAGER_H

#include <QObject>

#include "cloud.h"
#include "cloudinterface.h"
#include "cloudconnection.h"
#include "transportinterface.h"
#include "cloudauthenticator.h"

namespace guhserver {

class CloudManager : public TransportInterface
{
    Q_OBJECT

public:
    friend class CloudInterface;
    friend class CloudConnectionHandler;
    friend class CloudAuthenticationHandler;

    explicit CloudManager(const bool &enabled, const QUrl &authenticationServerUrl, const QUrl &proxyServerUrl, QObject *parent = 0);
    ~CloudManager();

    void connectToCloud(const QString &username, const QString &password) ;

    void sendData(const QUuid &clientId, const QVariantMap &data) override;
    void sendData(const QList<QUuid> &clients, const QVariantMap &data) override;

    bool enabled() const;
    bool connected() const;
    bool active() const;
    bool authenticated() const;

public slots:
    bool startServer() override;
    bool stopServer() override;

    void onCloudEnabledChanged();
    void onAuthenticationServerUrlChanged();
    void onProxyServerUrlChanged();

private:
    CloudConnection *m_cloudConnection;
    CloudInterface *m_interface;

    QHash<QUuid, QUuid> m_tunnelClients; // clientId | tunnelId

    QHash<int, CloudJsonReply *> m_replies;

    QUuid m_connectionId;

    bool m_enabled;
    bool m_active;
    bool m_authenticated;

    bool m_runningAuthentication;

    void setEnabled(const bool &enabled);
    void setActive(const bool &active);
    void setAuthenticated(const bool &authenticated);

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

    void authenticationFinished(const Cloud::CloudError &error);

    // Transport interface signals
    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);
    void dataAvailable(const QUuid &clientId, const QString &targetNamespace, const QString &method, const QVariantMap &message);

private slots:
    void onConnectedChanged();
    void onAuthenticatedChanged();

    //void authenticationProcessFinished(const bool &success, const CloudConnection::CloudConnectionError error);

};

}

#endif // CLOUDMANAGER_H
