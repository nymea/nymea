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

#include "loggingcategories.h"
#include "nymeaconfiguration.h"
#include "nymeasettings.h"

#include <QTimeZone>
#include <QCoreApplication>
#include <QFile>
#include <QDir>

NYMEA_LOGGING_CATEGORY(dcConfiguration, "Configuration")

namespace nymeaserver {

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
            qWarning(dcConfiguration()) << "Failed to open /etc/machine-id for reading. Generating a new UUID for this server instance.";
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

    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);

    // TcpServer
    bool createDefaults = !settings.childGroups().contains("TcpServer");
    if (settings.childGroups().contains("TcpServer")) {
        settings.beginGroup("TcpServer");
        if (settings.value("disabled").toBool()) {
            qCDebug(dcConfiguration) << "TCP server disabled by configuration";
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
        qCWarning(dcConfiguration) << "No TCP server configuration found. Generating default of nymeas://0.0.0.0:2222";
        ServerConfiguration config;
        config.id = "default";
        config.address = QHostAddress("0.0.0.0");
        config.port = 2222;
        config.sslEnabled = true;
        config.authenticationEnabled = true;
        m_tcpServerConfigs[config.id] = config;
        storeServerConfig("TcpServer", config);
    }

    // Webserver
    createDefaults = !settings.childGroups().contains("WebServer");
    if (settings.childGroups().contains("WebServer")) {
        settings.beginGroup("WebServer");
        if (settings.value("disabled").toBool()) {
            qCDebug(dcConfiguration) << "Web server disabled by configuration";
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
        qCWarning(dcConfiguration) << "No Web server configuration found. Generating defaults of http://0.0.0.0:80 and https://0.0.0.0:443";
        WebServerConfiguration insecureConfig;
        insecureConfig.id = "insecure";
        insecureConfig.address = QHostAddress("0.0.0.0");
        insecureConfig.port = 80;
        insecureConfig.sslEnabled = false;
        insecureConfig.authenticationEnabled = false;
        insecureConfig.publicFolder = defaultWebserverPublicFolderPath();
        insecureConfig.restServerEnabled = false;
        m_webServerConfigs[insecureConfig.id] = insecureConfig;
        storeWebServerConfig(insecureConfig);

        WebServerConfiguration secureConfig;
        secureConfig.id = "secure";
        secureConfig.address = QHostAddress("0.0.0.0");
        secureConfig.port = 443;
        secureConfig.sslEnabled = true;
        secureConfig.authenticationEnabled = false;
        secureConfig.publicFolder = defaultWebserverPublicFolderPath();
        secureConfig.restServerEnabled = false;
        m_webServerConfigs[secureConfig.id] = secureConfig;
        storeWebServerConfig(secureConfig);
    }

    // WebSocket Server
    createDefaults = !settings.childGroups().contains("WebSocketServer");
    if (settings.childGroups().contains("WebSocketServer")) {
        settings.beginGroup("WebSocketServer");
        if (settings.value("disabled").toBool()) {
            qCDebug(dcConfiguration) << "WebSocket server disabled by configuration.";
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
        qCWarning(dcConfiguration) << "No WebSocket server configuration found. Generating default of wss://0.0.0.0:4444";
        ServerConfiguration config;
        config.id = "default";
        config.address = QHostAddress("0.0.0.0");
        config.port = 4444;
        config.sslEnabled = true;
        config.authenticationEnabled = true;
        m_webSocketServerConfigs[config.id] = config;
        storeServerConfig("WebSocketServer", config);
    }

    // MQTT Server
    createDefaults = !settings.childGroups().contains("MqttServer");
    if (settings.childGroups().contains("MqttServer")) {
        settings.beginGroup("MqttServer");
        if (settings.value("disabled").toBool()) {
            qCDebug(dcConfiguration) << "MQTT server disabled by configuration.";
        } else if (!settings.childGroups().isEmpty()) {
            foreach (const QString &key, settings.childGroups()) {
                ServerConfiguration config = readServerConfig("MqttServer", key);
                m_mqttServerConfigs[config.id] = config;
            }
        } else {
            createDefaults = true;
        }
        settings.endGroup();
    }
    if (createDefaults) {
        qCWarning(dcConfiguration) << "No MQTT server configuration found. Generating default of 0.0.0.0:1883";
        ServerConfiguration config;
        config.id = "default";
        config.address = QHostAddress("0.0.0.0");
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

    // Write defaults for log settings
    settings.beginGroup("Logs");
    settings.setValue("logDBDriver", logDBDriver());
    settings.setValue("logDBName", logDBName());
    settings.setValue("logDBHost", logDBHost());
    settings.setValue("logDBUser", logDBUser());
    settings.setValue("logDBPassword", logDBPassword());
    settings.setValue("logDBMaxEntries", logDBMaxEntries());
    settings.endGroup();
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
    return settings.value("name", "nymea").toString();
}

void NymeaConfiguration::setServerName(const QString &serverName)
{
    qCDebug(dcConfiguration()) << "Configuration: Server name:" << serverName;

    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("nymead");
    if (settings.value("name").toString() == serverName) {
        qCDebug(dcConfiguration()) << "Configuration: Server name unchanged.";
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
    qCDebug(dcConfiguration()) << "Configuration: Time zone:" << QString::fromUtf8(timeZone);

    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("nymead");
    if (settings.value("timeZone").toByteArray() == timeZone) {
        qCDebug(dcConfiguration()) << "Configuration: Time zone unchanged.";
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
    qCDebug(dcConfiguration()) << "Configuration: Set system locale:" << locale.name() << locale.nativeCountryName() << locale.nativeLanguageName();

    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("nymead");
    if (settings.value("language").toString() == locale.name()) {
        qCDebug(dcConfiguration()) << "Configuration: Language unchanged.";
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
    settings.setValue("restServerEnabled", config.restServerEnabled);
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
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("BluetoothServer");
    return settings.value("enabled", false).toBool();
}

void NymeaConfiguration::setBluetoothServerEnabled(bool enabled)
{
    qCDebug(dcConfiguration()) << "Configuration: Bluetooth server" << (enabled ? "enabled" : "disabled");

    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("BluetoothServer");
    settings.setValue("enabled", enabled);
    settings.endGroup();
    emit bluetoothServerEnabledChanged();
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

void NymeaConfiguration::setCloudServerUrl(const QString &cloudServerUrl)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    settings.setValue("cloudServerUrl", cloudServerUrl);
}

QString NymeaConfiguration::cloudCertificateCA() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    return settings.value("cloudCertificateCA").toString();
}

void NymeaConfiguration::setCloudCertificateCA(const QString &cloudCertificateCA)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    settings.setValue("cloudCertificateCA", cloudCertificateCA);
}

QString NymeaConfiguration::cloudCertificate() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    return settings.value("cloudCertificate").toString();
}

void NymeaConfiguration::setCloudCertificate(const QString &cloudCertificate)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    settings.setValue("cloudCertificate", cloudCertificate);
}

QString NymeaConfiguration::cloudCertificateKey() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    return settings.value("cloudCertificateKey").toString();
}

void NymeaConfiguration::setCloudCertificateKey(const QString &cloudCertificateKey)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Cloud");
    settings.setValue("cloudCertificateKey", cloudCertificateKey);
}

