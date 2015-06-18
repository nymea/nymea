/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#ifndef DEVICEPLUGINPHILIPSHUE_H
#define DEVICEPLUGINPHILIPSHUE_H

#include "plugin/deviceplugin.h"
#include "huebridge.h"
#include "huelight.h"

class QNetworkReply;

class DevicePluginPhilipsHue: public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginphilipshue.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginPhilipsHue();

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    void deviceRemoved(Device *device) override;
    void upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList) override;

    DeviceManager::DeviceSetupStatus confirmPairing(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params) override;

    void networkManagerReplyReady(QNetworkReply *reply) override;

    void guhTimer() override;

public slots:
    DeviceManager::DeviceError executeAction(Device *device, const Action &action);

private slots:
    void lightStateChanged();

private:

    class PairingInfo {
    public:
        PairingTransactionId pairingTransactionId;
        QHostAddress host;
        QString apiKey;
    };

    QHash<QNetworkReply *, PairingInfo> m_pairingRequests;
    QHash<QNetworkReply *, PairingInfo> m_informationRequests;

    QList<HueBridge *> m_unconfiguredBridges;
    QList<HueLight *> m_unconfiguredLights;

    QHash<QNetworkReply *, Device *> m_lightRefreshRequests;
    QHash<QNetworkReply *, Device *> m_lightSetNameRequests;
    QHash<QNetworkReply *, Device *> m_bridgeRefreshRequests;
    QHash<QNetworkReply *, QPair<Device *, ActionId> > m_asyncActions;

    QHash<HueBridge*, Device*> m_bridges;
    QHash<HueLight*, Device*> m_lights;

    void refreshLight(Device *device);
    void refreshBridge(Device *device);

    void setLightName(Device *device, QString name);

    void processLightRefreshResponse(Device *device, const QByteArray &data);
    void processBridgeRefreshResponse(Device *device, const QByteArray &data);
    void processSetNameResponse(Device *device, const QByteArray &data);
    void processPairingResponse(const PairingInfo &pairingInfo, const QByteArray &data);
    void processInformationResponse(const PairingInfo &pairingInfo, const QByteArray &data);
    void processActionResponse(Device *device, const ActionId actionId, const QByteArray &data);

    void onBridgeError(Device *device);

    int brightnessToPercentage(int brightness);
    int percentageToBrightness(int percentage);
};

#endif // DEVICEPLUGINBOBLIGHT_H
