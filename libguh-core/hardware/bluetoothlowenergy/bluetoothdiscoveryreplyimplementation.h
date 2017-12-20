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

#ifndef BLUETOOTHDISCOVERYREPLYIMPLEMENTATION_H
#define BLUETOOTHDISCOVERYREPLYIMPLEMENTATION_H

#include <QObject>
#include <QBluetoothDeviceInfo>

#include "hardware/bluetoothlowenergy/bluetoothdiscoveryreply.h"

namespace guhserver {

class BluetoothDiscoveryReplyImplementation : public BluetoothDiscoveryReply
{
    Q_OBJECT

    friend class BluetoothLowEnergyManagerImplementation;

public:
    explicit BluetoothDiscoveryReplyImplementation(QObject *parent = nullptr);

    bool isFinished() const;
    BluetoothDiscoveryReplyError error() const;
    QList<QBluetoothDeviceInfo> discoveredDevices() const;

private:
    bool m_finished = false;
    BluetoothDiscoveryReplyError m_error = BluetoothDiscoveryReplyErrorNoError;
    QList<QBluetoothDeviceInfo> m_discoveredDevices;

    void setError(const BluetoothDiscoveryReplyError &error);
    void setDiscoveredDevices(const QList<QBluetoothDeviceInfo> &discoveredDevices);
    void setFinished();
};

}

#endif // BLUETOOTHDISCOVERYREPLYIMPLEMENTATION_H
