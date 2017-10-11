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

#ifndef CONFIGURATIONHANDLER_H
#define CONFIGURATIONHANDLER_H

#include <QObject>

#include "jsonhandler.h"

namespace guhserver {

class ConfigurationHandler : public JsonHandler
{
    Q_OBJECT

public:
    ConfigurationHandler(QObject *parent = 0);
    QString name() const;

    Q_INVOKABLE JsonReply *GetConfigurations(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetTimeZones(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetAvailableLanguages(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetServerName(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetTimeZone(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetLanguage(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetTcpServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *DeleteTcpServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetWebServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *DeleteWebServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetWebSocketServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *DeleteWebSocketServerConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetCloudEnabled(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetCloudEnabled(const QVariantMap &params) const;

signals:
    void BasicConfigurationChanged(const QVariantMap &params);
    void TcpServerConfigurationChanged(const QVariantMap &params);
    void WebServerConfigurationChanged(const QVariantMap &params);
    void WebSocketServerConfigurationChanged(const QVariantMap &params);
    void LanguageChanged(const QVariantMap &params);

private slots:
    void onBasicConfigurationChanged();
    void onTcpServerConfigurationChanged(const QString &id);
    void onWebServerConfigurationChanged(const QString &id);
    void onWebSocketServerConfigurationChanged(const QString &id);
    void onLanguageChanged();

};

}

#endif // CONFIGURATIONHANDLER_H
