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

/*!
    \class nymeaserver::ConfigurationHandler
    \brief This subclass of \l{JsonHandler} processes the JSON requests for the \tt Configuration namespace.

    \ingroup json
    \inmodule core

    This \l{JsonHandler} will be created in the \l{JsonRPCServer} and used to handle JSON-RPC requests
    for the \tt {Configuration} namespace of the API.

    \sa JsonHandler, JsonRPCServer
*/

/*! \fn void nymeaserver::ConfigurationHandler::BasicConfigurationChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the configurations of the server have been changed.
    The \a params contains the map for the notification.
*/

/*! \fn void nymeaserver::ConfigurationHandler::TcpServerConfigurationChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the configurations of the \l{TcpServer} have been changed.
    The \a params contains the map for the notification.
*/

/*! \fn void nymeaserver::ConfigurationHandler::WebServerConfigurationChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the configurations of the \l{WebServer} have been changed.
    The \a params contains the map for the notification.
*/

/*! \fn void nymeaserver::ConfigurationHandler::WebSocketServerConfigurationChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the configurations of the \l{WebSocketServer} have been changed.
    The \a params contains the map for the notification.
*/

/*! \fn void nymeaserver::ConfigurationHandler::LanguageChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the language of the system has changed.
    The \a params contains the map for the notification.
*/



#include "configurationhandler.h"
#include "nymeacore.h"
#include "nymeaconfiguration.h"
#include "loggingcategories.h"
#include "platform/platform.h"
#include "platform/platformsystemcontroller.h"

