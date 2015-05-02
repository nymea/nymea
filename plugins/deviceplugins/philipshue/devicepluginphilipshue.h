/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#ifndef DEVICEPLUGINPHILIPSHUE_H
#define DEVICEPLUGINPHILIPSHUE_H

#include "plugin/deviceplugin.h"
#include "discovery.h"
#include "huebridgeconnection.h"
#include "light.h"

class QNetworkReply;

class DevicePluginPhilipsHue: public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginphilipshue.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginPhilipsHue();

    DeviceManager::HardwareResources requiredHardware() const override;

    void startMonitoringAutoDevices() override;

    QList<ParamType> configurationDescription() const override;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;

    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;

    DeviceManager::DeviceSetupStatus confirmPairing(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params) override;

    void guhTimer() override;

public slots:
    DeviceManager::DeviceError executeAction(Device *device, const Action &action);

private slots:
    void discoveryDone(const QList<QHostAddress> &bridges);

    void createUserFinished(int id, const QVariant &params);
    void getLightsFinished(int id, const QVariant &params);
    void getFinished(int id, const QVariant &params);

    void lightStateChanged();

private:
    Discovery *m_discovery;

    class PairingInfo {
    public:
        PairingTransactionId pairingTransactionId;
        Param ipParam;
        Param usernameParam;
    };

    QHash<int, PairingInfo> m_pairings;
    HueBridgeConnection *m_bridge;

    QList<Light*> m_unconfiguredLights;
    QHash<Light*, Device*> m_lights;

    QHash<Light*, Device*> m_asyncSetups;
};

#endif // DEVICEPLUGINBOBLIGHT_H
