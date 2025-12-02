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

#include "bluetoothpairingjobimplementation.h"

#include "nymeabluetoothagent.h"

namespace nymeaserver
{

BluetoothPairingJobImplementation::BluetoothPairingJobImplementation(NymeaBluetoothAgent *agent, const QBluetoothAddress &address, QObject *parent)
    : BluetoothPairingJob{address, parent},
      m_agent{agent},
      m_address{address}
{
    connect(m_agent, &NymeaBluetoothAgent::passKeyRequested, this, [address, this](const QBluetoothAddress &addr){
        if (address != addr) {
            // Not for us...
            return;
        }
        emit passKeyRequested();
    });
    connect(m_agent, &NymeaBluetoothAgent::displayPinCode, this, [address, this](const QBluetoothAddress &addr, const QString &pinCode){
        if (address != addr) {
            // Not for us...
            return;
        }
        emit displayPinCode(pinCode);
    });

}

bool BluetoothPairingJobImplementation::isFinished() const
{
    return m_finished;
}

bool BluetoothPairingJobImplementation::success() const
{
    return m_success;
}

void BluetoothPairingJobImplementation::passKeyEntered(const QString passKey)
{
    m_agent->passKeyEntered(m_address, passKey);
}

void BluetoothPairingJobImplementation::finish(bool success)
{
    m_finished = true;
    m_success = success;
    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection, Q_ARG(bool, success));
}

}
