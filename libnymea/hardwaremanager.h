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

#ifndef HARDWAREMANAGER_H
#define HARDWAREMANAGER_H

#include <QObject>

class Radio433;
class UpnpDiscovery;
class PluginTimerManager;
class NetworkAccessManager;
class UpnpDeviceDescriptor;
class QtAvahiServiceBrowser;
class BluetoothLowEnergyManager;
class HardwareResource;

class HardwareManager : public QObject
{
    Q_OBJECT

public:
    HardwareManager(QObject *parent = nullptr);
    virtual ~HardwareManager() = default;

    virtual Radio433 *radio433() = 0;
    virtual PluginTimerManager *pluginTimerManager() = 0;
    virtual NetworkAccessManager *networkManager() = 0;
    virtual UpnpDiscovery *upnpDiscovery() = 0;
    virtual QtAvahiServiceBrowser *avahiBrowser() = 0;
    virtual BluetoothLowEnergyManager *bluetoothLowEnergyManager() = 0;

protected:
    void setResourceEnabled(HardwareResource* resource, bool enabled);
};

#endif // HARDWAREMANAGER_H
