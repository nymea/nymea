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

#ifndef GUHCONFIGURATION_H
#define GUHCONFIGURATION_H

#include <QObject>
#include <QUuid>
#include <QUrl>

namespace guhserver {

class GuhConfiguration : public QObject
{
    Q_OBJECT
public:
    explicit GuhConfiguration(QObject *parent = 0);

    // Global settings
    QUuid serverUuid() const;

    QString serverName() const;
    void setServerName(const QString &serverName);

    QByteArray timeZone() const;
    void setTimeZone(const QByteArray &timeZone);

    // TCP server
    uint tcpServerPort() const;
    void setTcpServerPort(const uint &port);

    // Webserver
    uint webserverPort() const;
    void setWebserverPort(const uint &port);

    // Websocket
    uint websocketPort() const;
    void setWebsocketPort(const uint &port);

    // SSL configuration
    bool sslEnabled() const;
    void setSslEnabled(const bool &sslEnabled);

    QString sslCertificate() const;
    QString sslCertificateKey() const;
    void setSslCertificate(const QString &sslCertificate, const QString &sslCertificateKey);

    // Cloud
    bool cloudEnabled() const;
    void setCloudEnabled(const bool &enabled);

    QUrl cloudAuthenticationServer() const;
    void setCloudAuthenticationServer(const QUrl &authenticationServer);

    QUrl cloudProxyServer() const;
    void setCloudProxyServer(const QUrl &cloudProxyServer);

private:
    QUuid m_serverUuid;
    QString m_serverName;
    QByteArray m_timeZone;

    uint m_tcpServerPort;
    uint m_webserverPort;
    uint m_websocketPort;

    bool m_sslEnabled;
    QString m_sslCertificate;
    QString m_sslCertificateKey;

    bool m_cloudEnabled;
    QUrl m_cloudAuthenticationServer;
    QUrl m_cloudProxyServer;

signals:
    void serverNameChanged();
    void timeZoneChanged();

    void tcpServerPortChanged();
    void webserverPortChanged();
    void websocketPortChanged();

    void sslEnabledChanged();
    void sslCertificateChanged();

    void cloudEnabledChanged();
    void cloudAuthenticationServerChanged();
    void cloudProxyServerChanged();
};

}

#endif // GUHCONFIGURATION_H
