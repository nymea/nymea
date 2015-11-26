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

#ifndef DEVICEPLUGINOSDOMOTICS_H
#define DEVICEPLUGINOSDOMOTICS_H

#include "plugin/deviceplugin.h"

#include <QHash>
#include <QDebug>

#include "coap/coap.h"

class DevicePluginOsdomotics : public DevicePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginosdomotics.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginOsdomotics();

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;
    void networkManagerReplyReady(QNetworkReply *reply) override;

    void postSetupDevice(Device *device) override;

    void guhTimer() override;
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

private:
    Coap *m_coap;
    QHash<QNetworkReply *, Device *> m_asyncSetup;
    QHash<QNetworkReply *, Device *> m_asyncNodeRescans;

    QHash<CoapReply *, Device *> m_discoveryRequests;
    QHash<CoapReply *, Device *> m_updateRequests;
    QHash<CoapReply *, Action> m_toggleLightRequests;

    void scanNodes(Device *device);
    void parseNodes(Device *device, const QByteArray &data);
    void updateNode(Device *device);

    Device *findDevice(const QHostAddress &address);

private slots:
    void coapReplyFinished(CoapReply *reply);

};

#endif // DEVICEPLUGINOSDOMOTICS_H
