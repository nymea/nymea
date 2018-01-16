/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017-2018 Simon Stürz <simon.stuerz@guh.io>              *
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

#ifndef BLUETOOTHDISCOVERYREPLY_H
#define BLUETOOTHDISCOVERYREPLY_H

#include <QObject>
#include <QBluetoothDeviceInfo>

#include "libguh.h"

class LIBGUH_EXPORT BluetoothDiscoveryReply : public QObject
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
    virtual QList<QBluetoothDeviceInfo> discoveredDevices() const = 0;

signals:
    void finished();
    void errorOccured(const BluetoothDiscoveryReplyError &error);

};

#endif // BLUETOOTHDISCOVERYREPLY_H
