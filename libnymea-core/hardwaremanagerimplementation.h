/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017-2018 Simon Stürz <simon.stuerz@guh.io>              *
 *                                                                         *
 *  This file is part of nymea.                                            *
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

#ifndef HARDWAREMANAGERIMPLEMENTATION_H
#define HARDWAREMANAGERIMPLEMENTATION_H

#include <QObject>
#include <QDBusConnection>
#include <QNetworkAccessManager>

#include "hardwaremanager.h"

class Radio433;
class UpnpDiscovery;
class PluginTimerManager;
class NetworkAccessManager;
class UpnpDeviceDescriptor;
class ZeroConfServiceBrowser;
class BluetoothLowEnergyManager;

namespace nymeaserver {

class Platform;
class MqttBroker;

class HardwareManagerImplementation : public HardwareManager
{
    Q_OBJECT

public:
    explicit HardwareManagerImplementation(Platform *platform, MqttBroker *mqttBroker, QObject *parent = nullptr);
    ~HardwareManagerImplementation() override;

    Radio433 *radio433() override;
    PluginTimerManager *pluginTimerManager() override;
    NetworkAccessManager *networkManager() override;
    UpnpDiscovery *upnpDiscovery() override;
    ZeroConfServiceBrowser *zeroConfServiceBrowser() override;
    BluetoothLowEnergyManager *bluetoothLowEnergyManager() override;
    MqttProvider *mqttProvider() override;

private:
    QNetworkAccessManager *m_networkAccessManager = nullptr;

    Platform *m_platform = nullptr;

    // Hardware Resources
    PluginTimerManager *m_pluginTimerManager = nullptr;
    Radio433 *m_radio433 = nullptr;
    NetworkAccessManager *m_networkManager = nullptr;
    UpnpDiscovery *m_upnpDiscovery = nullptr;
    BluetoothLowEnergyManager *m_bluetoothLowEnergyManager = nullptr;
    MqttProvider *m_mqttProvider = nullptr;
};

}

#endif // HARDWAREMANAGERIMPLEMENTATION_H
