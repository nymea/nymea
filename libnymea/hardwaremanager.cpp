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

/*!
    \class HardwareManager
    \brief The main entry point when interacting with \l{HardwareResource}{hardware resources}

    \inmodule libguh

    \sa HardwareResource
*/

/*! \fn HardwareManager::~HardwareManager();
    The virtual destructor of the HardwareManager.
*/

/*! \fn Radio433 * HardwareManager::radio433();
    Returns the Radio433 \l{HardwareResource}.
*/

/*! \fn PluginTimerManager * HardwareManager::pluginTimerManager();
    Returns the PluginTimerManager \l{HardwareResource}.
*/

/*! \fn NetworkAccessManager * HardwareManager::networkManager();
    Returns the NetworkAccessManager \l{HardwareResource}.
*/

/*! \fn UpnpDiscovery * HardwareManager::upnpDiscovery();
    Returns the UpnpDiscovery \l{HardwareResource}.
*/

/*! \fn QtAvahiServiceBrowser * HardwareManager::avahiBrowser();
    Returns the QtAvahiServiceBrowser \l{HardwareResource}.
*/

/*! \fn BluetoothLowEnergyManager * HardwareManager::bluetoothLowEnergyManager();
    Returns the BluetoothLowEnergyManager \l{HardwareResource}.
*/

/*! \fn MqttProvider* HardwareManager::mqttProvider();
    Returns the MqttProvider \l{HardwareResource}.
*/


#include "hardwaremanager.h"
#include "hardwareresource.h"

/*! Constructs a new HardwareManager with the given \a parent.*/
HardwareManager::HardwareManager(QObject *parent) :
    QObject(parent)
{
}

/*! Sets the given \a resource to \a enabled. This allows to enable/disable individual \l{HardwareResource}{HardwareResources}. */
void HardwareManager::setResourceEnabled(HardwareResource *resource, bool enabled)
{
    resource->setEnabled(enabled);
}
