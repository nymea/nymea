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

#include "guhconfiguration.h"
#include "guhsettings.h"

#include <QTimeZone>

namespace guhserver {

GuhConfiguration::GuhConfiguration(QObject *parent) :
    QObject(parent)
{
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);

    // guhd
    settings.beginGroup("guhd");
    m_serverName = settings.value("name", "guhIO").toString();
    m_timeZone = settings.value("timeZone", QTimeZone::systemTimeZoneId()).toByteArray();
    m_serverUuid = settings.value("uuid", QUuid()).toUuid();
    if (m_serverUuid.isNull()) {
        m_serverUuid = QUuid::createUuid();
        settings.setValue("uuid", m_serverUuid);
    }
    settings.endGroup();

    // TcpServer
    settings.beginGroup("TcpServer");
    m_tcpServerPort = settings.value("port", 2222).toUInt();
    settings.endGroup();

    // Webserver
    settings.beginGroup("Webserver");
    m_webserverPort = settings.value("port", 3333).toUInt();
    settings.endGroup();

    // Websocket server
    settings.beginGroup("WebsocketServer");
    m_websocketPort = settings.value("port", 4444).toUInt();
    settings.endGroup();

    // SSL configuration
    settings.beginGroup("SSL");
    m_sslEnabled = settings.value("enabled", false).toBool();
    m_sslCertificate = settings.value("certificate", "/etc/ssl/certs/guhd-certificate.crt").toString();
    m_sslCertificateKey = settings.value("certificate-key", "/etc/ssl/certs/guhd-certificate.key").toString();
    settings.endGroup();

    // Cloud
    settings.beginGroup("Cloud");
    m_cloudEnabled = settings.value("enabled", false).toBool();
    m_cloudAuthenticationServer = settings.value("authenticationServer", QUrl("http://localhost:8000/oauth2/token")).toUrl();
    m_cloudProxyServer = settings.value("proxyServer", QUrl("ws://127.0.0.1:1212")).toUrl();
    settings.endGroup();
}

QUuid GuhConfiguration::serverUuid() const
{
    return m_serverUuid;
}

QString GuhConfiguration::serverName() const
{
    return m_serverName;
}

void GuhConfiguration::setServerName(const QString &serverName)
{
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("guhd");
    settings.setValue("name", serverName);
    settings.endGroup();

    m_serverName = serverName;
    emit serverNameChanged();
}

QByteArray GuhConfiguration::timeZone() const
{
    return m_timeZone;
}

void GuhConfiguration::setTimeZone(const QByteArray &timeZone)
{
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("guhd");
    settings.setValue("timeZone", timeZone);
    settings.endGroup();

    m_timeZone = timeZone;
    emit timeZoneChanged();
}

uint GuhConfiguration::tcpServerPort() const
{
    return m_tcpServerPort;
}

void GuhConfiguration::setTcpServerPort(const uint &port)
{
    m_tcpServerPort = port;
    emit tcpServerPortChanged();
}

uint GuhConfiguration::webserverPort() const
{
    return m_webserverPort;
}

void GuhConfiguration::setWebserverPort(const uint &port)
{
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("Webserver");
    settings.setValue("port", port);
    settings.endGroup();

    m_webserverPort = port;
    emit webserverPortChanged();
}

uint GuhConfiguration::websocketPort() const
{
    return m_websocketPort;
}

void GuhConfiguration::setWebsocketPort(const uint &port)
{
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("WebsocketServer");
    settings.setValue("port", port);
    settings.endGroup();

    m_websocketPort = port;
    emit websocketPortChanged();
}

bool GuhConfiguration::sslEnabled() const
{
    return m_sslEnabled;
}

void GuhConfiguration::setSslEnabled(const bool &sslEnabled)
{
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("SSL");
    settings.setValue("enabled", sslEnabled);
    settings.endGroup();

    m_sslEnabled = sslEnabled;
    emit sslEnabledChanged();
}

QString GuhConfiguration::sslCertificate() const
{
    return m_sslCertificate;
}

QString GuhConfiguration::sslCertificateKey() const
{
    return m_sslCertificateKey;
}

void GuhConfiguration::setSslCertificate(const QString &sslCertificate, const QString &sslCertificateKey)
{
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("SSL");
    settings.setValue("certificate", sslCertificate);
    settings.setValue("certificate-key", sslCertificateKey);
    settings.endGroup();

    m_sslCertificate = sslCertificate;
    m_sslCertificateKey = sslCertificateKey;

    emit sslCertificateChanged();
}

bool GuhConfiguration::cloudEnabled() const
{
    return m_cloudEnabled;
}

void GuhConfiguration::setCloudEnabled(const bool &enabled)
{
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    settings.setValue("enabled", enabled);
    settings.endGroup();

    m_cloudEnabled = enabled;
    emit cloudEnabledChanged();
}

QUrl GuhConfiguration::cloudAuthenticationServer() const
{
    return m_cloudAuthenticationServer;
}

void GuhConfiguration::setCloudAuthenticationServer(const QUrl &authenticationServer)
{
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    settings.setValue("authenticationServer", authenticationServer.toString());
    settings.endGroup();

    m_cloudAuthenticationServer = authenticationServer;
    emit cloudAuthenticationServerChanged();
}

QUrl GuhConfiguration::cloudProxyServer() const
{
    return m_cloudProxyServer;
}

void GuhConfiguration::setCloudProxyServer(const QUrl &cloudProxyServer)
{
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    settings.setValue("proxyServer", cloudProxyServer.toString());
    settings.endGroup();

    m_cloudProxyServer = cloudProxyServer;
    emit cloudProxyServerChanged();
}

}
