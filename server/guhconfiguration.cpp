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

#ifndef TESTING_ENABLED
    // TcpServer
    bool createDefaults = !settings.childGroups().contains("TcpServer");
    if (settings.childGroups().contains("TcpServer")) {
        settings.beginGroup("TcpServer");
        if (settings.value("disabled").toBool()) {
            qCDebug(dcApplication) << "TCP Server disabled by configuration";
        } else if (!settings.childGroups().isEmpty()) {
            foreach (const QString &key, settings.childGroups()) {
                ServerConfiguration config = readServerConfig("TcpServer", key);
                m_tcpServerConfigs[config.id] = config;
            }
        } else {
            createDefaults = true;
        }
        settings.endGroup();
    }
    if (createDefaults) {
        qCWarning(dcApplication) << "No TCP Server configuration found. Generating default of 0.0.0.0:2222";
        ServerConfiguration config;
        config.id = "default";
        config.address = QHostAddress("0.0.0.0");
        config.port = 2222;
        // TODO enable encryption/authentication by default once the important clients are supporting it
        config.sslEnabled = false;
        config.authenticationEnabled = false;
        m_tcpServerConfigs[config.id] = config;
        storeServerConfig("TcpServer", config);
    }

    // Webserver
    createDefaults = !settings.childGroups().contains("WebServer");
    if (settings.childGroups().contains("WebServer")) {
        settings.beginGroup("WebServer");
        if (settings.value("disabled").toBool()) {
            qCDebug(dcApplication) << "WebServer disabled by configuration";
        } else if (!settings.childGroups().isEmpty()) {
            foreach (const QString &key, settings.childGroups()) {
                WebServerConfiguration config = readWebServerConfig(key);
                m_webServerConfigs[config.id] = config;
            }
        } else {
            createDefaults = true;
        }
        settings.endGroup();
    }
    if (createDefaults) {
        qCWarning(dcApplication) << "No WebServer configuration found. Generating default of 0.0.0.0:3333";
        WebServerConfiguration insecureConfig;
        insecureConfig.id = "insecure";
        insecureConfig.address = QHostAddress("0.0.0.0");
        insecureConfig.port = 80;
        insecureConfig.sslEnabled = false;
        insecureConfig.authenticationEnabled = false;
        insecureConfig.publicFolder = "/usr/share/guh-webinterface/public/";
        m_webServerConfigs[insecureConfig.id] = insecureConfig;
        storeWebServerConfig(insecureConfig);

        WebServerConfiguration secureConfig;
        secureConfig.id = "secure";
        secureConfig.address = QHostAddress("0.0.0.0");
        secureConfig.port = 443;
        secureConfig.sslEnabled = true;
        secureConfig.authenticationEnabled = false;
        secureConfig.publicFolder = "/usr/share/guh-webinterface/public/";
        m_webServerConfigs[secureConfig.id] = secureConfig;
        storeWebServerConfig(secureConfig);
    }

    // WebSocket Server
    createDefaults = !settings.childGroups().contains("WebServer");
    if (settings.childGroups().contains("WebSocketServer")) {
        settings.beginGroup("WebSocketServer");
        if (settings.value("disabled").toBool()) {
            qCDebug(dcApplication) << "WebSocket Server disabled by configuration.";
        } else if (!settings.childGroups().isEmpty()) {
            foreach (const QString &key, settings.childGroups()) {
                ServerConfiguration config = readServerConfig("WebSocketServer", key);
                m_webSocketServerConfigs[config.id] = config;
            }
        } else {
            createDefaults = true;
        }
        settings.endGroup();
    }
    if (createDefaults) {
        qCWarning(dcApplication) << "No WebSocketServer configuration found. Generating default of 0.0.0.0:4444";
        ServerConfiguration config;
        config.id = "default";
        config.address = QHostAddress("0.0.0.0");
        config.port = 4444;
        // TODO enable encryption/authentication by default once the important clients are supporting it
        config.sslEnabled = false;
        config.authenticationEnabled = false;
        m_webSocketServerConfigs[config.id] = config;
        storeServerConfig("WebSocketServer", config);
    }


#else
    ServerConfiguration tcpConfig;
    tcpConfig.id = "default";
    tcpConfig.address = QHostAddress("127.0.0.1");
    tcpConfig.port = 2222;
    tcpConfig.sslEnabled = true;
    tcpConfig.authenticationEnabled = true;
    m_tcpServerConfigs[tcpConfig.id] = tcpConfig;

    WebServerConfiguration wsConfig;
    wsConfig.id = "default";
    wsConfig.address = QHostAddress("127.0.0.1");
    wsConfig.port = 3333;
    wsConfig.sslEnabled = true;
    wsConfig.authenticationEnabled = true;
    wsConfig.publicFolder = qApp->applicationDirPath();
    m_webServerConfigs[wsConfig.id] = wsConfig;

    ServerConfiguration wssConfig;
    wssConfig.id = "default";
    wssConfig.address = QHostAddress("127.0.0.1");
    wssConfig.port = 4444;
    wssConfig.sslEnabled = true;
    wssConfig.authenticationEnabled = true;
    m_webSocketServerConfigs[wssConfig.id] = wssConfig;
