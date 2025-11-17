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

#include "zwaveadapter.h"

ZWaveAdapter::ZWaveAdapter()
{

}

QString ZWaveAdapter::serialPort() const
{
    return m_serialPort;
}

void ZWaveAdapter::setSerialPort(const QString &serialPort)
{
    m_serialPort = serialPort;
}

ZWaveAdapters::ZWaveAdapters()
{

}

ZWaveAdapters::ZWaveAdapters(const QList<ZWaveAdapter> &other):
    QList<ZWaveAdapter>(other)
{

}

bool ZWaveAdapters::hasSerialPort(const QString &serialPort)
{
    foreach (const ZWaveAdapter &adapter, *this) {
        if (adapter.serialPort() == serialPort) {
            return true;
        }
    }
    return false;
}

QVariant ZWaveAdapters::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void ZWaveAdapters::put(const QVariant &variant)
{
    append(variant.value<ZWaveAdapter>());
}
