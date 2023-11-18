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

#include "bluetoothdiscoveryreplyimplementation.h"

#include <QTimer>

namespace nymeaserver {

BluetoothDiscoveryReplyImplementation::BluetoothDiscoveryReplyImplementation(QObject *parent) :
    BluetoothDiscoveryReply(parent)
{

}

bool BluetoothDiscoveryReplyImplementation::isFinished() const
{
    return m_finished;
}

BluetoothDiscoveryReplyImplementation::BluetoothDiscoveryReplyError BluetoothDiscoveryReplyImplementation::error() const
{
    return m_error;
}

QList<QBluetoothDeviceInfo> BluetoothDiscoveryReplyImplementation::discoveredDevices() const
{
    return m_discoveredDevices;
}

void BluetoothDiscoveryReplyImplementation::setError(const BluetoothDiscoveryReplyImplementation::BluetoothDiscoveryReplyError &error)
{
    m_error = error;
    if (m_error != BluetoothDiscoveryReplyErrorNoError) {
        emit errorOccurred(m_error);
    }
}

void BluetoothDiscoveryReplyImplementation::setDiscoveredDevices(const QList<QBluetoothDeviceInfo> &discoveredDevices)
{
    m_discoveredDevices = discoveredDevices;
}

void BluetoothDiscoveryReplyImplementation::addDiscoveredDevice(const QBluetoothDeviceInfo &info)
{
    m_discoveredDevices.append(info);
}

void BluetoothDiscoveryReplyImplementation::setFinished()
{
    m_finished = true;
    // Note: this makes sure the finished signal will be processed in the next event loop
    QTimer::singleShot(0, this, &BluetoothDiscoveryReplyImplementation::finished);
    deleteLater();
}

}
