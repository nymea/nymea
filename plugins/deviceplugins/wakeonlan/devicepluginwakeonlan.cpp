/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
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
    \brief Plugin for wake up devices in the LAN (wol).

    \ingroup plugins
    \ingroup guh-plugins

    Wake-on-LAN (WOL) is an Ethernet computer networking standard that allows a computer
    to be turned on or awakened by a network message. This plugin allows you to send a
    a "magic packet" to a certain mac address in the local network.

    \underline{NOTE}: The WOL service has to be enabled on the host computer.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

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

DeviceManager::HardwareResources DevicePluginWakeOnLan::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

DeviceManager::DeviceError DevicePluginWakeOnLan::executeAction(Device *device, const Action &action)
{
    if(action.actionTypeId() == wolActionTypeId){
        qCDebug(dcWakeOnLan) << "Wake up" << device->name();
        wakeup(device->paramValue(macParamTypeId).toString());
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
    qCDebug(dcWakeOnLan) << "Created magic packet:" << packet.toHex();
    QUdpSocket udpSocket;
    udpSocket.writeDatagram(packet.data(), packet.size(), QHostAddress::Broadcast, 9);
}

