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

#ifndef DEVICEPLUGINAWATTAR_H
#define DEVICEPLUGINAWATTAR_H

#include "plugin/deviceplugin.h"

#include <QHash>
#include <QDebug>
#include <QTimer>

class DevicePluginAwattar : public DevicePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginawattar.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginAwattar();

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;
    void networkManagerReplyReady(QNetworkReply *reply) override;

private:
    QTimer *m_timer;
    QHash<QNetworkReply *, Device *> m_asyncSetup;
    QHash<QNetworkReply *, Device *> m_update;

    void processData(Device *device, const QVariantMap &data, const bool &fromSetup = false);

    QNetworkReply *requestData(const QString& token);
    void updateDevice(Device *device);

private slots:
    void onTimeout();

};

#endif // DEVICEPLUGINAWATTAR_H
