/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
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

/*!
    \page kodi.html
    \title Kodi

    \ingroup plugins
    \ingroup network


    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \note If a \l{StateType} has the parameter \tt{"writable": true}, an \l{ActionType} with the same uuid and \l{ParamType}{ParamTypes}
    will be created automatically.

    \quotefile plugins/deviceplugins/udpcommander/devicepluginudpcommander.json
*/

#include "devicepluginkodi.h"
#include "plugin/device.h"
#include "plugininfo.h"

DevicePluginKodi::DevicePluginKodi()
{

}

DeviceManager::HardwareResources DevicePluginKodi::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

DeviceManager::DeviceSetupStatus DevicePluginKodi::setupDevice(Device *device)
{
    Q_UNUSED(device)

    return DeviceManager::DeviceSetupStatusSuccess;
}

void DevicePluginKodi::deviceRemoved(Device *device)
{
    Q_UNUSED(device)
}

DeviceManager::DeviceSetupStatus DevicePluginKodi::confirmPairing(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(pairingTransactionId)
    Q_UNUSED(deviceClassId)
    Q_UNUSED(params)

    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::DeviceError DevicePluginKodi::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params)
    Q_UNUSED(deviceClassId)

    upnpDiscover();

    return DeviceManager::DeviceErrorNoError;
}

void DevicePluginKodi::upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList)
{
    Q_UNUSED(upnpDeviceDescriptorList)

}

void DevicePluginKodi::networkManagerReplyReady(QNetworkReply *reply)
{
    Q_UNUSED(reply)
}

void DevicePluginKodi::guhTimer()
{

}
