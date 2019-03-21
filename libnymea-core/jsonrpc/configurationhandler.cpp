/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

namespace nymeaserver {

/*! Constructs a new \l ConfigurationHandler with the given \a parent. */
ConfigurationHandler::ConfigurationHandler(QObject *parent):
    JsonHandler(parent)
{
    // Methods
    QVariantMap params; QVariantMap returns;
    setDescription("GetTimeZones", "Get the list of available timezones.");
    setParams("GetTimeZones", params);
    returns.insert("timeZones", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::String));
    setReturns("GetTimeZones", returns);

    params.clear(); returns.clear();
    setDescription("GetAvailableLanguages", "Returns a list of locale codes available for the server. i.e. en_US, de_AT");
    setParams("GetAvailableLanguages", params);
    returns.insert("languages", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::String));
    setReturns("GetAvailableLanguages", returns);

    params.clear(); returns.clear();
    setDescription("GetConfigurations", "Get all configuration parameters of the server.");
    setParams("GetConfigurations", params);
    QVariantMap basicConfiguration;
    basicConfiguration.insert("serverName", JsonTypes::basicTypeToString(JsonTypes::String));
    basicConfiguration.insert("serverUuid", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    basicConfiguration.insert("serverTime", JsonTypes::basicTypeToString(JsonTypes::Uint));
    basicConfiguration.insert("timeZone", JsonTypes::basicTypeToString(JsonTypes::String));
    basicConfiguration.insert("language", JsonTypes::basicTypeToString(JsonTypes::String));
    basicConfiguration.insert("debugServerEnabled", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("basicConfiguration", basicConfiguration);
    QVariantList tcpServerConfigurations;
    tcpServerConfigurations.append(JsonTypes::serverConfigurationRef());
    returns.insert("tcpServerConfigurations", tcpServerConfigurations);
    QVariantList webServerConfigurations;
    webServerConfigurations.append(JsonTypes::webServerConfigurationRef());
    returns.insert("webServerConfigurations", webServerConfigurations);
    QVariantList webSocketServerConfigurations;
    webSocketServerConfigurations.append(JsonTypes::serverConfigurationRef());
    returns.insert("webSocketServerConfigurations", webSocketServerConfigurations);
    QVariantList mqttServerConfigurations;
    mqttServerConfigurations.append(JsonTypes::serverConfigurationRef());
    QVariantMap cloudConfiguration;
    cloudConfiguration.insert("enabled", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("cloud", cloudConfiguration);
    setReturns("GetConfigurations", returns);

    params.clear(); returns.clear();
    setDescription("SetServerName", "Set the name of the server. Default is nymea.");
    params.insert("serverName",  JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("SetServerName", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetServerName", returns);

    params.clear(); returns.clear();
    setDescription("SetTimeZone", "Set the time zone of the server. See also: \"GetTimeZones\"");
    params.insert("timeZone",  JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("SetTimeZone", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetTimeZone", returns);

    params.clear(); returns.clear();
    setDescription("SetLanguage", "Sets the server language to the given language. See also: \"GetAvailableLanguages\"");
    params.insert("language",  JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("SetLanguage", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetLanguage", returns);

    params.clear(); returns.clear();
    setDescription("SetDebugServerEnabled", "Enable or disable the debug server.");
    params.insert("enabled",  JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("SetDebugServerEnabled", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetDebugServerEnabled", returns);

    params.clear(); returns.clear();
    setDescription("SetTcpServerConfiguration", "Configure a TCP interface of the server. If the ID is an existing one, the existing config will be modified, otherwise a new one will be added. Note: if you are changing the configuration for the interface you are currently connected to, the connection will be dropped.");
    params.insert("configuration", JsonTypes::serverConfigurationRef());
    setParams("SetTcpServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetTcpServerConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("DeleteTcpServerConfiguration", "Delete a TCP interface of the server. Note: if you are deleting the configuration for the interface you are currently connected to, the connection will be dropped.");
    params.insert("id", JsonTypes::basicTypeToString(QVariant::String));
    setParams("DeleteTcpServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("DeleteTcpServerConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("SetWebSocketServerConfiguration", "Configure a WebSocket Server interface of the server. If the ID is an existing one, the existing config will be modified, otherwise a new one will be added. Note: if you are changing the configuration for the interface you are currently connected to, the connection will be dropped.");
    params.insert("configuration", JsonTypes::serverConfigurationRef());
    setParams("SetWebSocketServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetWebSocketServerConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("DeleteWebSocketServerConfiguration", "Delete a WebSocket Server interface of the server. Note: if you are deleting the configuration for the interface you are currently connected to, the connection will be dropped.");
    params.insert("id", JsonTypes::basicTypeToString(QVariant::String));
    setParams("DeleteWebSocketServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("DeleteWebSocketServerConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("SetWebServerConfiguration", "Configure a WebServer interface of the server. If the ID is an existing one, the existing config will be modified, otherwise a new one will be added.");
    params.insert("configuration", JsonTypes::webServerConfigurationRef());
    setParams("SetWebServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetWebServerConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("DeleteWebServerConfiguration", "Delete a WebServer interface of the server.");
    params.insert("id", JsonTypes::basicTypeToString(QVariant::String));
    setParams("DeleteWebServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("DeleteWebServerConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("SetCloudEnabled", "Sets whether the cloud connection is enabled or disabled in the settings.");
    params.insert("enabled", JsonTypes::basicTypeToString(QVariant::Bool));
    setParams("SetCloudEnabled", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetCloudEnabled", returns);

    // MQTT
    params.clear(); returns.clear();
    setDescription("GetMqttServerConfigurations", "Get all MQTT Server configurations.");
    setParams("GetMqttServerConfigurations", params);
    returns.insert("mqttServerConfigurations", QVariantList() << JsonTypes::serverConfigurationRef());
    setReturns("GetMqttServerConfigurations", returns);

    params.clear(); returns.clear();
    setDescription("SetMqttServerConfiguration", "Configure a MQTT Server interface on the MQTT broker. If the ID is an existing one, the existing config will be modified, otherwise a new one will be added. Setting authenticationEnabled to true will require MQTT clients to use credentials set in the MQTT broker policies.");
    params.insert("configuration", JsonTypes::serverConfigurationRef());
    setParams("SetMqttServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetMqttServerConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("DeleteMqttServerConfiguration", "Delete a MQTT Server interface of the server.");
    params.insert("id", JsonTypes::basicTypeToString(QVariant::String));
    setParams("DeleteMqttServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("DeleteMqttServerConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("GetMqttPolicies", "Get all MQTT broker policies.");
    setParams("GetMqttPolicies", params);
    returns.insert("mqttPolicies", QVariantList() << JsonTypes::mqttPolicyRef());
    setReturns("GetMqttPolicies", returns);

    params.clear(); returns.clear();
    setDescription("SetMqttPolicy", "Configure a MQTT broker policy. If the ID is an existing one, the existing policy will be modified, otherwise a new one will be added.");
    params.insert("policy", JsonTypes::mqttPolicyRef());
    setParams("SetMqttPolicy", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetMqttPolicy", returns);

    params.clear(); returns.clear();
    setDescription("DeleteMqttPolicy", "Delete a MQTT policy from the broker.");
    params.insert("clientId", JsonTypes::basicTypeToString(QVariant::String));
    setParams("DeleteMqttPolicy", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("DeleteMqttPolicy", returns);

    // Notifications
    params.clear(); returns.clear();
    setDescription("BasicConfigurationChanged", "Emitted whenever the basic configuration of this server changes.");
    params.insert("basicConfiguration", basicConfiguration);
    setParams("BasicConfigurationChanged", params);

    params.clear(); returns.clear();
    setDescription("LanguageChanged", "Emitted whenever the language of the server changed. The Plugins, Vendors and DeviceClasses have to be reloaded to get the translated data.");
    params.insert("language", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("LanguageChanged", params);

    params.clear(); returns.clear();
    setDescription("TcpServerConfigurationChanged", "Emitted whenever the TCP server configuration changes.");
    params.insert("tcpServerConfiguration", JsonTypes::serverConfigurationRef());
    setParams("TcpServerConfigurationChanged", params);

    params.clear(); returns.clear();
    setDescription("TcpServerConfigurationRemoved", "Emitted whenever a TCP server configuration is removed.");
    params.insert("id", JsonTypes::basicTypeToString(QVariant::String));
    setParams("TcpServerConfigurationRemoved", params);

    params.clear(); returns.clear();
    setDescription("WebSocketServerConfigurationChanged", "Emitted whenever the web socket server configuration changes.");
    params.insert("webSocketServerConfiguration", JsonTypes::serverConfigurationRef());
    setParams("WebSocketServerConfigurationChanged", params);

    params.clear(); returns.clear();
    setDescription("WebSocketServerConfigurationRemoved", "Emitted whenever a WebSocket server configuration is removed.");
    params.insert("id", JsonTypes::basicTypeToString(QVariant::String));
    setParams("WebSocketServerConfigurationRemoved", params);

    params.clear(); returns.clear();
    setDescription("MqttServerConfigurationChanged", "Emitted whenever the MQTT broker configuration is changed.");
    params.insert("mqttServerConfiguration", JsonTypes::serverConfigurationRef());
    setParams("MqttServerConfigurationChanged", params);

    params.clear(); returns.clear();
    setDescription("MqttServerConfigurationRemoved", "Emitted whenever a MQTT server configuration is removed.");
    params.insert("id", JsonTypes::basicTypeToString(QVariant::String));
    setParams("MqttServerConfigurationRemoved", params);

    params.clear(); returns.clear();
    setDescription("WebServerConfigurationChanged", "Emitted whenever the web server configuration changes.");
    params.insert("webServerConfiguration", JsonTypes::webServerConfigurationRef());
    setParams("WebServerConfigurationChanged", params);

    params.clear(); returns.clear();
    setDescription("WebServerConfigurationRemoved", "Emitted whenever a Web server configuration is removed.");
    params.insert("id", JsonTypes::basicTypeToString(QVariant::String));
    setParams("WebServerConfigurationRemoved", params);

    params.clear(); returns.clear();
    setDescription("CloudConfigurationChanged", "Emitted whenever the cloud configuration is changed.");
    params.insert("cloudConfiguration", cloudConfiguration);
    setParams("CloudConfigurationChanged", params);

    params.clear(); returns.clear();
    setDescription("MqttPolicyChanged", "Emitted whenever a MQTT broker policy is changed.");
    params.insert("policy", JsonTypes::mqttPolicyRef());
    setParams("MqttPolicyChanged", params);

    params.clear(); returns.clear();
    setDescription("MqttPolicyRemoved", "Emitted whenever a MQTT broker policy is removed.");
    params.insert("clientId", JsonTypes::basicTypeToString(QVariant::String));
    setParams("MqttPolicyRemoved", params);

    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::serverNameChanged, this, &ConfigurationHandler::onBasicConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::timeZoneChanged, this, &ConfigurationHandler::onBasicConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::localeChanged, this, &ConfigurationHandler::onBasicConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::debugServerEnabledChanged, this, &ConfigurationHandler::onBasicConfigurationChanged);
    connect(NymeaCore::instance()->deviceManager(), &DeviceManager::languageUpdated, this, &ConfigurationHandler::onLanguageChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::tcpServerConfigurationChanged, this, &ConfigurationHandler::onTcpServerConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::tcpServerConfigurationRemoved, this, &ConfigurationHandler::onTcpServerConfigurationRemoved);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::webServerConfigurationChanged, this, &ConfigurationHandler::onWebServerConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::webServerConfigurationRemoved, this, &ConfigurationHandler::onWebServerConfigurationRemoved);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::webSocketServerConfigurationChanged, this, &ConfigurationHandler::onWebSocketServerConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::webSocketServerConfigurationRemoved, this, &ConfigurationHandler::onWebSocketServerConfigurationRemoved);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::mqttServerConfigurationChanged, this, &ConfigurationHandler::onMqttServerConfigurationChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::mqttServerConfigurationRemoved, this, &ConfigurationHandler::onMqttServerConfigurationRemoved);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::mqttPolicyChanged, this, &ConfigurationHandler::onMqttPolicyChanged);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::mqttPolicyRemoved, this, &ConfigurationHandler::onMqttPolicyRemoved);
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::cloudEnabledChanged, this, &ConfigurationHandler::onCloudConfigurationChanged);
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
    returns.insert("basicConfiguration", JsonTypes::packBasicConfiguration());
    QVariantList tcpServerConfigs;
    foreach (const ServerConfiguration &config, NymeaCore::instance()->configuration()->tcpServerConfigurations()) {
        tcpServerConfigs.append(JsonTypes::packServerConfiguration(config));
    }
    returns.insert("tcpServerConfigurations", tcpServerConfigs);

    QVariantList webServerConfigs;
    foreach (const WebServerConfiguration &config, NymeaCore::instance()->configuration()->webServerConfigurations()) {
        webServerConfigs.append(JsonTypes::packWebServerConfiguration(config));

    }
    returns.insert("webServerConfigurations", webServerConfigs);

    QVariantList webSocketServerConfigs;
    foreach (const ServerConfiguration &config, NymeaCore::instance()->configuration()->webSocketServerConfigurations()) {
        webSocketServerConfigs.append(JsonTypes::packServerConfiguration(config));
    }
    returns.insert("webSocketServerConfigurations", webSocketServerConfigs);

    QVariantMap cloudConfig;
    cloudConfig.insert("enabled", NymeaCore::instance()->configuration()->cloudEnabled());
    returns.insert("cloud", cloudConfig);

    return createReply(returns);
}

JsonReply *ConfigurationHandler::GetTimeZones(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantList timeZones;
    foreach (const QByteArray &timeZoneId, NymeaCore::instance()->timeManager()->availableTimeZones()) {
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

    QByteArray timeZone = params.value("timeZone").toString().toUtf8();
    if (!NymeaCore::instance()->timeManager()->setTimeZone(timeZone))
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidTimeZone));

    NymeaCore::instance()->configuration()->setTimeZone(timeZone);
    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetLanguage(const QVariantMap &params) const
{
    qCDebug(dcJsonRpc()) << "Setting language to" << params.value("language").toString();
    QLocale locale(params.value("language").toString());

    NymeaCore::instance()->configuration()->setLocale(locale);

    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetTcpServerConfiguration(const QVariantMap &params) const
{
    ServerConfiguration config = JsonTypes::unpackServerConfiguration(params.value("configuration").toMap());
    if (config.id.isEmpty()) {
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidId));
    }
    if (config.address.isNull())
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidHostAddress));

    if (config.port <= 0 || config.port > 65535) {
        qCWarning(dcJsonRpc()) << "Port out of range";
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidPort));
    }

    qCDebug(dcJsonRpc()) << QString("Configure TCP server %1:%2").arg(config.address.toString()).arg(config.port);

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
    WebServerConfiguration config = JsonTypes::unpackWebServerConfiguration(params.value("configuration").toMap());

    if (config.id.isEmpty()) {
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidId));
    }
    if (config.address.isNull())
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidHostAddress));

    if (config.port <= 0 || config.port > 65535) {
        qCWarning(dcJsonRpc()) << "Port out of range";
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidPort));
    }

    qCDebug(dcJsonRpc()) << QString("Configure web server %1:%2").arg(config.address.toString()).arg(config.port);

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
    ServerConfiguration config = JsonTypes::unpackServerConfiguration(params.value("configuration").toMap());
    if (config.id.isEmpty()) {
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidId));
    }
    if (config.address.isNull())
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidHostAddress));

    if (config.port <= 0 || config.port > 65535) {
        qCWarning(dcJsonRpc()) << "Port out of range";
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidPort));
    }

    qCDebug(dcJsonRpc()) << QString("Configuring web socket server %1:%2").arg(config.address.toString()).arg(config.port);

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

