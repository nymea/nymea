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

    TODO: description


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
#include "loggingcategories.h"

DevicePluginKodi::DevicePluginKodi()
{

}

DeviceManager::HardwareResources DevicePluginKodi::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer | DeviceManager::HardwareResourceUpnpDisovery;
}

DeviceManager::DeviceSetupStatus DevicePluginKodi::setupDevice(Device *device)
{
    KodiConnection *kodiConnection = new KodiConnection(QHostAddress(device->paramValue("ip").toString()), 9090, this);

    connect(kodiConnection, &KodiConnection::connectionStateChanged, this, &DevicePluginKodi::onConnectionChanged);

    kodiConnection->connectToKodi();

    m_kodiConnections.insert(kodiConnection, device);
    return DeviceManager::DeviceSetupStatusSuccess;
}

void DevicePluginKodi::deviceRemoved(Device *device)
{
    KodiConnection *kodiConnection = m_kodiConnections.key(device);
    m_kodiConnections.remove(kodiConnection);
    qCDebug(dcKodi) << "delete Kodi" << device->paramValue("name");
    kodiConnection->deleteLater();
}

void DevicePluginKodi::guhTimer()
{
    foreach (KodiConnection *kodi, m_kodiConnections.keys()) {
        if (!kodi->connected()) {
            kodi->connectToKodi();
            continue;
        } else {
            // update.. ?
        }
    }
}


DeviceManager::DeviceError DevicePluginKodi::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params)
    Q_UNUSED(deviceClassId)
    qCDebug(dcKodi) << "start Kodi UPnP search";
    upnpDiscover();
    return DeviceManager::DeviceErrorAsync;
}

void DevicePluginKodi::upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList)
{
    QList<DeviceDescriptor> deviceDescriptors;
    foreach (const UpnpDeviceDescriptor &upnpDescriptor, upnpDeviceDescriptorList) {
        if (upnpDescriptor.modelName().contains("Kodi")) {

            // check if we allready found the kodi on this ip
            bool alreadyAdded = false;
            foreach (const DeviceDescriptor dDescriptor, deviceDescriptors) {
                if (dDescriptor.params().paramValue("ip").toString() == upnpDescriptor.hostAddress().toString()) {
                    alreadyAdded = true;
                    break;
                }
            }
            if (alreadyAdded)
                continue;

            qCDebug(dcKodi) << upnpDescriptor;
            DeviceDescriptor deviceDescriptor(kodiDeviceClassId, "Kodi - Media Center", upnpDescriptor.hostAddress().toString());
            ParamList params;
            params.append(Param("name", upnpDescriptor.friendlyName()));
            params.append(Param("ip", upnpDescriptor.hostAddress().toString()));
            params.append(Param("port", 9090));
            deviceDescriptor.setParams(params);
            deviceDescriptors.append(deviceDescriptor);
        }
    }
    emit devicesDiscovered(kodiDeviceClassId, deviceDescriptors);
}

DeviceManager::DeviceError DevicePluginKodi::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() == kodiDeviceClassId) {
        if (action.actionTypeId() == sendNotificationActionTypeId) {


            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == volumeActionTypeId) {


            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

void DevicePluginKodi::onConnectionChanged(const bool &connected)
{
    KodiConnection *kodiConnection = static_cast<KodiConnection *>(sender());
    Device *device = m_kodiConnections.value(kodiConnection);

    device->setStateValue(connectedStateTypeId, connected);
}

void DevicePluginKodi::dataReceived(const QByteArray &data)
{
    KodiConnection *kodiConnection = static_cast<KodiConnection *>(sender());
    emit dataReady(kodiConnection, data);
}

