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

#ifndef GUHCONFIGURATION_H
#define GUHCONFIGURATION_H

#include <QHostAddress>
#include <QObject>
#include <QLocale>
#include <QUuid>
#include <QUrl>

namespace guhserver {

class GuhConfiguration : public QObject
{
    Q_OBJECT
    Q_ENUMS(ConfigurationError)

public:
    enum ConfigurationError {
        ConfigurationErrorNoError,
        ConfigurationErrorInvalidTimeZone,
        ConfigurationErrorInvalidStationName,
        ConfigurationErrorInvalidPort,
        ConfigurationErrorInvalidHostAddress,
        ConfigurationErrorBluetoothHardwareNotAvailable,
        ConfigurationErrorInvalidCertificate
    };

    explicit GuhConfiguration(QObject *parent = 0);

    // Global settings
    QUuid serverUuid() const;

    QString serverName() const;
    void setServerName(const QString &serverName);

    QByteArray timeZone() const;
    void setTimeZone(const QByteArray &timeZone);

    QLocale locale() const;
    void setLocale(const QLocale &locale);

    // TCP server
    uint tcpServerPort() const;
    QHostAddress tcpServerAddress() const;
    void setTcpServerConfiguration(const uint &port, const QHostAddress &address);

    // Webserver
    uint webServerPort() const;
    QHostAddress webServerAddress() const;
    QString webServerPublicFolder() const;
    void setWebServerConfiguration(const uint &port, const QHostAddress &address);

    // Websocket
    uint webSocketPort() const;
    QHostAddress webSocketAddress() const;
    void setWebSocketConfiguration(const uint &port, const QHostAddress &address);

    // Bluetooth
    bool bluetoothServerEnabled() const;
    void setBluetoothServerEnabled(const bool &enabled);

    // SSL configuration
    bool sslEnabled() const;
    void setSslEnabled(const bool &sslEnabled);

    QString sslCertificate() const;
    QString sslCertificateKey() const;
    void setSslCertificate(const QString &sslCertificate, const QString &sslCertificateKey);

private:
    QUuid m_serverUuid;
    QString m_serverName;
    QByteArray m_timeZone;
    QLocale m_locale;

    QHostAddress m_tcpServerAddress;
    uint m_tcpServerPort;

    QHostAddress m_webServerAddress;
    uint m_webServerPort;
    QString m_webServerPublicFolder;

    QHostAddress m_webSocketAddress;
    uint m_webSocketPort;

    bool m_bluetoothServerEnabled;

    bool m_sslEnabled;
    QString m_sslCertificate;
    QString m_sslCertificateKey;

    void setServerUuid(const QUuid &uuid);
    void setWebServerPublicFolder(const QString & path);

signals:
    void serverNameChanged();
    void timeZoneChanged();
    void localeChanged();

    void tcpServerConfigurationChanged();
    void webServerConfigurationChanged();
    void webSocketServerConfigurationChanged();

    void bluetoothServerEnabledChanged();

    void sslEnabledChanged();
    void sslCertificateChanged();
};

}

#endif // GUHCONFIGURATION_H
