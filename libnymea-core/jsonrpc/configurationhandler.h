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

#ifndef CONFIGURATIONHANDLER_H
#define CONFIGURATIONHANDLER_H

#include <QObject>

#include "jsonrpc/jsonhandler.h"
#include "nymeaconfiguration.h"

namespace nymeaserver {

class ConfigurationHandler : public JsonHandler
{
    Q_OBJECT

public:
    ConfigurationHandler(QObject *parent = nullptr);
    QString name() const override;

    Q_INVOKABLE JsonReply *GetConfigurations(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetTimeZones(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetAvailableLanguages(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetServerName(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetTimeZone(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetLanguage(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetLocation(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetDebugServerEnabled(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetTcpServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *DeleteTcpServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetWebServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *DeleteWebServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetWebSocketServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *DeleteWebSocketServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetTunnelProxyServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *DeleteTunnelProxyServerConfiguration(const QVariantMap &params) const;

    Q_INVOKABLE JsonReply *GetBackupConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetBackupConfiguration(const QVariantMap &params) const;

    Q_INVOKABLE JsonReply *GetMqttServerConfigurations(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetMqttServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *DeleteMqttServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetMqttPolicies(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetMqttPolicy(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *DeleteMqttPolicy(const QVariantMap &params) const;

signals:
    void BasicConfigurationChanged(const QVariantMap &params);

    void TcpServerConfigurationChanged(const QVariantMap &params);
    void TcpServerConfigurationRemoved(const QVariantMap &params);
    void WebServerConfigurationChanged(const QVariantMap &params);
    void WebServerConfigurationRemoved(const QVariantMap &params);
    void WebSocketServerConfigurationChanged(const QVariantMap &params);
    void WebSocketServerConfigurationRemoved(const QVariantMap &params);
    void TunnelProxyServerConfigurationChanged(const QVariantMap &params);
    void TunnelProxyServerConfigurationRemoved(const QVariantMap &params);

    void BackupConfigurationChanged(const QVariantMap &params);

    void MqttServerConfigurationChanged(const QVariantMap &params);
    void MqttServerConfigurationRemoved(const QVariantMap &params);
    void MqttPolicyChanged(const QVariantMap &params);
    void MqttPolicyRemoved(const QVariantMap &params);

private slots:
    void onBasicConfigurationChanged();
    void onTcpServerConfigurationChanged(const QString &id);
    void onTcpServerConfigurationRemoved(const QString &id);
    void onWebServerConfigurationChanged(const QString &id);
    void onWebServerConfigurationRemoved(const QString &id);
    void onWebSocketServerConfigurationChanged(const QString &id);
    void onWebSocketServerConfigurationRemoved(const QString &id);
    void onTunnelProxyServerConfigurationChanged(const QString &id);
    void onTunnelProxyServerConfigurationRemoved(const QString &id);
    void onBackupConfigurationChanged();
    void onMqttServerConfigurationChanged(const QString &id);
    void onMqttServerConfigurationRemoved(const QString &id);
    void onMqttPolicyChanged(const QString &clientId);
    void onMqttPolicyRemoved(const QString &clientId);

private:
    static QVariantMap packBasicConfiguration();
    static QVariantMap packBackupConfiguration();
    QVariantMap statusToReply(NymeaConfiguration::ConfigurationError status) const;

};

}

#endif // CONFIGURATIONHANDLER_H
