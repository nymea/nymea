/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2016 Bernhard Trinnes <bernhard.trinnes@guh.guru>        *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICEPLUGINDENON_H
#define DEVICEPLUGINDENON_H

#include "devicemanager.h"
#include "plugin/deviceplugin.h"


#include <QHash>
#include <QObject>
#include <QPointer>
#include <QHostAddress>
#include <QNetworkReply>

#include "denonconnection.h"

class DevicePluginDenon : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "deviceplugindenon.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginDenon();

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;

    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;
    void guhTimer() override;

private:
    QPointer<Device> m_device;
    QPointer<DenonConnection> m_denonConnection;
    QList<DenonConnection *> m_asyncSetups;

    QHash <ActionId, Device *> m_asyncActions;
    QHash <QNetworkReply *, ActionId> m_asyncActionReplies;

private slots:
    void onConnectionChanged();
    void onDataReceived(const QByteArray &data);
    void onSocketError();
};

#endif // DEVICEPLUGINDENON_H
