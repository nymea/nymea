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

#ifndef NYMEACONFIGURATION_H
#define NYMEACONFIGURATION_H

#include <QHostAddress>
#include <QObject>
#include <QLocale>
#include <QUuid>
#include <QUrl>

#include "nymeasettings.h"

namespace nymeaserver {

class ServerConfiguration {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString address MEMBER address)
    Q_PROPERTY(uint port MEMBER port)
    Q_PROPERTY(bool sslEnabled MEMBER sslEnabled)
    Q_PROPERTY(bool authenticationEnabled MEMBER authenticationEnabled)

public:
    QString id;
    QString address;
    void setAddress(const QString &address) {this->address = address; }
    uint port = 0;
    bool sslEnabled = true;
    bool authenticationEnabled = true;

    bool operator==(const ServerConfiguration &other) const {
        return id == other.id
                && address == other.address
                && port == other.port
                && sslEnabled == other.sslEnabled
                && authenticationEnabled == other.authenticationEnabled;
    }
};

QDebug operator <<(QDebug debug, const ServerConfiguration &configuration);

class WebServerConfiguration: public ServerConfiguration
{
    Q_GADGET
    Q_PROPERTY(QString publicFolder MEMBER publicFolder)

public:
    QString publicFolder;
    bool restServerEnabled = false;

    bool operator==(const WebServerConfiguration &other) const {
        return ServerConfiguration::operator==(other)
                && publicFolder == other.publicFolder
                && restServerEnabled == other.restServerEnabled;
    }
};

class TunnelProxyServerConfiguration: public ServerConfiguration
{
    Q_GADGET
    Q_PROPERTY(bool ignoreSslErrors MEMBER ignoreSslErrors)

public:
    bool ignoreSslErrors = false;

    bool operator==(const TunnelProxyServerConfiguration &other) const {
        return ServerConfiguration::operator==(other)
                && ignoreSslErrors == other.ignoreSslErrors;
    }
};

QDebug operator <<(QDebug debug, const TunnelProxyServerConfiguration &configuration);

class MqttPolicy
{
    Q_GADGET
    Q_PROPERTY(QString clientId MEMBER clientId)
    Q_PROPERTY(QString username MEMBER username)
    Q_PROPERTY(QString password MEMBER password)
    Q_PROPERTY(QStringList allowedPublishTopicFilters MEMBER allowedPublishTopicFilters)
    Q_PROPERTY(QStringList allowedSubscribeTopicFilters MEMBER allowedSubscribeTopicFilters)

public:
    QString clientId;
    QString username;
    QString password;
    QStringList allowedSubscribeTopicFilters;
    QStringList allowedPublishTopicFilters;
};
typedef QList<MqttPolicy> MqttPolicies;

class NymeaConfiguration : public QObject
{
    Q_OBJECT

public:
    enum ConfigurationError {
        ConfigurationErrorNoError,
        ConfigurationErrorInvalidTimeZone,
        ConfigurationErrorInvalidStationName,
        ConfigurationErrorInvalidId,
        ConfigurationErrorInvalidPort,
        ConfigurationErrorInvalidHostAddress,
        ConfigurationErrorBluetoothHardwareNotAvailable,
        ConfigurationErrorInvalidCertificate,
        ConfigurationErrorUnsupported
    };
    Q_ENUM(ConfigurationError)

    explicit NymeaConfiguration(QObject *parent = nullptr);

    // Global settings
    QUuid serverUuid() const;

    QString serverName() const;
    void setServerName(const QString &serverName);

    QByteArray timeZone() const;
    void setTimeZone(const QByteArray &timeZone);

    double locationLatitude() const;
    double locationLongitude() const;
    QString locationName() const;
    void setLocation(double latitude, double longitude, const QString &name);

    QLocale locale() const;
    void setLocale(const QLocale &locale);

    QString sslCertificate() const;
    QString sslCertificateKey() const;
    void setSslCertificate(const QString &sslCertificate, const QString &sslCertificateKey);

    // Debug server
    bool debugServerEnabled() const;
    void setDebugServerEnabled(bool enabled);

    // TCP server
    QHash<QString, ServerConfiguration> tcpServerConfigurations() const;
    void setTcpServerConfiguration(const ServerConfiguration &config);
    void removeTcpServerConfiguration(const QString &id);

