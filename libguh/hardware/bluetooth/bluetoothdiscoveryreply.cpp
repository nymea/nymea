/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
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

#include "bluetoothdiscoveryreply.h"

#include <QTimer>

bool BluetoothDiscoveryReply::isFinished() const
{
    return m_finished;
}

BluetoothDiscoveryReply::BluetoothDiscoveryReplyError BluetoothDiscoveryReply::error() const
{
    return m_error;
}

QList<QBluetoothDeviceInfo> BluetoothDiscoveryReply::discoveredDevices() const
{
    return m_discoveredDevices;
}

BluetoothDiscoveryReply::BluetoothDiscoveryReply(QObject *parent) : QObject(parent)
{

}

void BluetoothDiscoveryReply::setError(const BluetoothDiscoveryReply::BluetoothDiscoveryReplyError &error)
{
    m_error = error;
    if (m_error != BluetoothDiscoveryReplyErrorNoError) {
        emit errorOccured(m_error);
    }
}

void BluetoothDiscoveryReply::setDiscoveredDevices(const QList<QBluetoothDeviceInfo> &discoveredDevices)
{
    m_discoveredDevices = discoveredDevices;
}

void BluetoothDiscoveryReply::setFinished()
{
    m_finished = true;
    // Note: this makes sure the finished signal will be processed in the next event loop
    QTimer::singleShot(0, this, &BluetoothDiscoveryReply::finished);
}
