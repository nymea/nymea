/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#ifndef DEVICEPLUGINNETATMO_H
#define DEVICEPLUGINNETATMO_H

#include "plugin/deviceplugin.h"
#include "network/oauth2.h"
#include "netatmobasestation.h"
#include "netatmooutdoormodule.h"

#include <QHash>
#include <QTimer>

class DevicePluginNetatmo : public DevicePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginnetatmo.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginNetatmo();

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;
    void networkManagerReplyReady(QNetworkReply *reply) override;

    void guhTimer() override;

public slots:
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

private:
    QList<Device *> m_asyncSetups;

    QHash<OAuth2 *, Device *> m_authentications;
    QHash<NetatmoBaseStation *, Device *> m_indoorDevices;
    QHash<NetatmoOutdoorModule *, Device *> m_outdoorDevices;

    QHash<QNetworkReply *, Device *> m_refreshRequest;

    void refreshData(Device *device, const QString &token);
    void processRefreshData(const QVariantMap &data, const QString &connectionId);

    Device *findIndoorDevice(const QString &macAddress);
    Device *findOutdoorDevice(const QString &macAddress);

private slots:
    void onAuthenticationChanged();
    void onIndoorStatesChanged();
    void onOutdoorStatesChanged();

};

#endif // DEVICEPLUGINNETATMO_H
