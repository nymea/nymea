// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "loggingcategories.h"
#include "nymeaconfiguration.h"

#include <QTimeZone>
#include <QCoreApplication>
#include <QFile>
#include <QDir>

NYMEA_LOGGING_CATEGORY(dcConfiguration, "Configuration")

namespace nymeaserver {

NymeaConfiguration::NymeaConfiguration(QObject *parent) :
    QObject{parent},
    m_settings{new NymeaSettings(NymeaSettings::SettingsRoleGlobal, this)},
    m_mqttPoliciesSettings{new NymeaSettings(NymeaSettings::SettingsRoleMqttPolicies, this)}
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
            qCWarning(dcConfiguration()) << "Failed to open /etc/machine-id for reading. Generating a new UUID for this server instance.";
            setServerUuid(QUuid::createUuid());
        }
    }
    qCDebug(dcConfiguration()) << "System UUID is:" << serverUuid().toString();

    // Make sure default values are in configuration file so that it's easier for users to modify
    setServerName(serverName());
    setTimeZone(timeZone());
    setLocale(locale());
    setBluetoothServerEnabled(bluetoothServerEnabled());
    setSslCertificate(sslCertificate(), sslCertificateKey());
    setDebugServerEnabled(debugServerEnabled());

    // TcpServer
    bool createDefaults = !m_settings->childGroups().contains("TcpServer");
    if (m_settings->childGroups().contains("TcpServer")) {
        m_settings->beginGroup("TcpServer");
        if (m_settings->value("disabled").toBool()) {
            qCDebug(dcConfiguration()) << "TCP server disabled by configuration";
        } else if (!m_settings->childGroups().isEmpty()) {
            foreach (const QString &key, m_settings->childGroups()) {
                ServerConfiguration config = readServerConfig(key);
                m_tcpServerConfigs[config.id] = config;
            }
        } else {
            createDefaults = true;
        }
        m_settings->endGroup();
    }
    if (createDefaults) {
        qCWarning(dcConfiguration()) << "No TCP server configuration found. Generating default of nymeas://0.0.0.0:2222";
        ServerConfiguration config;
        config.id = "default";
        config.address = QHostAddress("0.0.0.0").toString();
        config.port = 2222;
        config.sslEnabled = true;
        config.authenticationEnabled = true;
        m_tcpServerConfigs[config.id] = config;
        storeServerConfig("TcpServer", config);
    }

    // Webserver
    createDefaults = !m_settings->childGroups().contains("WebServer");
    if (m_settings->childGroups().contains("WebServer")) {
        m_settings->beginGroup("WebServer");
        if (m_settings->value("disabled").toBool()) {
            qCDebug(dcConfiguration) << "Web server disabled by configuration";
        } else if (!m_settings->childGroups().isEmpty()) {
            foreach (const QString &key, m_settings->childGroups()) {
                WebServerConfiguration config = readWebServerConfig(key);
                m_webServerConfigs[config.id] = config;
            }
        } else {
            createDefaults = true;
        }
        m_settings->endGroup();
    }
    if (createDefaults) {
        qCWarning(dcConfiguration) << "No Web server configuration found. Generating defaults of http://0.0.0.0:80 and https://0.0.0.0:443";
        WebServerConfiguration insecureConfig;
        insecureConfig.id = "insecure";
        insecureConfig.address = QHostAddress("0.0.0.0").toString();
        insecureConfig.port = 80;
        insecureConfig.sslEnabled = false;
        insecureConfig.authenticationEnabled = false;
        insecureConfig.publicFolder = defaultWebserverPublicFolderPath();
        insecureConfig.restServerEnabled = false;
        m_webServerConfigs[insecureConfig.id] = insecureConfig;
        storeWebServerConfig(insecureConfig);

        WebServerConfiguration secureConfig;
        secureConfig.id = "secure";
        secureConfig.address = QHostAddress("0.0.0.0").toString();
        secureConfig.port = 443;
        secureConfig.sslEnabled = true;
        secureConfig.authenticationEnabled = false;
        secureConfig.publicFolder = defaultWebserverPublicFolderPath();
        secureConfig.restServerEnabled = false;
        m_webServerConfigs[secureConfig.id] = secureConfig;
        storeWebServerConfig(secureConfig);
    }

    // WebSocket Server
    createDefaults = !m_settings->childGroups().contains("WebSocketServer");
    if (m_settings->childGroups().contains("WebSocketServer")) {
        m_settings->beginGroup("WebSocketServer");
        if (m_settings->value("disabled").toBool()) {
            qCDebug(dcConfiguration) << "WebSocket server disabled by configuration.";
        } else if (!m_settings->childGroups().isEmpty()) {
            foreach (const QString &key, m_settings->childGroups()) {
                ServerConfiguration config = readServerConfig(key);
                m_webSocketServerConfigs[config.id] = config;
            }
        } else {
            createDefaults = true;
        }
        m_settings->endGroup();
    }
    if (createDefaults) {
        qCWarning(dcConfiguration) << "No WebSocket server configuration found. Generating default of wss://0.0.0.0:4444";
        ServerConfiguration config;
        config.id = "default";
        config.address = QHostAddress("0.0.0.0").toString();
        config.port = 4444;
        config.sslEnabled = true;
        config.authenticationEnabled = true;
        m_webSocketServerConfigs[config.id] = config;
        storeServerConfig("WebSocketServer", config);
    }

    // MQTT Server
    createDefaults = !m_settings->childGroups().contains("MqttServer");
    if (m_settings->childGroups().contains("MqttServer")) {
        m_settings->beginGroup("MqttServer");
        if (m_settings->value("disabled").toBool()) {
            qCDebug(dcConfiguration) << "MQTT server disabled by configuration.";
        } else if (!m_settings->childGroups().isEmpty()) {
            foreach (const QString &key, m_settings->childGroups()) {
                ServerConfiguration config = readServerConfig(key);
                m_mqttServerConfigs[config.id] = config;
            }
        } else {
            createDefaults = true;
        }
        m_settings->endGroup();
    }
    if (createDefaults) {
        qCWarning(dcConfiguration) << "No MQTT server configuration found. Generating default of 0.0.0.0:1883";
        ServerConfiguration config;
        config.id = "default";
        config.address = QHostAddress("0.0.0.0").toString();
        config.port = 1883;
        config.sslEnabled = false;
        config.authenticationEnabled = true;
        m_mqttServerConfigs[config.id] = config;
        storeServerConfig("MqttServer", config);
    }

    NymeaSettings mqttPolicies(NymeaSettings::SettingsRoleMqttPolicies);
    foreach (const QString &clientId, mqttPolicies.childGroups()) {
        mqttPolicies.beginGroup(clientId);
        MqttPolicy policy;
        policy.clientId = clientId;
        policy.username = mqttPolicies.value("username").toString();
        policy.password = mqttPolicies.value("password").toString();
        policy.allowedPublishTopicFilters = mqttPolicies.value("allowedPublishTopicFilters").toStringList();
        policy.allowedSubscribeTopicFilters = mqttPolicies.value("allowedSubscribeTopicFilters").toStringList();
        m_mqttPolicies.insert(clientId, policy);
        mqttPolicies.endGroup();
    }

    // Tunnel Proxy Server
    if (m_settings->childGroups().contains("TunnelProxyServer")) {
        m_settings->beginGroup("TunnelProxyServer");
        foreach (const QString &key, m_settings->childGroups()) {
            TunnelProxyServerConfiguration config = readTunnelProxyServerConfig(key);
            m_tunnelProxyServerConfigs[config.id] = config;
        }
        m_settings->endGroup();
    }

    // Write defaults for log settings
    m_settings->beginGroup("Logs");
    m_settings->setValue("logDBName", logDBName());
    m_settings->setValue("logDBHost", logDBHost());
    m_settings->setValue("logDBUser", logDBUser());
    m_settings->setValue("logDBPassword", logDBPassword());
    m_settings->endGroup();
}

