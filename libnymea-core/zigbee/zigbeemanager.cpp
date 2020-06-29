/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "zigbeemanager.h"
#include "nymeasettings.h"
#include "loggingcategories.h"

#include <QSerialPortInfo>

namespace nymeaserver {

ZigbeeManager::ZigbeeManager(QObject *parent) : QObject(parent)
{

}

bool ZigbeeManager::available() const
{
    return m_zigbeeNetwork != nullptr;
}

bool ZigbeeManager::enabled() const
{
    return m_zigbeeNetwork && m_zigbeeNetwork->state() != ZigbeeNetwork::StateUninitialized;
}

ZigbeeNetwork *ZigbeeManager::zigbeeNetwork() const
{
    return m_zigbeeNetwork;
}

ZigbeeSerialPortList ZigbeeManager::availablePorts()
{
    ZigbeeSerialPortList ports;
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        qCDebug(dcZigbee()) << "Found serial port" << serialPortInfo.portName();
        qCDebug(dcZigbee()) << "   Description:" << serialPortInfo.description();
        qCDebug(dcZigbee()) << "   System location:" << serialPortInfo.systemLocation();
        qCDebug(dcZigbee()) << "   Manufacturer:" << serialPortInfo.manufacturer();
        qCDebug(dcZigbee()) << "   Serialnumber:" << serialPortInfo.serialNumber();

        if (serialPortInfo.hasProductIdentifier()) {
            qCDebug(dcZigbee()) << "   Product identifier:" << serialPortInfo.productIdentifier();
        }
        if (serialPortInfo.hasVendorIdentifier()) {
            qCDebug(dcZigbee()) << "   Vendor identifier:" << serialPortInfo.vendorIdentifier();
        }

        ZigbeeSerialPort port;
        port.setName(serialPortInfo.portName());
        port.setDescription(serialPortInfo.description());
        port.setSystemLocation(serialPortInfo.systemLocation());
        ports.append(port);
    }

    return ports;
}

void ZigbeeManager::createZigbeeNetwork(const QString &serialPort, qint32 baudrate, ZigbeeNetworkManager::BackendType backend)
{
    if (m_zigbeeNetwork) {
        delete m_zigbeeNetwork;
        m_zigbeeNetwork = nullptr;
    }

    m_zigbeeNetwork = ZigbeeNetworkManager::createZigbeeNetwork(backend, this);
    m_zigbeeNetwork->setSerialPortName(serialPort);
    m_zigbeeNetwork->setSerialBaudrate(baudrate);
    m_zigbeeNetwork->setSettingsFileName(NymeaSettings(NymeaSettings::SettingsRoleGlobal).fileName());

    m_zigbeeNetwork->startNetwork();

    emit zigbeeNetworkChanged(m_zigbeeNetwork);
}

}
