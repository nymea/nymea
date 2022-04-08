/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2021, nymea GmbH
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

#include "modbusrtureplyimpl.h"

namespace nymeaserver {

ModbusRtuReplyImpl::ModbusRtuReplyImpl(int slaveAddress, int registerAddress, QObject *parent) :
    ModbusRtuReply(parent),
    m_slaveAddress(slaveAddress),
    m_registerAddress(registerAddress)
{
    m_timeoutTimer.start(10000);
    connect(&m_timeoutTimer, &QTimer::timeout, this, [=](){
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

}