QUuid NymeaConfiguration::serverUuid() const
{
    m_settings->beginGroup("nymead");
    QUuid value = m_settings->value("uuid", QUuid()).toUuid();
    m_settings->endGroup();
    return value;
}

QString NymeaConfiguration::serverName() const
{
    m_settings->beginGroup("nymead");
    QString value = m_settings->value("name", "nymea").toString();
    m_settings->endGroup();
    return value;
}

void NymeaConfiguration::setServerName(const QString &serverName)
{
    qCDebug(dcConfiguration()) << "Server name:" << serverName;

    m_settings->beginGroup("nymead");
    if (m_settings->value("name").toString() == serverName) {
        qCDebug(dcConfiguration()) << "Server name unchanged.";
        m_settings->endGroup();
    } else {
        m_settings->setValue("name", serverName);
        m_settings->endGroup();
        emit serverNameChanged(serverName);
    }
}

QByteArray NymeaConfiguration::timeZone() const
{
    m_settings->beginGroup("nymead");
    QByteArray value = m_settings->value("timeZone", QTimeZone::systemTimeZoneId()).toByteArray();
    m_settings->endGroup();
    return value;
}

void NymeaConfiguration::setTimeZone(const QByteArray &timeZone)
{
    qCDebug(dcConfiguration()) << "Time zone:" << QString::fromUtf8(timeZone);

    m_settings->beginGroup("nymead");
    if (m_settings->value("timeZone").toByteArray() == timeZone) {
        qCDebug(dcConfiguration()) << "Time zone unchanged.";
        m_settings->endGroup();
    } else {
        m_settings->setValue("timeZone", timeZone);
        m_settings->endGroup();
        emit timeZoneChanged();
    }
}

