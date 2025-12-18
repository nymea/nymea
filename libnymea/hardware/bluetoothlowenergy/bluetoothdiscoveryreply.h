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

#ifndef BLUETOOTHDISCOVERYREPLY_H
#define BLUETOOTHDISCOVERYREPLY_H

#include <QBluetoothDeviceInfo>
#include <QBluetoothHostInfo>
#include <QObject>

#include "libnymea.h"

class LIBNYMEA_EXPORT BluetoothDiscoveryReply : public QObject
{
    Q_OBJECT

public:
    enum BluetoothDiscoveryReplyError {
        BluetoothDiscoveryReplyErrorNoError,
        BluetoothDiscoveryReplyErrorNotAvailable,
        BluetoothDiscoveryReplyErrorNotEnabled,
        BluetoothDiscoveryReplyErrorBusy
    };
    Q_ENUM(BluetoothDiscoveryReplyError)

    explicit BluetoothDiscoveryReply(QObject *parent = nullptr);
    virtual ~BluetoothDiscoveryReply() = default;

    virtual bool isFinished() const = 0;
    virtual BluetoothDiscoveryReplyError error() const = 0;
    virtual QList<QPair<QBluetoothDeviceInfo, QBluetoothHostInfo>> discoveredDevices() const = 0;

signals:
    void finished();
    void errorOccurred(const BluetoothDiscoveryReplyError &error);
};

#endif // BLUETOOTHDISCOVERYREPLY_H
