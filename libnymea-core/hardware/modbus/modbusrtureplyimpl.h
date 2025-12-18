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

#ifndef MODBUSRTUREPLYIMPL_H
#define MODBUSRTUREPLYIMPL_H

#include <QObject>
#include <QTimer>

#include "hardware/modbus/modbusrtureply.h"

namespace nymeaserver {

class ModbusRtuReplyImpl : public ModbusRtuReply
{
    Q_OBJECT
public:
    explicit ModbusRtuReplyImpl(int slaveAddress, int registerAddress, QObject *parent = nullptr);

    bool isFinished() const override;
    void setFinished(bool finished);

    int slaveAddress() const override;
    int registerAddress() const override;

    QString errorString() const override;
    void setErrorString(const QString &errorString);

    ModbusRtuReply::Error error() const override;
    void setError(ModbusRtuReply::Error error);

    QVector<quint16> result() const override;
    void setResult(const QVector<quint16> &result);

private:
    bool m_finished = false;
    int m_slaveAddress;
    int m_registerAddress;
    Error m_error = UnknownError;
    QString m_errorString;
    QVector<quint16> m_result;
    QTimer m_timeoutTimer;
};

} // namespace nymeaserver

#endif // MODBUSRTUREPLYIMPL_H
