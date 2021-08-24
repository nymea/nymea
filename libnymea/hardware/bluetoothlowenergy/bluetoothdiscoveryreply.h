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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef BLUETOOTHDISCOVERYREPLY_H
#define BLUETOOTHDISCOVERYREPLY_H

#ifdef WITH_BLUETOOTH

#include <QObject>
#include <QBluetoothDeviceInfo>

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
    virtual QList<QBluetoothDeviceInfo> discoveredDevices() const = 0;

signals:
    void finished();
    void errorOccurred(const BluetoothDiscoveryReplyError &error);

};

#endif // WITH_BLUETOOTH

#endif // BLUETOOTHDISCOVERYREPLY_H
