/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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
    \page wakeonlan.html
    \title Wake On Lan

    \ingroup plugins
    \ingroup network

    Wake-on-LAN (WOL) is an Ethernet computer networking standard that allows a computer
    to be turned on or awakened by a network message. This plugin allows you to send a
    a "magic packet" to a certain mac address in the local network.

    \underline{NOTE}: The WOL service has to be enabled on the host computer.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \quotefile plugins/deviceplugins/wakeonlan/devicepluginwakeonlan.json
*/


#include "devicepluginwakeonlan.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>
#include <QStringList>
#include <QUdpSocket>

DevicePluginWakeOnLan::DevicePluginWakeOnLan()
{
}

//QList<DeviceClass> DevicePluginWakeOnLan::supportedDevices() const
//{
//    QList<DeviceClass> ret;

//    DeviceClass deviceClassWakeOnLan(pluginId(), supportedVendors().first().id(), wolDeviceClassId);
//    deviceClassWakeOnLan.setName("Wake On Lan");
    
//    QList<ParamType> wolParams;
//    ParamType nameParam("name", QVariant::String);
//    wolParams.append(nameParam);
//    ParamType wolParam("mac", QVariant::String);
//    wolParams.append(wolParam);


//    QList<ActionType> wolActions;
//    ActionType wolAction(wolActionTypeId);
//    wolAction.setName("wakeup");
//    wolActions.append(wolAction);

//    deviceClassWakeOnLan.setParamTypes(wolParams);
//    deviceClassWakeOnLan.setActions(wolActions);

//    ret.append(deviceClassWakeOnLan);
//    return ret;
//}

DeviceManager::HardwareResources DevicePluginWakeOnLan::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

DeviceManager::DeviceError DevicePluginWakeOnLan::executeAction(Device *device, const Action &action)
{
    qDebug() << "execute action " << action.actionTypeId().toString();
    if(action.actionTypeId() == wolActionTypeId){
        wakeup(device->paramValue("mac").toString());
    }
    return DeviceManager::DeviceErrorNoError;
}

void DevicePluginWakeOnLan::wakeup(QString mac)
{
    const char header[] = {char(0xff), char(0xff), char(0xff), char(0xff), char(0xff), char(0xff)};
    QByteArray packet = QByteArray::fromRawData(header, sizeof(header));
    for(int i = 0; i < 16; ++i) {
        packet.append(QByteArray::fromHex(mac.remove(':').toLocal8Bit()));
    }
    qDebug() << "created magic packet:" << packet.toHex();
    QUdpSocket udpSocket;
    udpSocket.writeDatagram(packet.data(), packet.size(), QHostAddress::Broadcast, 9);

}