namespace nymeaserver {

/*! Constructs a new \l ConfigurationHandler with the given \a parent. */
ConfigurationHandler::ConfigurationHandler(QObject *parent):
    JsonHandler(parent)
{
    // Enums
    registerEnum<NymeaConfiguration::ConfigurationError>();

    // Objects
    registerObject<ServerConfiguration>();
    registerObject<WebServerConfiguration>();
    registerObject<TunnelProxyServerConfiguration>();
    registerObject<MqttPolicy>();

    // Methods
    QString description; QVariantMap params; QVariantMap returns;
    description = "Get the list of available timezones.";
    returns.insert("timeZones", QVariantList() << enumValueName(String));
    registerMethod("GetTimeZones", description, params, returns, Types::PermissionScopeNone, "Use System.GetTimeZones instead.");

    params.clear(); returns.clear();
    description = "Returns a list of locale codes available for the server. i.e. en_US, de_AT";
    returns.insert("languages", QVariantList() << enumValueName(String));
    registerMethod("GetAvailableLanguages", description, params, returns, Types::PermissionScopeNone, "Use the locale property in the Handshake message instead.");

    params.clear(); returns.clear();
    description = "Get all configuration parameters of the server.";

    QVariantMap basicConfiguration;
    basicConfiguration.insert("serverName", enumValueName(String));
    basicConfiguration.insert("serverUuid", enumValueName(Uuid));
    basicConfiguration.insert("d:serverTime", enumValueName(Uint));
    basicConfiguration.insert("d:timeZone", enumValueName(String));
    basicConfiguration.insert("d:language", enumValueName(String));

    QVariantMap location;
    location.insert("latitude", enumValueName(Double));
    location.insert("longitude", enumValueName(Double));
    location.insert("name", enumValueName(String));
    basicConfiguration.insert("d:location", location);
    basicConfiguration.insert("debugServerEnabled", enumValueName(Bool));
    returns.insert("basicConfiguration", basicConfiguration);

    QVariantMap backupConfiguration;
    backupConfiguration.insert("destinationDirectory", enumValueName(String));
    backupConfiguration.insert("maxCount", enumValueName(Uint));

    QVariantList tcpServerConfigurations;
    tcpServerConfigurations.append(objectRef<ServerConfiguration>());
    returns.insert("tcpServerConfigurations", tcpServerConfigurations);
    QVariantList webServerConfigurations;
    webServerConfigurations.append(objectRef<WebServerConfiguration>());
    returns.insert("webServerConfigurations", webServerConfigurations);
    QVariantList webSocketServerConfigurations;
    webSocketServerConfigurations.append(objectRef<ServerConfiguration>());
    returns.insert("webSocketServerConfigurations", webSocketServerConfigurations);
    QVariantList tunnelProxyServerConfigurations;
    tunnelProxyServerConfigurations.append(objectRef<TunnelProxyServerConfiguration>());
    returns.insert("tunnelProxyServerConfigurations", tunnelProxyServerConfigurations);
    QVariantList mqttServerConfigurations;
    mqttServerConfigurations.append(objectRef<ServerConfiguration>());
    returns.insert("mqttServerConfigurations", mqttServerConfigurations);
    returns.insert("backupConfigurations", backupConfiguration);
    registerMethod("GetConfigurations", description, params, returns, Types::PermissionScopeNone);


    params.clear(); returns.clear();
    description = "Set the name of the server. Default is nymea.";
    params.insert("serverName",  enumValueName(String));
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("SetServerName", description, params, returns);

    params.clear(); returns.clear();
    description = "Set the time zone of the server. See also: \"GetTimeZones\"";
    params.insert("timeZone",  enumValueName(String));
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("SetTimeZone", description, params, returns, Types::PermissionScopeAdmin, "Use System.SetTimeZone instead.");

    params.clear(); returns.clear();
    description = "Sets the server language to the given language. See also: \"GetAvailableLanguages\"";
    params.insert("language",  enumValueName(String));
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("SetLanguage", description, params, returns, Types::PermissionScopeAdmin, "Use the locale property in the Handshake message instead.");

    params.clear(); returns.clear();
    description = "Sets the server location.";
    params.insert("location", location);
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("SetLocation", description, params, returns, Types::PermissionScopeAdmin);

    params.clear(); returns.clear();
    description = "Enable or disable the debug server.";
    params.insert("enabled", enumValueName(String));
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("SetDebugServerEnabled", description, params, returns);

    params.clear(); returns.clear();
    description = "Configure a TCP interface of the server. If the ID is an existing one, the existing config will be modified, otherwise a new one will be added. Note: if you are changing the configuration for the interface you are currently connected to, the connection will be dropped.";
    params.insert("configuration", objectRef<ServerConfiguration>());
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("SetTcpServerConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Delete a TCP interface of the server. Note: if you are deleting the configuration for the interface you are currently connected to, the connection will be dropped.";
    params.insert("id", enumValueName(String));
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("DeleteTcpServerConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Configure a WebSocket Server interface of the server. If the ID is an existing one, the existing config will be modified, otherwise a new one will be added. Note: if you are changing the configuration for the interface you are currently connected to, the connection will be dropped.";
    params.insert("configuration", objectRef<ServerConfiguration>());
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("SetWebSocketServerConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Delete a WebSocket Server interface of the server. Note: if you are deleting the configuration for the interface you are currently connected to, the connection will be dropped.";
    params.insert("id", enumValueName(String));
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("DeleteWebSocketServerConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Configure a Tunnel Proxy Server interface of the server. If the ID is an existing one, the existing config will be modified, otherwise a new one will be added. Note: if you are changing the configuration for the interface you are currently connected to, the connection will be dropped.";
    params.insert("configuration", objectRef<TunnelProxyServerConfiguration>());
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("SetTunnelProxyServerConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Delete a Tunnel Proxy Server interface of the server. Note: if you are deleting the configuration for the interface you are currently connected to, the connection will be dropped.";
    params.insert("id", enumValueName(String));
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("DeleteTunnelProxyServerConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Configure a WebServer interface of the server. If the ID is an existing one, the existing config will be modified, otherwise a new one will be added.";
    params.insert("configuration", objectRef<WebServerConfiguration>());
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("SetWebServerConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Delete a WebServer interface of the server.";
    params.insert("id", enumValueName(String));
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("DeleteWebServerConfiguration", description, params, returns);

    // Backup
    params.clear(); returns.clear();
    description = "Set the backup configuration. The destination directory is the location where the archives will be saved, the maxCount is the number of backups which will be kept. If maxCount is 0, all backups will be kept.";
    params.insert("destinationDirectory", enumValueName(String));
    params.insert("maxCount", enumValueName(Uint));
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("SetBackupConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Create a backup of the current configuration. It will be stored in the configured destination directory. Also the maxCout configuration will be considered.";
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("CreateBackup", description, params, returns);


    // MQTT
    params.clear(); returns.clear();
    description = "Get all MQTT Server configurations.";
    returns.insert("mqttServerConfigurations", QVariantList() << objectRef<ServerConfiguration>());
    registerMethod("GetMqttServerConfigurations", description, params, returns);

    params.clear(); returns.clear();
    description = "Configure a MQTT Server interface on the MQTT broker. If the ID is an existing one, the existing config will be modified, otherwise a new one will be added. Setting authenticationEnabled to true will require MQTT clients to use credentials set in the MQTT broker policies.";
    params.insert("configuration", objectRef<ServerConfiguration>());
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("SetMqttServerConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Delete a MQTT Server interface of the server.";
    params.insert("id", enumValueName(String));
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("DeleteMqttServerConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Get all MQTT broker policies.";
    returns.insert("mqttPolicies", QVariantList() << objectRef<MqttPolicy>());
    registerMethod("GetMqttPolicies", description, params, returns);

    params.clear(); returns.clear();
    description = "Configure a MQTT broker policy. If the ID is an existing one, the existing policy will be modified, otherwise a new one will be added.";
    params.insert("policy", objectRef<MqttPolicy>());
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("SetMqttPolicy", description, params, returns);

    params.clear(); returns.clear();
    description = "Delete a MQTT policy from the broker.";
    params.insert("clientId", enumValueName(String));
    returns.insert("configurationError", enumRef<NymeaConfiguration::ConfigurationError>());
    registerMethod("DeleteMqttPolicy", description, params, returns);

    // Notifications
    params.clear(); returns.clear();
    description = "Emitted whenever the basic configuration of this server changes.";
    params.insert("basicConfiguration", basicConfiguration);
    registerNotification("BasicConfigurationChanged", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever the language of the server changed. The Plugins, Vendors and ThingClasses have to be reloaded to get the translated data.";
    params.insert("language", enumValueName(String));
    registerNotification("LanguageChanged", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever the TCP server configuration changes.";
    params.insert("tcpServerConfiguration", objectRef<ServerConfiguration>());
    registerNotification("TcpServerConfigurationChanged", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever a TCP server configuration is removed.";
    params.insert("id", enumValueName(String));
    registerNotification("TcpServerConfigurationRemoved", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever the web socket server configuration changes.";
    params.insert("webSocketServerConfiguration", objectRef<ServerConfiguration>());
    registerNotification("WebSocketServerConfigurationChanged", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever a WebSocket server configuration is removed.";
    params.insert("id", enumValueName(String));
    registerNotification("WebSocketServerConfigurationRemoved", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever the tunnel proxy server configuration changes.";
    params.insert("tunnelProxyServerConfiguration", objectRef<TunnelProxyServerConfiguration>());
    registerNotification("TunnelProxyServerConfigurationChanged", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever a tunnel proxy server configuration is removed.";
    params.insert("id", enumValueName(String));
    registerNotification("TunnelProxyServerConfigurationRemoved", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever the backup configuration changes.";
    params.insert("destinationDirectory", enumValueName(String));
    params.insert("maxCount", enumValueName(Uint));
    registerNotification("BackupConfigurationChanged", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever the MQTT broker configuration is changed.";
    params.insert("mqttServerConfiguration", objectRef<ServerConfiguration>());
    registerNotification("MqttServerConfigurationChanged", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever a MQTT server configuration is removed.";
    params.insert("id", enumValueName(String));
    registerNotification("MqttServerConfigurationRemoved", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever the web server configuration changes.";
    params.insert("webServerConfiguration", objectRef<WebServerConfiguration>());
    registerNotification("WebServerConfigurationChanged", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever a Web server configuration is removed.";
    params.insert("id", enumValueName(String));
    registerNotification("WebServerConfigurationRemoved", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever a MQTT broker policy is changed.";
    params.insert("policy", objectRef<MqttPolicy>());
    registerNotification("MqttPolicyChanged", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever a MQTT broker policy is removed.";
    params.insert("clientId", enumValueName(String));
    registerNotification("MqttPolicyRemoved", description, params);

    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::serverNameChanged, this, &ConfigurationHandler::onBasicConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::timeZoneChanged, this, &ConfigurationHandler::onBasicConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::locationChanged, this, &ConfigurationHandler::onBasicConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::localeChanged, this, &ConfigurationHandler::onBasicConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::debugServerEnabledChanged, this, &ConfigurationHandler::onBasicConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::tcpServerConfigurationChanged, this, &ConfigurationHandler::onTcpServerConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::tcpServerConfigurationRemoved, this, &ConfigurationHandler::onTcpServerConfigurationRemoved);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::webServerConfigurationChanged, this, &ConfigurationHandler::onWebServerConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::webServerConfigurationRemoved, this, &ConfigurationHandler::onWebServerConfigurationRemoved);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::webSocketServerConfigurationChanged, this, &ConfigurationHandler::onWebSocketServerConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::webSocketServerConfigurationRemoved, this, &ConfigurationHandler::onWebSocketServerConfigurationRemoved);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::tunnelProxyServerConfigurationChanged, this, &ConfigurationHandler::onTunnelProxyServerConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::tunnelProxyServerConfigurationRemoved, this, &ConfigurationHandler::onTunnelProxyServerConfigurationRemoved);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::mqttServerConfigurationChanged, this, &ConfigurationHandler::onMqttServerConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::mqttServerConfigurationRemoved, this, &ConfigurationHandler::onMqttServerConfigurationRemoved);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::mqttPolicyChanged, this, &ConfigurationHandler::onMqttPolicyChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::mqttPolicyRemoved, this, &ConfigurationHandler::onMqttPolicyRemoved);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::backupDestinationDirectoryChanged, this, &ConfigurationHandler::onBackupConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::backupMaxCountChanged, this, &ConfigurationHandler::onBackupConfigurationChanged);
}

/*! Returns the name of the \l{ConfigurationHandler}. In this case \b Configuration.*/
QString ConfigurationHandler::name() const
{
    return "Configuration";
}

JsonReply *ConfigurationHandler::GetConfigurations(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap returns;
    returns.insert("basicConfiguration", packBasicConfiguration());
    returns.insert("backupConfigurations", packBackupConfiguration());

    QVariantList tcpServerConfigs;
    foreach (const ServerConfiguration &config, NymeaCore::instance()->configuration()->tcpServerConfigurations()) {
        tcpServerConfigs.append(pack(config));
    }
    returns.insert("tcpServerConfigurations", tcpServerConfigs);

    QVariantList webServerConfigs;
    foreach (const WebServerConfiguration &config, NymeaCore::instance()->configuration()->webServerConfigurations()) {
        webServerConfigs.append(pack(config));

    }
    returns.insert("webServerConfigurations", webServerConfigs);

    QVariantList webSocketServerConfigs;
    foreach (const ServerConfiguration &config, NymeaCore::instance()->configuration()->webSocketServerConfigurations()) {
        webSocketServerConfigs.append(pack(config));
    }
    returns.insert("webSocketServerConfigurations", webSocketServerConfigs);

    QVariantList tunnelProxyServerConfigs;
    foreach (const TunnelProxyServerConfiguration &config, NymeaCore::instance()->configuration()->tunnelProxyServerConfigurations()) {
        tunnelProxyServerConfigs.append(pack(config));
    }
    returns.insert("tunnelProxyServerConfigurations", tunnelProxyServerConfigs);

    return createReply(returns);
}

JsonReply *ConfigurationHandler::GetTimeZones(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantList timeZones;
    foreach (const QByteArray &timeZoneId, QTimeZone::availableTimeZoneIds()) {
        timeZones.append(QString::fromUtf8(timeZoneId));
    }

    QVariantMap returns;
    returns.insert("timeZones", timeZones);
    return createReply(returns);
}

JsonReply *ConfigurationHandler::GetAvailableLanguages(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantList languages;
    foreach (const QString &language, NymeaCore::getAvailableLanguages()) {
        languages.append(language);
    }
    QVariantMap returns;
    returns.insert("languages", languages);
    return createReply(returns);
}

JsonReply *ConfigurationHandler::SetServerName(const QVariantMap &params) const
{
    QString serverName = params.value("serverName").toString();
    NymeaCore::instance()->configuration()->setServerName(serverName);
    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetTimeZone(const QVariantMap &params) const
{
    qCDebug(dcJsonRpc()) << "Setting time zone to" << params.value("timeZone").toString();

    QByteArray timeZoneName = params.value("timeZone").toString().toUtf8();

    QTimeZone timeZone(timeZoneName);
    if (!timeZone.isValid()) {
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidTimeZone));
    }

    bool success = NymeaCore::instance()->platform()->systemController()->setTimeZone(timeZone);
    if (!success) {
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidTimeZone));
    }

    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetLanguage(const QVariantMap &params) const
{
    qCDebug(dcJsonRpc()) << "Setting language to" << params.value("language").toString();
    QLocale locale(params.value("language").toString());

    NymeaCore::instance()->configuration()->setLocale(locale);

    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetLocation(const QVariantMap &params) const
{
    QVariantMap locationMap = params.value("location").toMap();
    double latitude = locationMap.value("latitude").toDouble();
    double longitude = locationMap.value("longitude").toDouble();
    QString name = locationMap.value("name").toString();
    NymeaCore::instance()->configuration()->setLocation(latitude, longitude, name);
    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetTcpServerConfiguration(const QVariantMap &params) const
{
    ServerConfiguration config = unpack<ServerConfiguration>(params.value("configuration").toMap());
    if (config.id.isEmpty())
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidId));

    if (config.address.isNull())
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidHostAddress));

    if (config.port <= 0 || config.port > 65535) {
        qCWarning(dcJsonRpc()) << "Port out of range";
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidPort));
    }

    // To be compliant with the EN18031 we have to make sure the user cannot configure an insecure interface to the server.
    if (qEnvironmentVariable("NYMEA_INSECURE_INTERFACES_DISABLED", "0") != "0") {
        bool isLocalhost = config.address == "localhost" || config.address == "127.0.0.1";
        bool isSecured = config.sslEnabled && config.authenticationEnabled;
        if (!isLocalhost && !isSecured) {
            qCWarning(dcJsonRpc()) << "Cannot add insecure TCP server configuration" << config << "because insecure interfaces to the core are explicit disabled.";
            return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorUnsupported));
        }
    }

    qCDebug(dcJsonRpc()) << QString("Configure TCP server %1:%2").arg(config.address).arg(config.port);
    NymeaCore::instance()->configuration()->setTcpServerConfiguration(config);
    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::DeleteTcpServerConfiguration(const QVariantMap &params) const
{
    QString id = params.value("id").toString();
    if (id.isEmpty() || !NymeaCore::instance()->configuration()->tcpServerConfigurations().contains(id)) {
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidId));
    }
    NymeaCore::instance()->configuration()->removeTcpServerConfiguration(id);
    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetWebServerConfiguration(const QVariantMap &params) const
{
    WebServerConfiguration config = unpack<WebServerConfiguration>(params.value("configuration").toMap());

    if (config.id.isEmpty()) {
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidId));
    }
    if (config.address.isNull())
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidHostAddress));

    if (config.port <= 0 || config.port > 65535) {
        qCWarning(dcJsonRpc()) << "Port out of range";
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidPort));
    }

    qCDebug(dcJsonRpc()) << QString("Configure web server %1:%2").arg(config.address).arg(config.port);

    NymeaCore::instance()->configuration()->setWebServerConfiguration(config);
    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::DeleteWebServerConfiguration(const QVariantMap &params) const
{
    QString id = params.value("id").toString();
    if (id.isEmpty() || !NymeaCore::instance()->configuration()->webServerConfigurations().contains(id)) {
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidId));
    }
    NymeaCore::instance()->configuration()->removeWebServerConfiguration(id);
    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetWebSocketServerConfiguration(const QVariantMap &params) const
{
    ServerConfiguration config = unpack<ServerConfiguration>(params.value("configuration").toMap());
    if (config.id.isEmpty())
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidId));

    if (config.address.isNull())
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidHostAddress));

