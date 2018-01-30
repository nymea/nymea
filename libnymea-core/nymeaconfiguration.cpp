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
#include "nymeaconfiguration.h"
#include "nymeasettings.h"

#include <QTimeZone>
#include <QCoreApplication>
#include <QFile>

namespace guhserver {

NymeaConfiguration::NymeaConfiguration(QObject *parent) :
    QObject(parent)
{
    // Init server uuid if we don't have one.
    QUuid id = serverUuid();
    if (id.isNull()) {
        // If we can, let's use the system's machine-id as our UUID.
        QFile f("/etc/machine-id");
        if (f.open(QFile::ReadOnly)) {
            QString tmpId = QString::fromLatin1(f.readAll()).trimmed();
            tmpId.insert(8, "-");
            tmpId.insert(13, "-");
            tmpId.insert(18, "-");
            tmpId.insert(23, "-");
            setServerUuid(QUuid(tmpId));
        } else {
            qWarning(dcApplication()) << "Failed to open /etc/machine-id for reading. Generating a new UUID for this server instance.";
            setServerUuid(QUuid::createUuid());
        }
    }
    qCDebug(dcApplication()) << "System UUID is:" << serverUuid().toString();

    // Make sure default values are in configuration file so that it's easier for users to modify
    setServerName(serverName());
    setTimeZone(timeZone());
    setLocale(locale());
    setBluetoothServerEnabled(bluetoothServerEnabled());
    setSslCertificate(sslCertificate(), sslCertificateKey());
    setDebugServerEnabled(debugServerEnabled());

    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);

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
        insecureConfig.publicFolder = defaultWebserverPublicFolderPath();
        m_webServerConfigs[insecureConfig.id] = insecureConfig;
        storeWebServerConfig(insecureConfig);

        WebServerConfiguration secureConfig;
        secureConfig.id = "secure";
        secureConfig.address = QHostAddress("0.0.0.0");
        secureConfig.port = 443;
        secureConfig.sslEnabled = true;
        secureConfig.authenticationEnabled = false;
        secureConfig.publicFolder = defaultWebserverPublicFolderPath();
        m_webServerConfigs[secureConfig.id] = secureConfig;
        storeWebServerConfig(secureConfig);
    }

    // WebSocket Server
    createDefaults = !settings.childGroups().contains("WebSocketServer");
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
}

QUuid NymeaConfiguration::serverUuid() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("nymead");
    return settings.value("uuid", QUuid()).toUuid();
}

QString NymeaConfiguration::serverName() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("nymead");
    return settings.value("name", "guhIO").toString();
}

void NymeaConfiguration::setServerName(const QString &serverName)
{
    qCDebug(dcApplication()) << "Configuration: Server name:" << serverName;

    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("nymead");
    if (settings.value("name").toString() == serverName) {
        qCDebug(dcApplication()) << "Configuration: Server name unchanged.";
        settings.endGroup();
    } else {
        settings.setValue("name", serverName);
        settings.endGroup();
        emit serverNameChanged(serverName);
    }
}

QByteArray NymeaConfiguration::timeZone() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("nymead");
    return settings.value("timeZone", QTimeZone::systemTimeZoneId()).toByteArray();
}

void NymeaConfiguration::setTimeZone(const QByteArray &timeZone)
{
    qCDebug(dcApplication()) << "Configuration: Time zone:" << QString::fromUtf8(timeZone);

    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("nymead");
    if (settings.value("timeZone").toByteArray() == timeZone) {
        qCDebug(dcApplication()) << "Configuration: Time zone unchanged.";
        settings.endGroup();
    } else {
        settings.setValue("timeZone", timeZone);
        settings.endGroup();
        emit timeZoneChanged();
    }
}

QLocale NymeaConfiguration::locale() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("nymead");
    return settings.value("language", "en_US").toString();
}

void NymeaConfiguration::setLocale(const QLocale &locale)
{
    qCDebug(dcApplication()) << "Configuration: set locale:" << locale.name() << locale.nativeCountryName() << locale.nativeLanguageName();

    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("nymead");
    if (settings.value("language").toString() == locale.name()) {
        qCDebug(dcApplication()) << "Configuration: Language unchanged.";
        settings.endGroup();
    } else {
        settings.setValue("language", locale.name());
        settings.endGroup();
        emit localeChanged();
    }
}

QHash<QString, ServerConfiguration> NymeaConfiguration::tcpServerConfigurations() const
{
    return m_tcpServerConfigs;
}

void NymeaConfiguration::setTcpServerConfiguration(const ServerConfiguration &config)
{
    m_tcpServerConfigs[config.id] = config;
    storeServerConfig("TcpServer", config);
    emit tcpServerConfigurationChanged(config.id);
}

void NymeaConfiguration::removeTcpServerConfiguration(const QString &id)
{
    m_tcpServerConfigs.take(id);
    deleteServerConfig("TcpServer", id);
    emit tcpServerConfigurationRemoved(id);
}

QHash<QString, WebServerConfiguration> NymeaConfiguration::webServerConfigurations() const
{
    return m_webServerConfigs;
}

void NymeaConfiguration::setWebServerConfiguration(const WebServerConfiguration &config)
{
    m_webServerConfigs[config.id] = config;

    storeServerConfig("WebServer", config);

    // This is a bit odd that we need to open the config once more just for the publicFolder...
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("WebServer");
    settings.beginGroup(config.id);
    settings.setValue("publicFolder", config.publicFolder);
    settings.endGroup();
    settings.endGroup();

    emit webServerConfigurationChanged(config.id);
}

