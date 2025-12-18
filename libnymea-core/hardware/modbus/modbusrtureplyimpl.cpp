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

#include "modbusrtureplyimpl.h"

namespace nymeaserver {

ModbusRtuReplyImpl::ModbusRtuReplyImpl(int slaveAddress, int registerAddress, QObject *parent)
    : ModbusRtuReply(parent)
    , m_slaveAddress(slaveAddress)
    , m_registerAddress(registerAddress)
{
    m_timeoutTimer.start(10000);
    connect(&m_timeoutTimer, &QTimer::timeout, this, [=]() {
        if (!m_finished) {
            m_error = TimeoutError;
            emit errorOccurred(TimeoutError);
        }
    });
}

bool ModbusRtuReplyImpl::isFinished() const
{
    return m_finished;
}

void ModbusRtuReplyImpl::setFinished(bool finished)
{
    m_finished = finished;
}

int ModbusRtuReplyImpl::slaveAddress() const
{
    return m_slaveAddress;
}

int ModbusRtuReplyImpl::registerAddress() const
{
    return m_registerAddress;
}

QString ModbusRtuReplyImpl::errorString() const
{
    return m_errorString;
}

void ModbusRtuReplyImpl::setErrorString(const QString &errorString)
{
    m_errorString = errorString;
}

ModbusRtuReply::Error ModbusRtuReplyImpl::error() const
{
    return m_error;
}

void ModbusRtuReplyImpl::setError(ModbusRtuReply::Error error)
{
    m_error = error;
}

QVector<quint16> ModbusRtuReplyImpl::result() const
{
    return m_result;
}

void ModbusRtuReplyImpl::setResult(const QVector<quint16> &result)
{
    m_result = result;
}

} // namespace nymeaserver
