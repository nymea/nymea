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

/*!
    \class guhserver::ConfigurationHandler
    \brief This subclass of \l{JsonHandler} processes the JSON requests for the \tt Configuration namespace.

    \ingroup json
    \inmodule core

    This \l{JsonHandler} will be created in the \l{JsonRPCServer} and used to handle JSON-RPC requests
    for the \tt {Configuration} namespace of the API.

    \sa JsonHandler, JsonRPCServer
*/

/*! \fn void guhserver::ConfigurationHandler::BasicConfigurationChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the configurations of the server have been changed.
    The \a params contains the map for the notification.
*/

/*! \fn void guhserver::ConfigurationHandler::TcpServerConfigurationChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the configurations of the \l{TcpServer} have been changed.
    The \a params contains the map for the notification.
*/

/*! \fn void guhserver::ConfigurationHandler::WebServerConfigurationChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the configurations of the \l{WebServer} have been changed.
    The \a params contains the map for the notification.
*/

/*! \fn void guhserver::ConfigurationHandler::WebSocketServerConfigurationChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the configurations of the \l{WebSocketServer} have been changed.
    The \a params contains the map for the notification.
*/

/*! \fn void guhserver::ConfigurationHandler::LanguageChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the language of the system has changed.
    The \a params contains the map for the notification.
*/



#include "configurationhandler.h"
#include "guhcore.h"

