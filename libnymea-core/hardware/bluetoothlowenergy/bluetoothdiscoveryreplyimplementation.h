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

#ifndef BLUETOOTHDISCOVERYREPLYIMPLEMENTATION_H
#define BLUETOOTHDISCOVERYREPLYIMPLEMENTATION_H

#include <QObject>
#include <QBluetoothDeviceInfo>

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
    QList<QBluetoothDeviceInfo> discoveredDevices() const override;

private:
    bool m_finished = false;
    BluetoothDiscoveryReplyError m_error = BluetoothDiscoveryReplyErrorNoError;
    QList<QBluetoothDeviceInfo> m_discoveredDevices;

    void setError(const BluetoothDiscoveryReplyError &error);
    void setDiscoveredDevices(const QList<QBluetoothDeviceInfo> &discoveredDevices);
    void addDiscoveredDevice(const QBluetoothDeviceInfo &info);
    void setFinished();
};

}

#endif // BLUETOOTHDISCOVERYREPLYIMPLEMENTATION_H