void NymeaConfiguration::removeWebServerConfiguration(const QString &id)
{
    m_webServerConfigs.take(id);
    deleteServerConfig("WebServer", id);
    emit webServerConfigurationRemoved(id);
}

QHash<QString, ServerConfiguration> NymeaConfiguration::webSocketServerConfigurations() const
{
    return m_webSocketServerConfigs;
}

void NymeaConfiguration::setWebSocketServerConfiguration(const ServerConfiguration &config)
{
    m_webSocketServerConfigs[config.id] = config;
    storeServerConfig("WebSocketServer", config);
    emit webSocketServerConfigurationChanged(config.id);
}

void NymeaConfiguration::removeWebSocketServerConfiguration(const QString &id)
{
    m_webSocketServerConfigs.take(id);
    deleteServerConfig("WebSocketServer", id);
    emit webSocketServerConfigurationRemoved(id);
}

bool NymeaConfiguration::bluetoothServerEnabled() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("BluetoothServer");
    return settings.value("enabled", false).toBool();
}

void NymeaConfiguration::setBluetoothServerEnabled(const bool &enabled)
{
    qCDebug(dcApplication()) << "Configuration: Bluetooth server" << (enabled ? "enabled" : "disabled");

    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("BluetoothServer");
    settings.setValue("enabled", enabled);
    settings.endGroup();
    emit bluetoothServerEnabled();
}

bool NymeaConfiguration::cloudEnabled() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    return settings.value("enabled", false).toBool();
}

void NymeaConfiguration::setCloudEnabled(bool enabled)
{
    if (cloudEnabled() != enabled) {
        NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
        settings.beginGroup("Cloud");
        settings.setValue("enabled", enabled);
        settings.endGroup();
        emit cloudEnabledChanged(enabled);
    }
}

QString NymeaConfiguration::cloudServerUrl() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    return settings.value("cloudServerUrl").toString();
}

QString NymeaConfiguration::cloudCertificateCA() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    return settings.value("cloudCertificateCA").toString();
}

QString NymeaConfiguration::cloudCertificate() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    return settings.value("cloudCertificate").toString();
}

QString NymeaConfiguration::cloudCertificateKey() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    return settings.value("cloudCertificateKey").toString();
}

QString NymeaConfiguration::sslCertificate() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("SSL");
    return settings.value("certificate").toString();
}

QString NymeaConfiguration::sslCertificateKey() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("SSL");
    return settings.value("certificate-key").toString();
}

void NymeaConfiguration::setSslCertificate(const QString &sslCertificate, const QString &sslCertificateKey)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("SSL");
    settings.setValue("certificate", sslCertificate);
    settings.setValue("certificate-key", sslCertificateKey);
    settings.endGroup();
}

bool NymeaConfiguration::debugServerEnabled() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("nymead");
    return settings.value("debugServerEnabled", false).toBool();
}

void NymeaConfiguration::setDebugServerEnabled(bool enabled)
{
    qCDebug(dcApplication()) << "Configuration: Set debug server" << (enabled ? "enabled" : "disabled");
    bool currentValue = debugServerEnabled();
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("nymead");
    settings.setValue("debugServerEnabled", enabled);
    settings.endGroup();

    if (currentValue != enabled) {
        emit debugServerEnabledChanged(enabled);
    }
}

void NymeaConfiguration::setServerUuid(const QUuid &uuid)
{
    qCDebug(dcApplication()) << "Configuration: Server uuid:" << uuid.toString();

    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("nymead");
    settings.setValue("uuid", uuid);
    settings.endGroup();
}

QString NymeaConfiguration::defaultWebserverPublicFolderPath() const
{
    QString publicFolderPath;
    if (!qgetenv("SNAP").isEmpty()) {
        // FIXME: one could point to sensible data by changing the SNAP env to i.e /etc
        publicFolderPath = QString(qgetenv("SNAP")) + "/nymea-webinterface";
    } else {
        publicFolderPath = "/usr/share/nymea-webinterface/public/";
    }
    return publicFolderPath;
}

void NymeaConfiguration::storeServerConfig(const QString &group, const ServerConfiguration &config)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
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

ServerConfiguration NymeaConfiguration::readServerConfig(const QString &group, const QString &id)
{
    ServerConfiguration config;
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
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

void NymeaConfiguration::deleteServerConfig(const QString &group, const QString &id)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup(group);
    settings.remove(id);
    if (settings.childGroups().isEmpty()) {
        settings.setValue("disabled", true);
    }
    settings.endGroup();
}

void NymeaConfiguration::storeWebServerConfig(const WebServerConfiguration &config)
{
    storeServerConfig("WebServer", config);
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("WebServer");
    settings.beginGroup(config.id);
    settings.setValue("publicFolder", config.publicFolder);
    settings.endGroup();
    settings.endGroup();
}

WebServerConfiguration NymeaConfiguration::readWebServerConfig(const QString &id)
{
    WebServerConfiguration config;
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
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

QDebug operator <<(QDebug debug, const ServerConfiguration &configuration)
{
    debug.nospace() << "ServerConfiguration(" << configuration.address;
    debug.nospace() << ", " << configuration.id;
    debug.nospace() << ", " << QString("%1:%2").arg(configuration.address.toString()).arg(configuration.port);
    debug.nospace() << ") ";
    return debug;
}

}
