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

#ifndef NYMEACONFIGURATION_H
#define NYMEACONFIGURATION_H

#include <QHostAddress>
#include <QObject>
#include <QLocale>
#include <QUuid>
#include <QUrl>

namespace nymeaserver {

class ServerConfiguration {
public:
    QString id;
    QHostAddress address;
    uint port = 0;
    bool sslEnabled = true;
    bool authenticationEnabled = true;

    bool operator==(const ServerConfiguration &other) {
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
public:
    QString publicFolder;
    bool restServerEnabled = false;
};

class NymeaConfiguration : public QObject
{
    Q_OBJECT
    Q_ENUMS(ConfigurationError)

public:
    enum ConfigurationError {
        ConfigurationErrorNoError,
        ConfigurationErrorInvalidTimeZone,
        ConfigurationErrorInvalidStationName,
        ConfigurationErrorInvalidId,
        ConfigurationErrorInvalidPort,
        ConfigurationErrorInvalidHostAddress,
        ConfigurationErrorBluetoothHardwareNotAvailable,
        ConfigurationErrorInvalidCertificate
    };

    explicit NymeaConfiguration(QObject *parent = 0);

    // Global settings
    QUuid serverUuid() const;

    QString serverName() const;
    void setServerName(const QString &serverName);

    QByteArray timeZone() const;
    void setTimeZone(const QByteArray &timeZone);

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

    // Bluetooth
    bool bluetoothServerEnabled() const;
    void setBluetoothServerEnabled(const bool &enabled);

    // Cloud
    bool cloudEnabled() const;
    void setCloudEnabled(bool enabled);

    QString cloudServerUrl() const;
    void setCloudServerUrl(const QString &cloudServerUrl);
    QString cloudCertificateCA() const;
    void setCloudCertificateCA(const QString &cloudCertificateCA);
    QString cloudCertificate() const;
    void setCloudCertificate(const QString &cloudCertificate);
    QString cloudCertificateKey() const;
    void setCloudCertificateKey(const QString &cloudCertificateKey);

    // Logging
    QString logDBDriver() const;
    QString logDBName() const;
    QString logDBHost() const;
    QString logDBUser() const;
    QString logDBPassword() const;
    int logDBMaxEntries() const;

private:
    QHash<QString, ServerConfiguration> m_tcpServerConfigs;
    QHash<QString, WebServerConfiguration> m_webServerConfigs;
    QHash<QString, ServerConfiguration> m_webSocketServerConfigs;

    void setServerUuid(const QUuid &uuid);
    void setWebServerPublicFolder(const QString & path);

    QString defaultWebserverPublicFolderPath() const;

    void storeServerConfig(const QString &group, const ServerConfiguration &config);
    ServerConfiguration readServerConfig(const QString &group, const QString &id);
    void deleteServerConfig(const QString &group, const QString &id);
    void storeWebServerConfig(const WebServerConfiguration &config);
    WebServerConfiguration readWebServerConfig(const QString &id);

signals:
    void serverNameChanged(const QString &serverName);
    void timeZoneChanged();
    void localeChanged();

    void tcpServerConfigurationChanged(const QString &configId);
    void tcpServerConfigurationRemoved(const QString &configId);
    void webServerConfigurationChanged(const QString &configId);
    void webServerConfigurationRemoved(const QString &configId);
    void webSocketServerConfigurationChanged(const QString &configId);
    void webSocketServerConfigurationRemoved(const QString &configId);

    void bluetoothServerEnabledChanged();
    void cloudEnabledChanged(bool enabled);
    void debugServerEnabledChanged(bool enabled);
};

}

#endif // NYMEACONFIGURATION_H