#endif

    // Bluetooth server
    settings.beginGroup("BluetoothServer");
    setBluetoothServerEnabled(settings.value("enabled", false).toBool());
    settings.endGroup();

    // SSL configuration
    settings.beginGroup("SSL");
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

QHash<QString, ServerConfiguration> GuhConfiguration::tcpServerConfigurations() const
{
    return m_tcpServerConfigs;
}

void GuhConfiguration::setTcpServerConfiguration(const ServerConfiguration &config)
{
    m_tcpServerConfigs[config.id] = config;
    storeServerConfig("TcpServer", config);
    emit tcpServerConfigurationChanged(config.id);
}

void GuhConfiguration::removeTcpServerConfiguration(const QString &id)
{
    m_tcpServerConfigs.take(id);
    deleteServerConfig("TcpServer", id);
    emit tcpServerConfigurationRemoved(id);
}

QHash<QString, WebServerConfiguration> GuhConfiguration::webServerConfigurations() const
{
    return m_webServerConfigs;
}

void GuhConfiguration::setWebServerConfiguration(const WebServerConfiguration &config)
{
    m_webServerConfigs[config.id] = config;

    storeServerConfig("WebServer", config);

    // This is a bit odd that we need to open the config once more just for the publicFolder...
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("WebServer");
    settings.beginGroup(config.id);
    settings.setValue("publicFolder", config.publicFolder);
    settings.endGroup();
    settings.endGroup();

    emit webServerConfigurationChanged(config.id);
}

void GuhConfiguration::removeWebServerConfiguration(const QString &id)
{
    m_webServerConfigs.take(id);
    deleteServerConfig("WebServer", id);
    emit webServerConfigurationRemoved(id);
}

QHash<QString, ServerConfiguration> GuhConfiguration::webSocketServerConfigurations() const
{
    return m_webSocketServerConfigs;
}

void GuhConfiguration::setWebSocketServerConfiguration(const ServerConfiguration &config)
{
    m_webSocketServerConfigs[config.id] = config;
    storeServerConfig("WebSocketServer", config);
    emit webSocketServerConfigurationChanged(config.id);
}

void GuhConfiguration::removeWebSocketServerConfiguration(const QString &id)
{
    m_webSocketServerConfigs.take(id);
    deleteServerConfig("WebSocketServer", id);
    emit webSocketServerConfigurationRemoved(id);
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

void GuhConfiguration::storeServerConfig(const QString &group, const ServerConfiguration &config)
{
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup(group);
    settings.remove("disabled");
    settings.beginGroup(config.id);
    settings.setValue("address", config.address.toString());
    settings.setValue("port", config.port);
    settings.setValue("sslEnabled", config.sslEnabled);
    settings.setValue("authenticationEnabled", config.authenticationEnabled);
    settings.endGroup();
    settings.endGroup();
}

ServerConfiguration GuhConfiguration::readServerConfig(const QString &group, const QString &id)
{
    ServerConfiguration config;
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup(group);
    settings.beginGroup(id);
    config.id = id;
    config.address = QHostAddress(settings.value("address").toString());
    config.port = settings.value("port").toUInt();
    config.sslEnabled = settings.value("sslEnabled", true).toBool();
    config.authenticationEnabled = settings.value("authenticationEnabled", true).toBool();
    settings.endGroup();
    settings.endGroup();
    return config;
}

void GuhConfiguration::deleteServerConfig(const QString &group, const QString &id)
{
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup(group);
    settings.remove(id);
    if (settings.childGroups().isEmpty()) {
        settings.setValue("disabled", true);
    }
    settings.endGroup();
}

void GuhConfiguration::storeWebServerConfig(const WebServerConfiguration &config)
{
    storeServerConfig("WebServer", config);
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("WebServer");
    settings.beginGroup(config.id);
    settings.setValue("publicFolder", config.publicFolder);
    settings.endGroup();
    settings.endGroup();
}

WebServerConfiguration GuhConfiguration::readWebServerConfig(const QString &id)
{
    WebServerConfiguration config;
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("WebServer");
    settings.beginGroup(id);
    config.id = id;
    config.address = QHostAddress(settings.value("address").toString());
    config.port = settings.value("port").toUInt();
    config.sslEnabled = settings.value("sslEnabled", true).toBool();
    config.authenticationEnabled = settings.value("authenticationEnabled", true).toBool();
    config.publicFolder = settings.value("publicFolder").toString();
    settings.endGroup();
    settings.endGroup();
    return config;
}

}
