/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Alexander Lampret <alexander.lampret@gmail.com>     *
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

#ifndef DEVICEPLUGINPUSHBULLET_H
#define DEVICEPLUGINPUSHBULLET_H

#include "plugin/deviceplugin.h"
#include "devicemanager.h"

#include <QDebug>
#include <QNetworkInterface>
#include <QProcess>
#include <QUrlQuery>

class DevicePluginPushbullet: public DevicePlugin {
	Q_OBJECT
    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginpushbullet.json")
	Q_INTERFACES(DevicePlugin)

public:
    DevicePluginPushbullet();
    DeviceManager::HardwareResources requiredHardware() const override;
	DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void networkManagerReplyReady(QNetworkReply *reply) override;

public slots:
	DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

private:
    void sendNotification(Device* device, ParamList params);
};

#endif // DEVICEPLUGINPUSHBULLET_H