    if (config.port <= 0 || config.port > 65535) {
        qCWarning(dcJsonRpc()) << "Port out of range";
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidPort));
    }

    // To be compliant with the EN18031 we have to make sure the user cannot configure an insecure interface to the server.
    if (qEnvironmentVariable("NYMEA_INSECURE_INTERFACES_DISABLED", "0") != "0") {
        bool isLocalhost = config.address == "localhost" || config.address == "127.0.0.1";
        bool isSecured = config.sslEnabled && config.authenticationEnabled;
        if (!isLocalhost && !isSecured) {
            qCWarning(dcJsonRpc()) << "Cannot add insecure WebSocket server configuration" << config << "because insecure interfaces to the core are explicit disabled.";
            return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorUnsupported));
        }
    }

    qCDebug(dcJsonRpc()) << QString("Configuring web socket server %1:%2").arg(config.address).arg(config.port);

    NymeaCore::instance()->configuration()->setWebSocketServerConfiguration(config);

    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::DeleteWebSocketServerConfiguration(const QVariantMap &params) const
{
    QString id = params.value("id").toString();
    if (id.isEmpty() || !NymeaCore::instance()->configuration()->webSocketServerConfigurations().contains(id)) {
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidId));
    }
    NymeaCore::instance()->configuration()->removeWebSocketServerConfiguration(id);
    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetTunnelProxyServerConfiguration(const QVariantMap &params) const
{
    TunnelProxyServerConfiguration config = unpack<TunnelProxyServerConfiguration>(params.value("configuration").toMap());
    if (config.id.isEmpty())
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidId));

    if (config.address.isNull())
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidHostAddress));

    if (config.port <= 0 || config.port > 65535) {
        qCWarning(dcJsonRpc()) << "Port out of range";
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidPort));
    }

    // To be compliant with the EN18031 we have to make sure the user cannot configure an insecure interface to the server.
    if (qEnvironmentVariable("NYMEA_INSECURE_INTERFACES_DISABLED", "0") != "0") {
        if (!config.sslEnabled || !config.authenticationEnabled || config.ignoreSslErrors) {
            qCWarning(dcJsonRpc()) << "Cannot add insecure tunnelproxy server configuration" << config << "because insecure interfaces to the core are explicit disabled.";
            return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorUnsupported));
        }
    }

    qCDebug(dcJsonRpc()) << QString("Configuring tunnel proxy server %1:%2").arg(config.address).arg(config.port);

    NymeaCore::instance()->configuration()->setTunnelProxyServerConfiguration(config);

    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::DeleteTunnelProxyServerConfiguration(const QVariantMap &params) const
{
    QString id = params.value("id").toString();
    if (id.isEmpty() || !NymeaCore::instance()->configuration()->tunnelProxyServerConfigurations().contains(id)) {
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidId));
    }
    NymeaCore::instance()->configuration()->removeTunnelProxyServerConfiguration(id);
    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetBackupConfiguration(const QVariantMap &params) const
{
    QString destinationDirectory = params.value("destinationDirectory").toString();
    uint maxCount = params.value("maxCount").toUInt();
    QDir destinationDir(destinationDirectory);
    if (!destinationDir.exists()) {
        // Try to make the directory
        if (!destinationDir.mkpath(destinationDirectory)) {
            qCWarning(dcJsonRpc()) << "Failed to set backup configuration. The destination directory does not exist and could not be created." << destinationDir;
            return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidDestinationDir));
        }
    }

    NymeaCore::instance()->configuration()->setBackupDestinationDirectory(destinationDirectory);
    NymeaCore::instance()->configuration()->setBackupMaxCount(maxCount);
    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::CreateBackup(const QVariantMap &params) const
{
    Q_UNUSED(params);
    qCDebug(dcJsonRpc()) << "Request to create a configuration backup received.";

    NymeaCore::instance()->
}

