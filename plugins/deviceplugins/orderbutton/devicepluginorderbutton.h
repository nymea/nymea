/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2015 Bernhard Trinnes <bernhard.trinnes@guh.guru>        *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICEPLUGINORDERBUTTON_H
#define DEVICEPLUGINORDERBUTTON_H

#include "plugin/deviceplugin.h"
#include "types/action.h"
#include "coap/coap.h"

#include <QHash>

class DevicePluginOrderButton : public DevicePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginorderbutton.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginOrderButton();

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

    QHash<CoapReply *, Action> m_resetCounterRequests;
    QHash<CoapReply *, Action> m_setCount;
    QHash<CoapReply *, Action> m_setLedPower;

    void pingDevice(Device *device);

    void updateBattery(Device *device);
    void updateCount(Device *device);
    void updateButton(Device *device);
    void updateLed(Device *device);

    void enableNotifications(Device *device);

    void setReachable(Device *device, const bool &reachable);

    bool deviceAlreadyAdded(const QHostAddress &address);
    Device *findDevice(const QHostAddress &address);

private slots:
    void coapReplyFinished(CoapReply *reply);
    void onNotificationReceived(const CoapObserveResource &resource, const int &notificationNumber, const QByteArray &payload);
};

#endif // DEVICEPLUGINORDERBUTTON_H
