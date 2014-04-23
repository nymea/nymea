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

#include "devicepluginwakeonlan.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include <QDebug>
#include <QStringList>
#include <QUdpSocket>

extern VendorId guhVendorId;
DeviceClassId wolDeviceClassId = DeviceClassId("3c8f2447-dcd0-4882-8c09-99e579e4d24c");
ActionTypeId wolActionTypeId = ActionTypeId("fb9b9d87-218f-4f0d-9e16-39f8a105029a");

DevicePluginWakeOnLan::DevicePluginWakeOnLan()
{
}

QList<Vendor> DevicePluginWakeOnLan::supportedVendors() const
{
    QList<Vendor> ret;
    Vendor guh(guhVendorId, "guh");
    ret.append(guh);
    return ret;
}

QList<DeviceClass> DevicePluginWakeOnLan::supportedDevices() const
{
    QList<DeviceClass> ret;

    DeviceClass deviceClassWakeOnLan(pluginId(), guhVendorId, wolDeviceClassId);
    deviceClassWakeOnLan.setName("Wake On Lan");
    
    QList<ParamType> wolParams;
    ParamType wolParam("mac", QVariant::String);
    wolParams.append(wolParam);


    QList<ActionType> wolActions;
    ActionType wolAction(wolActionTypeId);
    wolAction.setName("wakeup");
    wolActions.append(wolAction);

    deviceClassWakeOnLan.setParams(wolParams);
    deviceClassWakeOnLan.setActions(wolActions);

    ret.append(deviceClassWakeOnLan);
    return ret;
}

DeviceManager::HardwareResources DevicePluginWakeOnLan::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

QString DevicePluginWakeOnLan::pluginName() const
{
    return "Wake On Lan";
}

PluginId DevicePluginWakeOnLan::pluginId() const
{
    return PluginId("b5a87848-de56-451e-84a6-edd26ad4958f");
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