    // Web server
    QHash<QString, WebServerConfiguration> webServerConfigurations() const;
    void setWebServerConfiguration(const WebServerConfiguration &config);
    void removeWebServerConfiguration(const QString &id);

    // Websocket
    QHash<QString, ServerConfiguration> webSocketServerConfigurations() const;
    void setWebSocketServerConfiguration(const ServerConfiguration &config);
    void removeWebSocketServerConfiguration(const QString &id);

    // Tunnel proxy server
    QHash<QString, TunnelProxyServerConfiguration> tunnelProxyServerConfigurations() const;
    void setTunnelProxyServerConfiguration(const TunnelProxyServerConfiguration &config);
    void removeTunnelProxyServerConfiguration(const QString &id);

    // MQTT
    QHash<QString, ServerConfiguration> mqttServerConfigurations() const;
    void setMqttServerConfiguration(const ServerConfiguration &config);
    void removeMqttServerConfiguration(const QString &id);

    QHash<QString, MqttPolicy> mqttPolicies() const;
    void updateMqttPolicy(const MqttPolicy &policy);
    bool removeMqttPolicy(const QString &clientId);

    // Bluetooth
    bool bluetoothServerEnabled() const;
    void setBluetoothServerEnabled(bool enabled);

    // Logging
    QString logDBName() const;
    QString logDBHost() const;
    QString logDBUser() const;
    QString logDBPassword() const;

private:
    NymeaSettings *m_settings = nullptr;
    NymeaSettings *m_mqttPoliciesSettings = nullptr;
    QHash<QString, ServerConfiguration> m_tcpServerConfigs;
    QHash<QString, WebServerConfiguration> m_webServerConfigs;
    QHash<QString, ServerConfiguration> m_webSocketServerConfigs;
    QHash<QString, ServerConfiguration> m_mqttServerConfigs;
    QHash<QString, MqttPolicy> m_mqttPolicies;
    QHash<QString, TunnelProxyServerConfiguration> m_tunnelProxyServerConfigs;

    void setServerUuid(const QUuid &uuid);
    void setWebServerPublicFolder(const QString & path);

    QString defaultWebserverPublicFolderPath() const;

    void storeServerConfig(const QString &group, const ServerConfiguration &config);
    ServerConfiguration readServerConfig(const QString &id);
    void deleteServerConfig(const QString &group, const QString &id);
    void storeWebServerConfig(const WebServerConfiguration &config);
    WebServerConfiguration readWebServerConfig(const QString &id);
    void storeTunnelProxyServerConfig(const TunnelProxyServerConfiguration &config);
    TunnelProxyServerConfiguration readTunnelProxyServerConfig(const QString &id);

    void storeMqttPolicy(const MqttPolicy &policy);
    MqttPolicy readMqttPolicy(const QString &clientId);
    void deleteMqttPolicy(const QString &clientId);

signals:
    void serverNameChanged(const QString &serverName);
    void timeZoneChanged();
    void localeChanged();
    void locationChanged();

    void tcpServerConfigurationChanged(const QString &configId);
    void tcpServerConfigurationRemoved(const QString &configId);
    void webServerConfigurationChanged(const QString &configId);
    void webServerConfigurationRemoved(const QString &configId);
    void webSocketServerConfigurationChanged(const QString &configId);
    void webSocketServerConfigurationRemoved(const QString &configId);
    void mqttServerConfigurationChanged(const QString &configId);
    void mqttServerConfigurationRemoved(const QString &configId);
    void tunnelProxyServerConfigurationChanged(const QString &configId);
    void tunnelProxyServerConfigurationRemoved(const QString &configId);

    void mqttPolicyChanged(const QString &clientId);
    void mqttPolicyRemoved(const QString &clientId);

    void bluetoothServerEnabledChanged();
    void mqttBrokerEnabledChanged();
    void mqttPortChanged();
    void debugServerEnabledChanged(bool enabled);
};

}
Q_DECLARE_METATYPE(nymeaserver::ServerConfiguration)
Q_DECLARE_METATYPE(nymeaserver::MqttPolicy)

#endif // NYMEACONFIGURATION_H