JsonReply *ConfigurationHandler::GetMqttServerConfigurations(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap returns;
    QVariantList mqttServerConfigs;
    foreach (const ServerConfiguration &config, NymeaCore::instance()->configuration()->mqttServerConfigurations())
        mqttServerConfigs << pack(config);

    returns.insert("mqttServerConfigurations", mqttServerConfigs);
    return createReply(returns);
}

JsonReply *ConfigurationHandler::SetMqttServerConfiguration(const QVariantMap &params) const
{
    ServerConfiguration config = unpack<ServerConfiguration>(params.value("configuration").toMap());
    if (config.id.isEmpty())
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidId));

    if (config.address.isNull())
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidHostAddress));

    if (config.port <= 0 || config.port > 65535) {
        qCWarning(dcJsonRpc()) << "Port out of range";
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidPort));
    }

    qCDebug(dcJsonRpc()) << QString("Configure MQTT server %1:%2").arg(config.address).arg(config.port);

    NymeaCore::instance()->configuration()->setMqttServerConfiguration(config);

    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::DeleteMqttServerConfiguration(const QVariantMap &params) const
{
    QString id = params.value("id").toString();
    if (id.isEmpty() || !NymeaCore::instance()->configuration()->mqttServerConfigurations().contains(id)) {
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidId));
    }
    NymeaCore::instance()->configuration()->removeMqttServerConfiguration(id);
    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::GetMqttPolicies(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantList mqttPolicies;
    foreach (const MqttPolicy &policy, NymeaCore::instance()->configuration()->mqttPolicies()) {
        mqttPolicies << pack(policy);
    }
    QVariantMap returns;
    returns.insert("mqttPolicies", mqttPolicies);
    return createReply(returns);
}

