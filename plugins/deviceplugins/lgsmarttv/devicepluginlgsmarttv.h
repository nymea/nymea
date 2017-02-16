/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef DEVICEPLUGINLGSMARTTV_H
#define DEVICEPLUGINLGSMARTTV_H

#include "plugin/deviceplugin.h"
#include "tvdevice.h"
#include "network/upnp/upnpdevicedescriptor.h"

class DevicePluginLgSmartTv : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginlgsmarttv.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginLgSmartTv();

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;

    void upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList) override;

    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;
    DeviceManager::DeviceError displayPin(const PairingTransactionId &pairingTransactionId, const DeviceDescriptor &deviceDescriptor) override;
    DeviceManager::DeviceSetupStatus confirmPairing(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params, const QString &secret) override;

    void networkManagerReplyReady(QNetworkReply *reply) override;

    void guhTimer() override;

private:
    QHash<TvDevice *, Device *> m_tvList;
    QHash<QString, QString> m_tvKeys;

    // first pairing setup
    QHash<QNetworkReply *, PairingTransactionId> m_setupPairingTv;
    QHash<QNetworkReply *, PairingTransactionId> m_setupEndPairingTv;
    QList<QNetworkReply *> m_showPinReply;

    // async setup
    QHash<QNetworkReply *, Device *> m_asyncSetup;
    QHash<QNetworkReply *, Device *> m_pairRequests;
    QList<QNetworkReply *> m_deleteTv;

    // action requests
    QHash<QNetworkReply *, ActionId> m_asyncActions;

    // update requests
    QHash<QNetworkReply *, Device *> m_volumeInfoRequests;
    QHash<QNetworkReply *, Device *> m_channelInfoRequests;

    void pairTvDevice(Device *device, const bool &setup = false);
    void unpairTvDevice(Device *device);
    void refreshTv(Device *device);

private slots:
    void stateChanged();
};

#endif // DEVICEPLUGINLGSMARTTV_H
