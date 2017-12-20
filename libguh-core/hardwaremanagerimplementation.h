/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Simon Stürz <simon.stuerz@guh.io>                   *
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

// FIXME: use forward declaration for timeTick
#include "hardware/plugintimermanagerimplementation.h"

class Radio433;
class UpnpDiscovery;
class NetworkAccessManager;
class UpnpDeviceDescriptor;
class QtAvahiServiceBrowser;
class BluetoothLowEnergyManager;


namespace guhserver {

class HardwareManagerImplementation : public HardwareManager
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.guh.nymead")

public:
    explicit HardwareManagerImplementation(QObject *parent = nullptr);

    Radio433 *radio433();
    PluginTimerManager *pluginTimerManager();
    NetworkAccessManager *networkManager();
    UpnpDiscovery *upnpDiscovery();
    QtAvahiServiceBrowser *avahiBrowser();
    BluetoothLowEnergyManager *bluetoothLowEnergyManager();

    // D-Bus method for enable/disable bluetooth support
    Q_SCRIPTABLE void EnableBluetooth(const bool &enabled);

private:
    QNetworkAccessManager *m_networkAccessManager;

    // Hardware Resources
    PluginTimerManagerImplementation *m_pluginTimerManager = nullptr;
    Radio433 *m_radio433 = nullptr;
    NetworkAccessManager *m_networkManager = nullptr;
    UpnpDiscovery *m_upnpDiscovery = nullptr;
    QtAvahiServiceBrowser *m_avahiBrowser = nullptr;
    BluetoothLowEnergyManager *m_bluetoothLowEnergyManager = nullptr;

public slots:
    void timeTick();

};

}

#endif // HARDWAREMANAGERIMPLEMENTATION_H