JsonReply *ConfigurationHandler::SetMqttPolicy(const QVariantMap &params) const
{
    MqttPolicy policy = unpack<MqttPolicy>(params.value("policy").toMap());
    NymeaCore::instance()->configuration()->updateMqttPolicy(policy);
    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::DeleteMqttPolicy(const QVariantMap &params) const
{
    QString clientId = params.value("clientId").toString();
    bool success = NymeaCore::instance()->configuration()->removeMqttPolicy(clientId);
    return createReply(statusToReply(success ? NymeaConfiguration::ConfigurationErrorNoError : NymeaConfiguration::ConfigurationErrorInvalidId));
}

JsonReply *ConfigurationHandler::SetDebugServerEnabled(const QVariantMap &params) const
{
    bool enabled = params.value("enabled").toBool();
    NymeaCore::instance()->configuration()->setDebugServerEnabled(enabled);
    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

void ConfigurationHandler::onBasicConfigurationChanged()
{
    qCDebug(dcJsonRpc()) << "Notification: Basic configuration changed";
    QVariantMap params;
    params.insert("basicConfiguration", packBasicConfiguration());
    emit BasicConfigurationChanged(params);
}

void ConfigurationHandler::onBackupConfigurationChanged()
{
    qCDebug(dcJsonRpc()) << "Notification: Backup configuration changed";
    emit BackupConfigurationChanged(packBackupConfiguration());
}

void ConfigurationHandler::onTcpServerConfigurationChanged(const QString &id)
{
    qCDebug(dcJsonRpc()) << "Notification: TCP server configuration changed";
    QVariantMap params;
    params.insert("tcpServerConfiguration", pack(NymeaCore::instance()->configuration()->tcpServerConfigurations().value(id)));
    emit TcpServerConfigurationChanged(params);
}

void ConfigurationHandler::onTcpServerConfigurationRemoved(const QString &id)
{
    qCDebug(dcJsonRpc()) << "Notification: TCP server configuration removed";
    QVariantMap params;
    params.insert("id", id);
    emit TcpServerConfigurationRemoved(params);
}

void ConfigurationHandler::onWebServerConfigurationChanged(const QString &id)
{
    qCDebug(dcJsonRpc()) << "Notification: web server configuration changed";
    QVariantMap params;
    params.insert("webServerConfiguration", pack(NymeaCore::instance()->configuration()->webServerConfigurations().value(id)));
    emit WebServerConfigurationChanged(params);
}

void ConfigurationHandler::onWebServerConfigurationRemoved(const QString &id)
{
    qCDebug(dcJsonRpc()) << "Notification: Web server configuration removed";
    QVariantMap params;
    params.insert("id", id);
    emit WebServerConfigurationRemoved(params);
}

void ConfigurationHandler::onWebSocketServerConfigurationChanged(const QString &id)
{
    qCDebug(dcJsonRpc()) << "Notification: web socket server configuration changed";
    QVariantMap params;
    params.insert("webSocketServerConfiguration", pack(NymeaCore::instance()->configuration()->webSocketServerConfigurations().value(id)));
    emit WebSocketServerConfigurationChanged(params);
}

void ConfigurationHandler::onWebSocketServerConfigurationRemoved(const QString &id)
{
    qCDebug(dcJsonRpc()) << "Notification: WebSocket server configuration removed";
    QVariantMap params;
    params.insert("id", id);
    emit WebSocketServerConfigurationRemoved(params);
}

void ConfigurationHandler::onTunnelProxyServerConfigurationChanged(const QString &id)
{
    qCDebug(dcJsonRpc()) << "Notification: Tunnel proxy server configuration changed";
    QVariantMap params;
    params.insert("tunnelProxyServerConfiguration", pack(NymeaCore::instance()->configuration()->tunnelProxyServerConfigurations().value(id)));
    emit TunnelProxyServerConfigurationChanged(params);
}

void ConfigurationHandler::onTunnelProxyServerConfigurationRemoved(const QString &id)
{
    qCDebug(dcJsonRpc()) << "Notification: Tunnel proxy server configuration removed";
    QVariantMap params;
    params.insert("id", id);
    emit TunnelProxyServerConfigurationRemoved(params);
}

void ConfigurationHandler::onMqttServerConfigurationChanged(const QString &id)
{
    qCDebug(dcJsonRpc()) << "Notification: MQTT server configuration changed";
    QVariantMap params;
    params.insert("mqttServerConfiguration", pack(NymeaCore::instance()->configuration()->mqttServerConfigurations().value(id)));
    emit MqttServerConfigurationChanged(params);
}

void ConfigurationHandler::onMqttServerConfigurationRemoved(const QString &id)
{
    qCDebug(dcJsonRpc()) << "Notification: MQTT server configuration removed";
    QVariantMap params;
    params.insert("id", id);
    emit MqttServerConfigurationRemoved(params);
}

void ConfigurationHandler::onMqttPolicyChanged(const QString &clientId)
{
    qCDebug(dcJsonRpc()) << "Notification: MQTT policy changed";
    QVariantMap params;
    params.insert("policy", pack(NymeaCore::instance()->configuration()->mqttPolicies().value(clientId)));
    emit MqttPolicyChanged(params);
}

void ConfigurationHandler::onMqttPolicyRemoved(const QString &clientId)
{
    qCDebug(dcJsonRpc()) << "Notification: MQTT policy removed";
    QVariantMap params;
    params.insert("clientId", clientId);
    emit MqttPolicyRemoved(params);
}

QVariantMap ConfigurationHandler::packBasicConfiguration()
{
    QVariantMap basicConfiguration;
    basicConfiguration.insert("serverName", NymeaCore::instance()->configuration()->serverName());
    basicConfiguration.insert("serverUuid", NymeaCore::instance()->configuration()->serverUuid().toString());
    basicConfiguration.insert("serverTime", NymeaCore::instance()->timeManager()->currentDateTime().toSecsSinceEpoch());
    basicConfiguration.insert("timeZone", QTimeZone::systemTimeZoneId());
    basicConfiguration.insert("language", NymeaCore::instance()->configuration()->locale().name());
    basicConfiguration.insert("location", QVariantMap{
                                              {"latitude", NymeaCore::instance()->configuration()->locationLatitude()},
                                              {"longitude", NymeaCore::instance()->configuration()->locationLongitude()},
                                              {"name", NymeaCore::instance()->configuration()->locationName()}
                                          });
    basicConfiguration.insert("debugServerEnabled", NymeaCore::instance()->configuration()->debugServerEnabled());
    return basicConfiguration;
}

QVariantMap ConfigurationHandler::packBackupConfiguration()
{
    QVariantMap configuration;
    configuration.insert("destinationDirectory", NymeaCore::instance()->configuration()->backupDestinationDirectory());
    configuration.insert("maxCount", NymeaCore::instance()->configuration()->backupMaxCount());
    return configuration;
}

QVariantMap ConfigurationHandler::statusToReply(NymeaConfiguration::ConfigurationError status) const
{
    QVariantMap returns;
    returns.insert("configurationError", enumValueName<NymeaConfiguration::ConfigurationError>(status));
    return returns;
}

}