double NymeaConfiguration::locationLatitude() const
{
    m_settings->beginGroup("nymead");
    double value = m_settings->value("locationLatitude").toDouble();
    m_settings->endGroup();
    return value;
}

double NymeaConfiguration::locationLongitude() const
{
    m_settings->beginGroup("nymead");
    double value = m_settings->value("locationLongitude").toDouble();
    m_settings->endGroup();
    return value;
}

QString NymeaConfiguration::locationName() const
{
    m_settings->beginGroup("nymead");
    QString value = m_settings->value("locationName").toString();
    m_settings->endGroup();
    return value;
}

void NymeaConfiguration::setLocation(double latitude, double longitude, const QString &name)
{
    m_settings->beginGroup("nymead");
    if (m_settings->value("locationLatitude").toDouble() != latitude ||
        m_settings->value("locationLongitude").toDouble() != longitude ||
        m_settings->value("locationName").toString() != name) {

        m_settings->setValue("locationLatitude", latitude);
        m_settings->setValue("locationLongitude", longitude);
        m_settings->setValue("locationName", name);
        emit locationChanged();
    }
    m_settings->endGroup();
}

QLocale NymeaConfiguration::locale() const
{
    m_settings->beginGroup("nymead");
    QString value = m_settings->value("language", "en_US").toString();
    m_settings->endGroup();
    return value;
}

void NymeaConfiguration::setLocale(const QLocale &locale)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qCDebug(dcConfiguration()) << "Configuration: Set system locale:" << locale.name() << locale.nativeCountryName() << locale.nativeLanguageName();
#else
    qCDebug(dcConfiguration()) << "Configuration: Set system locale:" << locale.name() << locale.nativeTerritoryName() << locale.nativeLanguageName();

