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

#include "loggingcategories.h"
#include "guhconfiguration.h"
#include "guhsettings.h"

#include <QTimeZone>
#include <QCoreApplication>

namespace guhserver {

GuhConfiguration::GuhConfiguration(QObject *parent) :
    QObject(parent)
{
    // Load guhd settings
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("guhd");
    setServerName(settings.value("name", "guhIO").toString());
    QUuid serverUuid = settings.value("uuid", QUuid()).toUuid();
    if (serverUuid.isNull())
        serverUuid = QUuid::createUuid();

    setServerUuid(serverUuid);
    setTimeZone(settings.value("timeZone", QTimeZone::systemTimeZoneId()).toByteArray());
    settings.endGroup();

    // TcpServer
    settings.beginGroup("TcpServer");
    // Defaults for tests
    QHostAddress tcpServerAddress("127.0.0.1");
    uint tcpServerPort = 2222;
#ifndef TESTING_ENABLED
    tcpServerAddress = QHostAddress(settings.value("address", "0.0.0.0").toString());
    tcpServerPort = settings.value("port", 2222).toUInt();
#endif
    setTcpServerConfiguration(tcpServerPort, tcpServerAddress);
    settings.endGroup();

    // Webserver
    settings.beginGroup("WebServer");
    // Defaults for tests
    QHostAddress webServerAddress("127.0.0.1");
    QString webServerPublicFolder = qApp->applicationDirPath();
    uint webServerPort = 3333;
#ifndef TESTING_ENABLED
    webServerAddress = QHostAddress(settings.value("address", "0.0.0.0").toString());
    webServerPublicFolder = settings.value("publicFolder", "/usr/share/guh-webinterface/public/").toString();
    webServerPort = settings.value("port", 3333).toUInt();
#endif
#ifdef SNAPPY
    webServerAddress = QHostAddress(settings.value("address", "0.0.0.0").toString());
    webServerPublicFolder = QString(qgetenv("SNAP")) + "/guh-webinterface/";
    webServerPort = settings.value("port", 3333).toUInt();
#endif
    setWebServerConfiguration(webServerPort, webServerAddress);
    setWebServerPublicFolder(webServerPublicFolder);
    settings.endGroup();

    // Websocket server
    settings.beginGroup("WebSocketServer");
    // Defaults for tests
    QHostAddress webSocketAddress("127.0.0.1");
    uint webSocketPort = 4444;
#ifndef TESTING_ENABLED
    webSocketAddress = QHostAddress(settings.value("address", "0.0.0.0").toString());
    webSocketPort = settings.value("port", 4444).toUInt();
#endif
    setWebSocketConfiguration(webSocketPort, webSocketAddress);
    settings.endGroup();

    // Bluetooth server
    settings.beginGroup("BluetoothServer");
    setBluetoothServerEnabled(settings.value("enabled", false).toBool());
    settings.endGroup();

    // SSL configuration
    settings.beginGroup("SSL");
    setSslEnabled(settings.value("enabled", true).toBool());
    setSslCertificate(settings.value("certificate", "/etc/ssl/certs/guhd-certificate.crt").toString(), settings.value("certificate-key", "/etc/ssl/certs/guhd-certificate.key").toString());
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
    qCDebug(dcApplication()) << "Configuration: Server name:" << serverName;

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
    qCDebug(dcApplication()) << "Configuration: Time zone:" << QString::fromUtf8(timeZone);

    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("guhd");
    settings.setValue("timeZone", timeZone);
    settings.endGroup();

    m_timeZone = timeZone;
    emit timeZoneChanged();
}

QLocale GuhConfiguration::locale() const
{
    return m_locale;
}

void GuhConfiguration::setLocale(const QLocale &locale)
{
    qCDebug(dcApplication()) << "Configuration: set locale:" << locale.name() << locale.nativeCountryName() << locale.nativeLanguageName();

    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("guhd");
    settings.setValue("language", locale.name());
    settings.endGroup();

    m_locale = locale;
    emit localeChanged();
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
    qCDebug(dcApplication()) << "Configuration: TCP server" << QString("%1:%2").arg(address.toString()).arg(port);

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
    qCDebug(dcApplication()) << "Configuration: Web server" << QString("%1:%2").arg(address.toString()).arg(port);

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
    qCDebug(dcApplication()) << "Configuration: Websocket server" << QString("%1:%2").arg(address.toString()).arg(port);

    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("WebSocketServer");
    settings.setValue("address", address.toString());
    settings.setValue("port", port);
    settings.endGroup();

    m_webSocketAddress = address;
    m_webSocketPort = port;

    emit webSocketServerConfigurationChanged();
}

bool GuhConfiguration::bluetoothServerEnabled() const
{
    return m_bluetoothServerEnabled;
}

void GuhConfiguration::setBluetoothServerEnabled(const bool &enabled)
{
    qCDebug(dcApplication()) << "Configuration: Bluetooth server" << (enabled ? "enabled" : "disabled");

    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("BluetoothServer");
    settings.setValue("enabled", enabled);
    settings.endGroup();

    m_bluetoothServerEnabled = enabled;
    emit bluetoothServerEnabled();
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
    qCDebug(dcApplication()) << "Configuration: SSL certificate:" << sslCertificate << "SSL certificate key:" << sslCertificateKey;

    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("SSL");
    settings.setValue("certificate", sslCertificate);
    settings.setValue("certificate-key", sslCertificateKey);
    settings.endGroup();

    m_sslCertificate = sslCertificate;
    m_sslCertificateKey = sslCertificateKey;

    emit sslCertificateChanged();
}

void GuhConfiguration::setServerUuid(const QUuid &uuid)
{
    qCDebug(dcApplication()) << "Configuration: Server uuid:" << uuid.toString();

    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("guhd");
    settings.setValue("uuid", uuid);
    settings.endGroup();

    m_serverUuid = uuid;
}

void GuhConfiguration::setWebServerPublicFolder(const QString &path)
{
    qCDebug(dcApplication()) << "Configuration: Web Server public folder:" << path;

    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("WebServer");
    settings.setValue("publicFolder", path);
    settings.endGroup();

    m_webServerPublicFolder = path;
}

}
