/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2015 Bernhard Trinnes <bernhard.trinnes@guh.guru>        *
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

#ifndef DEVICEPLUGINWS2812_H
#define DEVICEPLUGINWS2812_H

#include "plugin/deviceplugin.h"
#include "types/action.h"
#include "coap/coap.h"

#include <QColor>
#include <QHash>

class DevicePluginWs2812 : public DevicePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginws2812.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginWs2812();

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;
    void networkManagerReplyReady(QNetworkReply *reply) override;

    void postSetupDevice(Device *device) override;

    void guhTimer() override;
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

private:

    QPointer<Coap> m_coap;
    QHash<QNetworkReply *, DeviceClassId> m_asyncNodeScans;
    QHash<CoapReply *, Device *> m_enableNotification;
    QHash<CoapReply *, Device *> m_pingReplies;

    // State updates
    QHash<CoapReply *, Device *> m_updateReplies;

    // Actions
    QHash<ActionId, Device *> m_asyncActions;

    QHash<CoapReply *, Action> m_setColor;
    QHash<CoapReply *, Action> m_setEffect;
    QHash<CoapReply *, Action> m_setPix;
    QHash<CoapReply *, Action> m_setBrightness;
    QHash<CoapReply *, Action> m_setSpeed;
    QHash<CoapReply *, Action> m_setTColor;

    void pingDevice(Device *device);

    void updateBattery(Device *device);
    void updateColor(Device *device);
    void updateEffect(Device *device);
    void updateMaxPix(Device *device);
    void updateTricolore(Device *device);

    void enableNotifications(Device *device);

    void setReachable(Device *device, const bool &reachable);

    bool deviceAlreadyAdded(const QHostAddress &address);
    Device *findDevice(const QHostAddress &address);

    QColor tColor1;
    QColor tColor2;
    QColor tColor3;

private slots:
    void coapReplyFinished(CoapReply *reply);
    void onNotificationReceived(const CoapObserveResource &resource, const int &notificationNumber, const QByteArray &payload);
};

#endif // DEVICEPLUGINWS2812_H