#endif

    m_settings->beginGroup("nymead");
    if (m_settings->value("language").toString() == locale.name()) {
        qCDebug(dcConfiguration()) << "Language unchanged.";
        m_settings->endGroup();
    } else {
        m_settings->setValue("language", locale.name());
        m_settings->endGroup();
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
    m_settings->beginGroup("WebServer");
    m_settings->beginGroup(config.id);
    m_settings->setValue("publicFolder", config.publicFolder);
    m_settings->setValue("restServerEnabled", config.restServerEnabled);
    m_settings->endGroup();
    m_settings->endGroup();

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

QHash<QString, TunnelProxyServerConfiguration> NymeaConfiguration::tunnelProxyServerConfigurations() const
{
    return m_tunnelProxyServerConfigs;
}

void NymeaConfiguration::setTunnelProxyServerConfiguration(const TunnelProxyServerConfiguration &config)
{
    m_tunnelProxyServerConfigs[config.id] = config;
    storeTunnelProxyServerConfig(config);
    emit tunnelProxyServerConfigurationChanged(config.id);
}

void NymeaConfiguration::removeTunnelProxyServerConfiguration(const QString &id)
{
    m_tunnelProxyServerConfigs.take(id);
    deleteServerConfig("TunnelProxyServer", id);
    emit tunnelProxyServerConfigurationRemoved(id);
}

QHash<QString, ServerConfiguration> NymeaConfiguration::mqttServerConfigurations() const
{
    return m_mqttServerConfigs;
}

void NymeaConfiguration::setMqttServerConfiguration(const ServerConfiguration &config)
{
    m_mqttServerConfigs[config.id] = config;
    storeServerConfig("MqttServer", config);
    emit mqttServerConfigurationChanged(config.id);
}

void NymeaConfiguration::removeMqttServerConfiguration(const QString &id)
{
    m_mqttServerConfigs.take(id);
    deleteServerConfig("MqttServer", id);
    emit mqttServerConfigurationRemoved(id);
}

QHash<QString, MqttPolicy> NymeaConfiguration::mqttPolicies() const
{
    return m_mqttPolicies;
}

void NymeaConfiguration::updateMqttPolicy(const MqttPolicy &policy)
{
    m_mqttPolicies[policy.clientId] = policy;
    storeMqttPolicy(policy);
    emit mqttPolicyChanged(policy.clientId);
}

bool NymeaConfiguration::removeMqttPolicy(const QString &clientId)
{
    if (!m_mqttPolicies.contains(clientId)) {
        return false;
    }
    m_mqttPolicies.take(clientId);
    deleteMqttPolicy(clientId);
    emit mqttPolicyRemoved(clientId);
    return true;
}

bool NymeaConfiguration::bluetoothServerEnabled() const
{
    m_settings->beginGroup("BluetoothServer");
    bool value = m_settings->value("enabled", false).toBool();
    m_settings->endGroup();
    return value;
}

void NymeaConfiguration::setBluetoothServerEnabled(bool enabled)
{
    qCDebug(dcConfiguration()) << "Bluetooth server" << (enabled ? "enabled" : "disabled");

    m_settings->beginGroup("BluetoothServer");
    m_settings->setValue("enabled", enabled);
    m_settings->endGroup();

    emit bluetoothServerEnabledChanged();
}

QString NymeaConfiguration::logDBHost() const
{
    m_settings->beginGroup("Logs");
    QString value = m_settings->value("logDBHost", "127.0.0.1").toString();
    m_settings->endGroup();
    return value;
}

QString NymeaConfiguration::logDBName() const
{
    m_settings->beginGroup("Logs");

    // Migration from < 1.8. Switching the driver is not supported any more.
    // As sqlite used an absolute filename which won't work as DB name in other databases
    // we'll reset the config if the user has explicitly set it.
    if (m_settings->value("logDBDriver").toString() == "QSQLITE") {
        m_settings->remove("logDBName");
        m_settings->remove("logDBDriver");
    }

    QString value = m_settings->value("logDBName", "nymea").toString();
    m_settings->endGroup();
    return value;
}

QString NymeaConfiguration::logDBUser() const
{
    m_settings->beginGroup("Logs");
    QString value = m_settings->value("logDBUser").toString();
    m_settings->endGroup();
    return value;
}

QString NymeaConfiguration::logDBPassword() const
{
    m_settings->beginGroup("Logs");
    QString value = m_settings->value("logDBPassword").toString();
    m_settings->endGroup();
    return value;
}

QString NymeaConfiguration::sslCertificate() const
{
    m_settings->beginGroup("SSL");
    QString value = m_settings->value("certificate").toString();
    m_settings->endGroup();
    return value;
}

QString NymeaConfiguration::sslCertificateKey() const
{
    m_settings->beginGroup("SSL");
    QString value = m_settings->value("certificate-key").toString();
    m_settings->endGroup();
    return value;
}

void NymeaConfiguration::setSslCertificate(const QString &sslCertificate, const QString &sslCertificateKey)
{
    m_settings->beginGroup("SSL");
    m_settings->setValue("certificate", sslCertificate);
    m_settings->setValue("certificate-key", sslCertificateKey);
    m_settings->endGroup();
}

bool NymeaConfiguration::debugServerEnabled() const
{
    m_settings->beginGroup("nymead");
    bool value = m_settings->value("debugServerEnabled", false).toBool();
    m_settings->endGroup();
    return value;
}

void NymeaConfiguration::setDebugServerEnabled(bool enabled)
{
    qCDebug(dcConfiguration()) << "Set debug server" << (enabled ? "enabled" : "disabled");
    bool currentValue = debugServerEnabled();

    m_settings->beginGroup("nymead");
    m_settings->setValue("debugServerEnabled", enabled);
    m_settings->endGroup();

    if (currentValue != enabled) {
        emit debugServerEnabledChanged(enabled);
    }
}

void NymeaConfiguration::setServerUuid(const QUuid &uuid)
{
    qCDebug(dcConfiguration()) << "Server uuid:" << uuid.toString();

    m_settings->beginGroup("nymead");
    m_settings->setValue("uuid", uuid.toString());
    m_settings->endGroup();
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
    m_settings->beginGroup(group);
    m_settings->remove("disabled");
    m_settings->beginGroup(config.id);
    m_settings->setValue("address", config.address);
    m_settings->setValue("port", config.port);
    m_settings->setValue("sslEnabled", config.sslEnabled);
    m_settings->setValue("authenticationEnabled", config.authenticationEnabled);
    m_settings->endGroup();
    m_settings->endGroup();
}

ServerConfiguration NymeaConfiguration::readServerConfig(const QString &id)
{
    ServerConfiguration config;
    m_settings->beginGroup(id);
    config.id = id;
    config.address = m_settings->value("address").toString();
    config.port = m_settings->value("port").toUInt();
    config.sslEnabled = m_settings->value("sslEnabled", true).toBool();
    config.authenticationEnabled = m_settings->value("authenticationEnabled", true).toBool();
    m_settings->endGroup();
    return config;
}

void NymeaConfiguration::deleteServerConfig(const QString &group, const QString &id)
{
    m_settings->beginGroup(group);
    m_settings->remove(id);
    if (m_settings->childGroups().isEmpty()) {
        m_settings->setValue("disabled", true);
    }
    m_settings->endGroup();
}

void NymeaConfiguration::storeWebServerConfig(const WebServerConfiguration &config)
{
    storeServerConfig("WebServer", config);

    m_settings->beginGroup("WebServer");
    m_settings->beginGroup(config.id);
    m_settings->setValue("publicFolder", config.publicFolder);
    m_settings->setValue("restServerEnabled", config.restServerEnabled);
    m_settings->endGroup();
    m_settings->endGroup();
}

WebServerConfiguration NymeaConfiguration::readWebServerConfig(const QString &id)
{
    WebServerConfiguration config;    
    m_settings->beginGroup("WebServer");
    m_settings->beginGroup(id);
    config.id = id;
    config.address = m_settings->value("address").toString();
    config.port = m_settings->value("port").toUInt();
    config.sslEnabled = m_settings->value("sslEnabled", true).toBool();
    config.authenticationEnabled = m_settings->value("authenticationEnabled", true).toBool();
    config.publicFolder = m_settings->value("publicFolder").toString();
    config.restServerEnabled = m_settings->value("restServerEnabled", false).toBool();
    m_settings->endGroup();
    m_settings->endGroup();
    return config;
}

void NymeaConfiguration::storeTunnelProxyServerConfig(const TunnelProxyServerConfiguration &config)
{
    storeServerConfig("TunnelProxyServer", config);

    m_settings->beginGroup("TunnelProxyServer");
    m_settings->beginGroup(config.id);
    m_settings->setValue("ignoreSslErrors", config.ignoreSslErrors);
    m_settings->endGroup();
    m_settings->endGroup();
}

TunnelProxyServerConfiguration NymeaConfiguration::readTunnelProxyServerConfig(const QString &id)
{
    TunnelProxyServerConfiguration config;
    m_settings->beginGroup("TunnelProxyServer");
    m_settings->beginGroup(id);
    config.id = id;
    config.address = m_settings->value("address").toString();
    config.port = m_settings->value("port").toUInt();
    config.sslEnabled = m_settings->value("sslEnabled", true).toBool();
    config.authenticationEnabled = m_settings->value("authenticationEnabled", true).toBool();
    config.ignoreSslErrors = m_settings->value("ignoreSslErrors").toBool();
    m_settings->endGroup();
    m_settings->endGroup();
    return config;
}

void NymeaConfiguration::storeMqttPolicy(const MqttPolicy &policy)
{
    m_mqttPoliciesSettings->beginGroup(policy.clientId);
    m_mqttPoliciesSettings->setValue("username", policy.username);
    m_mqttPoliciesSettings->setValue("password", policy.password);
    m_mqttPoliciesSettings->setValue("allowedPublishTopicFilters", policy.allowedPublishTopicFilters);
    m_mqttPoliciesSettings->setValue("allowedSubscribeTopicFilters", policy.allowedSubscribeTopicFilters);
    m_mqttPoliciesSettings->endGroup();
}

void NymeaConfiguration::deleteMqttPolicy(const QString &clientId)
{
    m_mqttPoliciesSettings->remove(clientId);
}

QDebug operator <<(QDebug debug, const ServerConfiguration &configuration)
{
    debug.nospace() << "ServerConfiguration(" << configuration.address;
    debug.nospace() << ", " << configuration.id;
    debug.nospace() << ", " << QString("%1:%2").arg(configuration.address).arg(configuration.port);
    debug.nospace() << ") ";
    return debug.maybeSpace();
}

QDebug operator <<(QDebug debug, const TunnelProxyServerConfiguration &configuration)
{
    debug.nospace() << "TunnelProxyServerConfiguration(" << configuration.id;
    debug.nospace() << ", " << QString("%1:%2").arg(configuration.address).arg(configuration.port);
    if (configuration.sslEnabled) {
        debug.nospace() << ", SSL enabled";
    } else {
        debug.nospace() << ", SSL disabled";
    }
    if (configuration.authenticationEnabled) {
        debug.nospace() << ", Authentication enabled";
    } else {
        debug.nospace() << ", Authentication disabled";
    }

    if (configuration.ignoreSslErrors) {
        debug.nospace() << ", Ignoring SSL errors";
    }

    debug.nospace() << ") ";
    return debug.maybeSpace();
}

}
