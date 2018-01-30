/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017-2018 Simon Stürz <simon.stuerz@guh.io>              *
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
class QtAvahiServiceBrowser;
class BluetoothLowEnergyManager;


namespace nymeaserver {

class HardwareManagerImplementation : public HardwareManager
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.guh.nymead")

public:
    explicit HardwareManagerImplementation(QObject *parent = nullptr);
    ~HardwareManagerImplementation();

    Radio433 *radio433() override;
    PluginTimerManager *pluginTimerManager() override;
    NetworkAccessManager *networkManager() override;
    UpnpDiscovery *upnpDiscovery() override;
    QtAvahiServiceBrowser *avahiBrowser() override;
    BluetoothLowEnergyManager *bluetoothLowEnergyManager() override;

     // D-Bus method for enable/disable bluetooth support
    Q_SCRIPTABLE void EnableBluetooth(const bool &enabled);

private:
    QNetworkAccessManager *m_networkAccessManager = nullptr;

    // Hardware Resources
    PluginTimerManager *m_pluginTimerManager = nullptr;
    Radio433 *m_radio433 = nullptr;
    NetworkAccessManager *m_networkManager = nullptr;
    UpnpDiscovery *m_upnpDiscovery = nullptr;
    QtAvahiServiceBrowser *m_avahiBrowser = nullptr;
    BluetoothLowEnergyManager *m_bluetoothLowEnergyManager = nullptr;
};

}

#endif // HARDWAREMANAGERIMPLEMENTATION_H
