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
    Q_INVOKABLE JsonReply *SetDebugServerEnabled(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetCloudEnabled(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetTcpServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *DeleteTcpServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetWebServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *DeleteWebServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetWebSocketServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *DeleteWebSocketServerConfiguration(const QVariantMap &params) const;

    Q_INVOKABLE JsonReply *GetMqttServerConfigurations(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetMqttServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *DeleteMqttServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetMqttPolicies(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetMqttPolicy(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *DeleteMqttPolicy(const QVariantMap &params) const;

signals:
    void BasicConfigurationChanged(const QVariantMap &params);
    // TODO: remove, should be part of BasicConfigurationChanged
    void LanguageChanged(const QVariantMap &params);
    void CloudConfigurationChanged(const QVariantMap &params);
    void TcpServerConfigurationChanged(const QVariantMap &params);
    void TcpServerConfigurationRemoved(const QVariantMap &params);
    void WebServerConfigurationChanged(const QVariantMap &params);
    void WebServerConfigurationRemoved(const QVariantMap &params);
    void WebSocketServerConfigurationChanged(const QVariantMap &params);
    void WebSocketServerConfigurationRemoved(const QVariantMap &params);

    void MqttServerConfigurationChanged(const QVariantMap &params);
    void MqttServerConfigurationRemoved(const QVariantMap &params);
    void MqttPolicyChanged(const QVariantMap &params);
    void MqttPolicyRemoved(const QVariantMap &params);

private slots:
    void onBasicConfigurationChanged();
    void onLanguageChanged();
    void onCloudConfigurationChanged(bool enabled);
    void onTcpServerConfigurationChanged(const QString &id);
    void onTcpServerConfigurationRemoved(const QString &id);
    void onWebServerConfigurationChanged(const QString &id);
    void onWebServerConfigurationRemoved(const QString &id);
    void onWebSocketServerConfigurationChanged(const QString &id);
    void onWebSocketServerConfigurationRemoved(const QString &id);
    void onMqttServerConfigurationChanged(const QString &id);
    void onMqttServerConfigurationRemoved(const QString &id);
    void onMqttPolicyChanged(const QString &clientId);
    void onMqttPolicyRemoved(const QString &clientId);

private:
    static QVariantMap packBasicConfiguration();
    QVariantMap statusToReply(NymeaConfiguration::ConfigurationError status) const;

};

}

#endif // CONFIGURATIONHANDLER_H