JsonReply *ConfigurationHandler::GetMqttServerConfigurations(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap ret;
    QVariantList mqttServerConfigs;
    foreach (const ServerConfiguration &config, NymeaCore::instance()->configuration()->mqttServerConfigurations()) {
        mqttServerConfigs << JsonTypes::packServerConfiguration(config);
    }
    ret.insert("mqttServerConfigurations", mqttServerConfigs);
    return createReply(ret);
}

JsonReply *ConfigurationHandler::SetMqttServerConfiguration(const QVariantMap &params) const
{
    ServerConfiguration config = JsonTypes::unpackServerConfiguration(params.value("configuration").toMap());
    if (config.id.isEmpty()) {
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidId));
    }
    if (config.address.isNull())
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidHostAddress));

    if (config.port <= 0 || config.port > 65535) {
        qCWarning(dcJsonRpc()) << "Port out of range";
        return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorInvalidPort));
    }

    qCDebug(dcJsonRpc()) << QString("Configure MQTT server %1:%2").arg(config.address.toString()).arg(config.port);

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
        mqttPolicies << JsonTypes::packMqttPolicy(policy);
    }
    QVariantMap ret;
    ret.insert("mqttPolicies", mqttPolicies);
    return createReply(ret);
}

JsonReply *ConfigurationHandler::SetMqttPolicy(const QVariantMap &params) const
{
    MqttPolicy policy = JsonTypes::unpackMqttPolicy(params.value("policy").toMap());
    NymeaCore::instance()->configuration()->updateMqttPolicy(policy);
    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::DeleteMqttPolicy(const QVariantMap &params) const
{
    QString clientId = params.value("clientId").toString();
    bool success = NymeaCore::instance()->configuration()->removeMqttPolicy(clientId);
    return createReply(statusToReply(success ? NymeaConfiguration::ConfigurationErrorNoError : NymeaConfiguration::ConfigurationErrorInvalidId));
}

JsonReply *ConfigurationHandler::SetCloudEnabled(const QVariantMap &params) const
{
    bool enabled = params.value("enabled").toBool();
    NymeaCore::instance()->configuration()->setCloudEnabled(enabled);
    return createReply(statusToReply(NymeaConfiguration::ConfigurationErrorNoError));
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
    params.insert("basicConfiguration", JsonTypes::packBasicConfiguration());
    emit BasicConfigurationChanged(params);
}

void ConfigurationHandler::onTcpServerConfigurationChanged(const QString &id)
{
    qCDebug(dcJsonRpc()) << "Notification: TCP server configuration changed";
    QVariantMap params;
    params.insert("tcpServerConfiguration", JsonTypes::packServerConfiguration(NymeaCore::instance()->configuration()->tcpServerConfigurations().value(id)));
    emit TcpServerConfigurationChanged(params);
}

void ConfigurationHandler::onTcpServerConfigurationRemoved(const QString &id)
{
    qCDebug(dcJsonRpc) << "Notification: TCP server configuration removed";
    QVariantMap params;
    params.insert("id", id);
    emit TcpServerConfigurationRemoved(params);
}

void ConfigurationHandler::onWebServerConfigurationChanged(const QString &id)
{
    qCDebug(dcJsonRpc()) << "Notification: web server configuration changed";
    QVariantMap params;
    params.insert("webServerConfiguration", JsonTypes::packWebServerConfiguration(NymeaCore::instance()->configuration()->webServerConfigurations().value(id)));
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
    params.insert("webSocketServerConfiguration", JsonTypes::packServerConfiguration(NymeaCore::instance()->configuration()->webSocketServerConfigurations().value(id)));
    emit WebSocketServerConfigurationChanged(params);
}

void ConfigurationHandler::onWebSocketServerConfigurationRemoved(const QString &id)
{
    qCDebug(dcJsonRpc()) << "Notification: WebSocket server configuration removed";
    QVariantMap params;
    params.insert("id", id);
    emit WebSocketServerConfigurationRemoved(params);
}

void ConfigurationHandler::onMqttServerConfigurationChanged(const QString &id)
{
    qCDebug(dcJsonRpc()) << "Notification: MQTT server configuration changed";
    QVariantMap params;
    params.insert("mqttServerConfiguration", JsonTypes::packServerConfiguration(NymeaCore::instance()->configuration()->mqttServerConfigurations().value(id)));
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
    params.insert("policy", JsonTypes::packMqttPolicy(NymeaCore::instance()->configuration()->mqttPolicies().value(clientId)));
    emit MqttPolicyChanged(params);
}

void ConfigurationHandler::onMqttPolicyRemoved(const QString &clientId)
{
    qCDebug(dcJsonRpc()) << "Notification: MQTT policy removed";
    QVariantMap params;
    params.insert("clientId", clientId);
    emit MqttPolicyRemoved(params);
}

void ConfigurationHandler::onCloudConfigurationChanged(bool enabled)
{
    qCDebug(dcJsonRpc()) << "Notification: cloud configuration changed";
    QVariantMap params;
    QVariantMap cloudConfiguration;
    cloudConfiguration.insert("enabled", enabled);
    params.insert("cloudConfiguration", cloudConfiguration);
    emit CloudConfigurationChanged(params);
}

void ConfigurationHandler::onLanguageChanged()
{
    qCDebug(dcJsonRpc()) << "Notification: language configuration changed";
    QVariantMap params;
    params.insert("language", NymeaCore::instance()->configuration()->locale().name());
    emit LanguageChanged(params);
}

}
