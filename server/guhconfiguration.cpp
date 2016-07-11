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

#include "loggingcategories.h"
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

    qCDebug(dcApplication()) << "Configuration: Server name:" << m_serverName;
    qCDebug(dcApplication()) << "Configuration: Server uuid:" << m_serverUuid.toString();
    qCDebug(dcApplication()) << "Configuration: Server time zone:" << QString::fromUtf8(m_timeZone);

    // TcpServer
    settings.beginGroup("TcpServer");
    m_tcpServerAddress = QHostAddress(settings.value("address", "0.0.0.0").toString());
    m_tcpServerPort = settings.value("port", 2222).toUInt();
    settings.endGroup();

    qCDebug(dcApplication()) << "Configuration: TCP server:" << QString("%1:%2").arg(m_tcpServerAddress.toString()).arg(m_tcpServerPort);

    // Webserver
    settings.beginGroup("WebServer");
    m_webServerAddress = QHostAddress(settings.value("address", "0.0.0.0").toString());
    m_webServerPort = settings.value("port", 3333).toUInt();
    m_webServerPublicFolder = settings.value("publicFolder", "/usr/share/guh-webinterface/public/").toString();
    settings.endGroup();

    qCDebug(dcApplication()) << "Configuration: Webserver:" << QString("%1:%2 on %3").arg(m_webServerAddress.toString()).arg(m_webServerPort).arg(m_webServerPublicFolder);

    // Websocket server
    settings.beginGroup("WebSocketServer");
    m_webSocketAddress = QHostAddress(settings.value("address", "0.0.0.0").toString());
    m_webSocketPort = settings.value("port", 4444).toUInt();
    settings.endGroup();

    qCDebug(dcApplication()) << "Configuration: Websocket server:" << QString("%1:%2").arg(m_webSocketAddress.toString()).arg(m_webSocketPort);

    // SSL configuration
    settings.beginGroup("SSL");
    m_sslEnabled = settings.value("enabled", false).toBool();
    m_sslCertificate = settings.value("certificate", "/etc/ssl/certs/guhd-certificate.crt").toString();
    m_sslCertificateKey = settings.value("certificate-key", "/etc/ssl/certs/guhd-certificate.key").toString();
    settings.endGroup();

    qCDebug(dcApplication()) << "Configuration: SSL" << (m_sslEnabled ? "enabled" : "disabled");

    // Cloud
    settings.beginGroup("Cloud");
    m_cloudEnabled = settings.value("enabled", false).toBool();
    m_cloudAuthenticationServer = settings.value("authenticationServer", QUrl("http://localhost:8000/oauth2/token")).toUrl();
    m_cloudProxyServer = settings.value("proxyServer", QUrl("ws://127.0.0.1:1212")).toUrl();
    settings.endGroup();

    qCDebug(dcApplication()) << "Configuration: Cloud connection" << (m_cloudEnabled ? "enabled" : "disabled");
    qCDebug(dcApplication()) << "Configuration: Cloud authentication server" << m_cloudAuthenticationServer.toString();
    qCDebug(dcApplication()) << "Configuration: Cloud proxy server" << m_cloudProxyServer.toString();

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
    qCDebug(dcApplication()) << "Configuration: set server name:" << serverName;

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
    qCDebug(dcApplication()) << "Configuration: set time zone:" << QString::fromUtf8(timeZone);

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

QHostAddress GuhConfiguration::tcpServerAddress() const
{
    return m_tcpServerAddress;
}

void GuhConfiguration::setTcpServerConfiguration(const uint &port, const QHostAddress &address)
{
    qCDebug(dcApplication()) << QString("Configuration: set TCP server %1:%2").arg(address.toString()).arg(port);

    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("TcpServer");
    settings.setValue("address", address.toString());
    settings.setValue("port", port);
    settings.endGroup();

    m_tcpServerPort = port;
    m_tcpServerAddress = address;

    emit tcpServerConfigurationChanged();
}

uint GuhConfiguration::webServerPort() const
{
    return m_webServerPort;
}

QHostAddress GuhConfiguration::webServerAddress() const
{
    return m_webServerAddress;
}

QString GuhConfiguration::webServerPublicFolder() const
{
    return m_webServerPublicFolder;
}

void GuhConfiguration::setWebServerConfiguration(const uint &port, const QHostAddress &address)
{
    qCDebug(dcApplication()) << QString("Configuration: set web server %1:%2").arg(address.toString()).arg(port);

    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("WebServer");
    settings.setValue("address", address.toString());
    settings.setValue("port", port);
    settings.endGroup();

    m_webServerPort = port;
    m_webServerAddress = address;

    emit webServerConfigurationChanged();
}

uint GuhConfiguration::webSocketPort() const
{
    return m_webSocketPort;
}

QHostAddress GuhConfiguration::webSocketAddress() const
{
    return m_webSocketAddress;
}

void GuhConfiguration::setWebSocketConfiguration(const uint &port, const QHostAddress &address)
{
    qCDebug(dcApplication()) << QString("Configuration: set websocket server %1:%2").arg(address.toString()).arg(port);

    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("WebSocketServer");
    settings.setValue("address", address.toString());
    settings.setValue("port", port);
    settings.endGroup();

    m_webSocketAddress = address;
    m_webSocketPort = port;

    emit webSocketServerConfigurationChanged();
}

bool GuhConfiguration::sslEnabled() const
{
    return m_sslEnabled;
}

void GuhConfiguration::setSslEnabled(const bool &sslEnabled)
{
    qCDebug(dcApplication()) << "Configuration: SSL" << (sslEnabled ? "enabled" : "disabled");

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
    qCDebug(dcApplication()) << "Configuration: SSL certificate:" << sslCertificate << endl << "SSL certificate key:" << sslCertificateKey;

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
    qCDebug(dcApplication()) << "Configuration: Cloud connection" << (enabled ? "enabled" : "disabled");

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
    qCDebug(dcApplication()) << "Configuration: Cloud authentication server" << authenticationServer.toString();

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
    qCDebug(dcApplication()) << "Configuration: Cloud proxy server" << cloudProxyServer.toString();

    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    settings.setValue("proxyServer", cloudProxyServer.toString());
    settings.endGroup();

    m_cloudProxyServer = cloudProxyServer;
    emit cloudProxyServerChanged();
}

}
