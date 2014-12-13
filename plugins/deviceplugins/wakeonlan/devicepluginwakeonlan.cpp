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
    \ingroup services

    Wake-on-LAN (WOL) is an Ethernet computer networking standard that allows a computer
    to be turned on or awakened by a network message. This plugin allows you to send a
    a "magic packet" to a certain mac address in the local network. The WOL service must
    be enabled on the host computer.

    \section1 Examples
    \section2 Adding a WOL device
    In order to add a WOL device you need to know the mac address of the host computer you
    want to wake up.
    \code
    {
        "id":1,
        "method":"Devices.AddConfiguredDevice",
        "params":{
            "deviceClassId": "{3c8f2447-dcd0-4882-8c09-99e579e4d24c}",
            "deviceParams":{
                "mac":"00:11:22:33:44:55",
                "name":"Wohnzimmer-PC"
            }
        }
    }
    \endcode
    response...
    \code
    {
        "id": 1,
        "params": {
            "deviceId": "{76347a44-2091-428c-b320-5a8db4c359f6}",
            "errorMessage": "",
            "success": true
        },
        "status": "success"
    }
    \endcode

    \section2 Wake up a device
    In order to wake up a configured device send following message:
    \code
    {
        "id":1,
        "method":"Actions.ExecuteAction",
        "params":{
            "actionTypeId": "{fb9b9d87-218f-4f0d-9e16-39f8a105029a}",
            "deviceId":"{76347a44-2091-428c-b320-5a8db4c359f6}"
        }
    }
    \endcode
    response...
    \code
    {
        "id": 1,
        "params": {
            "errorMessage": "",
            "success": true
        },
        "status": "success"
    }
    \endcode
    \section1 Plugin propertys:
        \section2 Plugin parameters
        Each configured plugin has following paramters:

        \table
            \header
                \li Name
                \li Description
                \li Data Type
            \row
                \li name
                \li This parameter holds the name/description of the target computer
                \li string
            \row
                \li mac
                \li This parameter holds the mac address of the target computer
                \li string
        \endtable
        \section2 Plugin actions:
        Following list contains all plugin \l{Action}s:
            \table
            \header
                \li Name
                \li Description
                \li UUID
            \row
                \li wakeup
                \li This action send the "magic package" to the mac address configured in the device parameter.
                \li fb9b9d87-218f-4f0d-9e16-39f8a105029a
            \endtable

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

