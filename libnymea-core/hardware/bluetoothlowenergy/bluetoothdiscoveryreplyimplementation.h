// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef BLUETOOTHDISCOVERYREPLYIMPLEMENTATION_H
#define BLUETOOTHDISCOVERYREPLYIMPLEMENTATION_H

#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>
#include <QObject>

#include "hardware/bluetoothlowenergy/bluetoothdiscoveryreply.h"

namespace nymeaserver {

class BluetoothDiscoveryReplyImplementation : public BluetoothDiscoveryReply
{
    Q_OBJECT

    friend class BluetoothLowEnergyManagerImplementation;

public:
    explicit BluetoothDiscoveryReplyImplementation(QObject *parent = nullptr);

    bool isFinished() const override;
    BluetoothDiscoveryReplyError error() const override;
    QList<QPair<QBluetoothDeviceInfo, QBluetoothHostInfo>> discoveredDevices() const override;

private:
    bool m_finished = false;
    BluetoothDiscoveryReplyError m_error = BluetoothDiscoveryReplyErrorNoError;
    QList<QPair<QBluetoothDeviceInfo, QBluetoothHostInfo>> m_discoveredDevices;

    void setError(const BluetoothDiscoveryReplyError &error);
    void setDiscoveredDevices(const QList<QPair<QBluetoothDeviceInfo, QBluetoothHostInfo>> &discoveredDevices);
    void addDiscoveredDevice(const QBluetoothDeviceInfo &info, const QBluetoothHostInfo &hostInfo);
    void setFinished();
};

} // namespace nymeaserver

#endif // BLUETOOTHDISCOVERYREPLYIMPLEMENTATION_H
