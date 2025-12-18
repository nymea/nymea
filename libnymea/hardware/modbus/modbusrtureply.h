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

#ifndef MODBUSRTUREPLY_H
#define MODBUSRTUREPLY_H

#include <QObject>
#include <QVector>

class ModbusRtuReply : public QObject
{
    Q_OBJECT
public:
    enum Error { NoError, ReadError, WriteError, ConnectionError, ConfigurationError, TimeoutError, ProtocolError, ReplyAbortedError, UnknownError };
    Q_ENUM(Error)

    virtual bool isFinished() const = 0;

    virtual int slaveAddress() const = 0;
    virtual int registerAddress() const = 0;

    virtual QString errorString() const = 0;
    virtual ModbusRtuReply::Error error() const = 0;

    virtual QVector<quint16> result() const = 0;

protected:
    explicit ModbusRtuReply(QObject *parent = nullptr)
        : QObject(parent){};
    virtual ~ModbusRtuReply() = default;

signals:
    void finished();
    void errorOccurred(ModbusRtuReply::Error error);
};

#endif // MODBUSRTUREPLY_H