QString NymeaConfiguration::logDBDriver() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Logs");
    return settings.value("logDBDriver", "QSQLITE").toString();
}

QString NymeaConfiguration::logDBHost() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Logs");
    return settings.value("logDBHost", "127.0.0.1").toString();
}

QString NymeaConfiguration::logDBName() const
{
    QString defaultLogPath;
    QString organisationName = QCoreApplication::instance()->organizationName();

    if (!qgetenv("SNAP").isEmpty()) {
        defaultLogPath = QString(qgetenv("SNAP_COMMON")) + "/nymead.sqlite";
    } else if (organisationName == "nymea-test") {
        defaultLogPath = "/tmp/" + organisationName + "/nymead-test.sqlite";
    } else if (NymeaSettings::isRoot()) {
        defaultLogPath = "/var/log/nymead.sqlite";
    } else {
        defaultLogPath = QDir::homePath() + "/.config/" + organisationName + "/nymead.sqlite";
    }

    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Logs");
    return settings.value("logDBName", defaultLogPath).toString();
}

QString NymeaConfiguration::logDBUser() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Logs");
    return settings.value("logDBUser").toString();
}

QString NymeaConfiguration::logDBPassword() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Logs");
    return settings.value("logDBPassword").toString();
}

int NymeaConfiguration::logDBMaxEntries() const
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Logs");
    return settings.value("logDBMaxEntries", 200000).toInt();
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
    qCDebug(dcConfiguration()) << "Configuration: Set debug server" << (enabled ? "enabled" : "disabled");
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
    qCDebug(dcConfiguration()) << "Configuration: Server uuid:" << uuid.toString();

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
    settings.setValue("restServerEnabled", config.restServerEnabled);
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
    config.restServerEnabled = settings.value("restServerEnabled", false).toBool();
    settings.endGroup();
    settings.endGroup();
    return config;
}

void NymeaConfiguration::storeMqttPolicy(const MqttPolicy &policy)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleMqttPolicies);
    settings.beginGroup(policy.clientId);
    settings.setValue("username", policy.username);
    settings.setValue("password", policy.password);
    settings.setValue("allowedPublishTopicFilters", policy.allowedPublishTopicFilters);
    settings.setValue("allowedSubscribeTopicFilters", policy.allowedSubscribeTopicFilters);
    settings.endGroup();
}

void NymeaConfiguration::deleteMqttPolicy(const QString &clientId)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleMqttPolicies);
    settings.remove(clientId);
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
