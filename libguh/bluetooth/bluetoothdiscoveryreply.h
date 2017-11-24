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

#ifndef BLUETOOTHDISCOVERYREPLY_H
#define BLUETOOTHDISCOVERYREPLY_H

#include <QObject>
#include <QBluetoothDeviceInfo>

class BluetoothDiscoveryReply : public QObject
{
    Q_OBJECT

    friend class BluetoothLowEnergyManager;

public:
    enum BluetoothDiscoveryReplyError {
        BluetoothDiscoveryReplyErrorNoError,
        BluetoothDiscoveryReplyErrorNotAvailable,
        BluetoothDiscoveryReplyErrorNotEnabled,
        BluetoothDiscoveryReplyErrorBusy
    };
    Q_ENUM(BluetoothDiscoveryReplyError)

    bool isFinished() const;
    BluetoothDiscoveryReplyError error() const;
    QList<QBluetoothDeviceInfo> discoveredDevices() const;

private:
    explicit BluetoothDiscoveryReply(QObject *parent = nullptr);

    bool m_finished = false;
    BluetoothDiscoveryReplyError m_error = BluetoothDiscoveryReplyErrorNoError;
    QList<QBluetoothDeviceInfo> m_discoveredDevices;

    void setError(const BluetoothDiscoveryReplyError &error);
    void setDiscoveredDevices(const QList<QBluetoothDeviceInfo> &discoveredDevices);
    void setFinished();

signals:
    void finished();
    void errorOccured(const BluetoothDiscoveryReplyError &error);

};

#endif // BLUETOOTHDISCOVERYREPLY_H