namespace guhserver {

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
    returns.insert("basicConfiguration", basicConfiguration);
    QVariantMap tcpServerConfiguration;
    tcpServerConfiguration.insert("host", JsonTypes::basicTypeToString(JsonTypes::String));
    tcpServerConfiguration.insert("port", JsonTypes::basicTypeToString(JsonTypes::Uint));
    returns.insert("tcpServerConfiguration", tcpServerConfiguration);
    QVariantMap webServerConfiguration;
    webServerConfiguration.insert("host", JsonTypes::basicTypeToString(JsonTypes::String));
    webServerConfiguration.insert("port", JsonTypes::basicTypeToString(JsonTypes::Uint));
    returns.insert("webServerConfiguration", webServerConfiguration);
    QVariantMap webSocketServerConfiguration;
    webSocketServerConfiguration.insert("host", JsonTypes::basicTypeToString(JsonTypes::String));
    webSocketServerConfiguration.insert("port", JsonTypes::basicTypeToString(JsonTypes::Uint));
    returns.insert("webSocketServerConfiguration", webSocketServerConfiguration);
    QVariantMap sslConfiguration;
    sslConfiguration.insert("enabled", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("sslConfiguration", sslConfiguration);
    setReturns("GetConfigurations", returns);

    params.clear(); returns.clear();
    setDescription("SetServerName", "Set the name of the server. Default is guhIO.");
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
    setDescription("SetTcpServerConfiguration", "Configure the TCP interface of the server. Note: if you are using the TCP server for this call you will loose the connection.");
    params.insert("host", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("port", JsonTypes::basicTypeToString(JsonTypes::Uint));
    setParams("SetTcpServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetTcpServerConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("SetWebSocketServerConfiguration", "Configure the web socket interface of the server. Note: if you are using the web socket server for this call you will loose the connection.");
    params.insert("host", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("port", JsonTypes::basicTypeToString(JsonTypes::Uint));
    setParams("SetWebSocketServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetWebSocketServerConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("SetWebServerConfiguration", "Configure the web server of the server.");
    params.insert("host", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("port", JsonTypes::basicTypeToString(JsonTypes::Uint));
    setParams("SetWebServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetWebServerConfiguration", returns);

    // Notifications
    params.clear(); returns.clear();
    setDescription("BasicConfigurationChanged", "Emitted whenever the basic configuration of this server changes.");
    params.insert("serverName", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("serverUuid", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("serverTime", JsonTypes::basicTypeToString(JsonTypes::Uint));
    params.insert("timeZone", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("BasicConfigurationChanged", params);

    params.clear(); returns.clear();
    setDescription("TcpServerConfigurationChanged", "Emitted whenever the TCP server configuration changes.");
    params.insert("host", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("port", JsonTypes::basicTypeToString(JsonTypes::Uint));
    setParams("TcpServerConfigurationChanged", params);

    params.clear(); returns.clear();
    setDescription("WebServerConfigurationChanged", "Emitted whenever the web server configuration changes.");
    params.insert("host", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("port", JsonTypes::basicTypeToString(JsonTypes::Uint));
    setParams("WebServerConfigurationChanged", params);

    params.clear(); returns.clear();
    setDescription("WebSocketServerConfigurationChanged", "Emitted whenever the web socket server configuration changes.");
    params.insert("host", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("port", JsonTypes::basicTypeToString(JsonTypes::Uint));
    setParams("WebSocketServerConfigurationChanged", params);

    params.clear(); returns.clear();
    setDescription("LanguageChanged", "Emitted whenever the language of the server changed. The Plugins, Vendors and DeviceClasses have to be reloaded to get the translated data.");
    params.insert("language", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("LanguageChanged", params);

    connect(GuhCore::instance()->configuration(), &GuhConfiguration::serverNameChanged, this, &ConfigurationHandler::onBasicConfigurationChanged);
    connect(GuhCore::instance()->configuration(), &GuhConfiguration::timeZoneChanged, this, &ConfigurationHandler::onBasicConfigurationChanged);
    connect(GuhCore::instance()->configuration(), &GuhConfiguration::localeChanged, this, &ConfigurationHandler::onBasicConfigurationChanged);
    connect(GuhCore::instance()->configuration(), &GuhConfiguration::tcpServerConfigurationChanged, this, &ConfigurationHandler::onTcpServerConfigurationChanged);
    connect(GuhCore::instance()->configuration(), &GuhConfiguration::webServerConfigurationChanged, this, &ConfigurationHandler::onWebServerConfigurationChanged);
    connect(GuhCore::instance()->configuration(), &GuhConfiguration::webSocketServerConfigurationChanged, this, &ConfigurationHandler::onWebSocketServerConfigurationChanged);
    connect(GuhCore::instance()->deviceManager(), &DeviceManager::languageUpdated, this, &ConfigurationHandler::onLanguageChanged);
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
    returns.insert("tcpServerConfiguration", JsonTypes::packTcpServerConfiguration());
    returns.insert("webServerConfiguration", JsonTypes::packWebServerConfiguration());
    returns.insert("webSocketServerConfiguration", JsonTypes::packWebSocketServerConfiguration());
    QVariantMap sslConfiguration;
    sslConfiguration.insert("enabled", GuhCore::instance()->configuration()->sslEnabled());
    returns.insert("sslConfiguration", sslConfiguration);
    return createReply(returns);
}

JsonReply *ConfigurationHandler::GetTimeZones(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantList timeZones;
    foreach (const QByteArray &timeZoneId, GuhCore::instance()->timeManager()->availableTimeZones()) {
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
    foreach (const QString &language, GuhCore::getAvailableLanguages()) {
        languages.append(language);
    }
    QVariantMap returns;
    returns.insert("languages", languages);
    return createReply(returns);
}

JsonReply *ConfigurationHandler::SetServerName(const QVariantMap &params) const
{
    QString serverName = params.value("serverName").toString();
    GuhCore::instance()->configuration()->setServerName(serverName);
    return createReply(statusToReply(GuhConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetTimeZone(const QVariantMap &params) const
{
    qCDebug(dcJsonRpc()) << "Setting time zone to" << params.value("timeZone").toString();

    QByteArray timeZone = params.value("timeZone").toString().toUtf8();
    if (!GuhCore::instance()->timeManager()->setTimeZone(timeZone))
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidTimeZone));

    GuhCore::instance()->configuration()->setTimeZone(timeZone);
    return createReply(statusToReply(GuhConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetLanguage(const QVariantMap &params) const
{
    qCDebug(dcJsonRpc()) << "Setting language to" << params.value("language").toString();
    QLocale locale(params.value("language").toString());

    GuhCore::instance()->configuration()->setLocale(locale);

    return createReply(statusToReply(GuhConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetTcpServerConfiguration(const QVariantMap &params) const
{
    QHostAddress host = QHostAddress(params.value("host").toString());
    if (host.isNull())
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidHostAddress));

    uint port = params.value("port").toUInt();
    if (port <= 0 || port > 65535) {
        qCWarning(dcJsonRpc()) << "Port out of range";
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidPort));
    }

    qCDebug(dcJsonRpc()) << QString("Configure TCP server %1:%2").arg(host.toString()).arg(port);

    if (!GuhCore::instance()->tcpServer()->reconfigureServer(host, port))
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidPort));

    GuhCore::instance()->configuration()->setTcpServerConfiguration(port, host);

    return createReply(statusToReply(GuhConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetWebServerConfiguration(const QVariantMap &params) const
{
    QHostAddress host = QHostAddress(params.value("host").toString());
    if (host.isNull())
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidHostAddress));

    uint port = params.value("port").toUInt();
    if (port <= 0 || port > 65535) {
        qCWarning(dcJsonRpc()) << "Port out of range";
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidPort));
    }

    qCDebug(dcJsonRpc()) << QString("Configure web server %1:%2").arg(host.toString()).arg(port);

    if (!GuhCore::instance()->webServer()->reconfigureServer(host, port))
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidPort));

    GuhCore::instance()->configuration()->setWebServerConfiguration(port, host);
    return createReply(statusToReply(GuhConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetWebSocketServerConfiguration(const QVariantMap &params) const
{
    QHostAddress host = QHostAddress(params.value("host").toString());
    if (host.isNull())
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidHostAddress));

    uint port = params.value("port").toUInt();
    if (port <= 0 || port > 65535) {
        qCWarning(dcJsonRpc()) << "Port out of range";
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidPort));
    }

    qCDebug(dcJsonRpc()) << QString("Configure web socket server %1:%2").arg(host.toString()).arg(port);

    if (!GuhCore::instance()->webSocketServer()->reconfigureServer(host, port))
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidPort));

    GuhCore::instance()->configuration()->setWebServerConfiguration(port, host);
    return createReply(statusToReply(GuhConfiguration::ConfigurationErrorNoError));
}

void ConfigurationHandler::onBasicConfigurationChanged()
{
    QVariantMap params;
    qCDebug(dcJsonRpc()) << "Notification: Basic configuration changed";
    params.insert("basicConfiguration", JsonTypes::packBasicConfiguration());
    emit BasicConfigurationChanged(params);
}

void ConfigurationHandler::onTcpServerConfigurationChanged()
{
    QVariantMap params;
    qCDebug(dcJsonRpc()) << "Notification: TCP server configuration changed";
    params.insert("tcpServerConfiguration", JsonTypes::packTcpServerConfiguration());
    emit TcpServerConfigurationChanged(params);
}

void ConfigurationHandler::onWebServerConfigurationChanged()
{
    QVariantMap params;
    qCDebug(dcJsonRpc()) << "Notification: web server configuration changed";
    params.insert("webServerConfiguration", JsonTypes::packWebServerConfiguration());
    emit WebServerConfigurationChanged(params);
}

void ConfigurationHandler::onWebSocketServerConfigurationChanged()
{
    QVariantMap params;
    qCDebug(dcJsonRpc()) << "Notification: web socket server configuration changed";
    params.insert("webSocketServerConfiguration", JsonTypes::packWebSocketServerConfiguration());
    emit WebSocketServerConfigurationChanged(params);
}

void ConfigurationHandler::onLanguageChanged()
{
    QVariantMap params;
    qCDebug(dcJsonRpc()) << "Notification: language configuration changed";
    params.insert("language", GuhCore::instance()->configuration()->locale().name());
    emit LanguageChanged(params);
}

}
