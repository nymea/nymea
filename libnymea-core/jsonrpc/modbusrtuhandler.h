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

#ifndef MODBUSRTUHANDLER_H
#define MODBUSRTUHANDLER_H

#include <QObject>

#include "hardware/modbus/modbusrtumaster.h"
#include "jsonrpc/jsonhandler.h"

namespace nymeaserver {

class ModbusRtuManager;

class ModbusRtuHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit ModbusRtuHandler(ModbusRtuManager *modbusRtuManager, QObject *parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply *GetSerialPorts(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetModbusRtuMasters(const QVariantMap &params);

    Q_INVOKABLE JsonReply *AddModbusRtuMaster(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveModbusRtuMaster(const QVariantMap &params);
    Q_INVOKABLE JsonReply *ReconfigureModbusRtuMaster(const QVariantMap &params);

signals:
    void SerialPortAdded(const QVariantMap &params);
    void SerialPortRemoved(const QVariantMap &params);

    void ModbusRtuMasterAdded(const QVariantMap &params);
    void ModbusRtuMasterRemoved(const QVariantMap &params);
    void ModbusRtuMasterChanged(const QVariantMap &params);

private:
    ModbusRtuManager *m_modbusRtuManager = nullptr;

    QVariantMap packModbusRtuMaster(ModbusRtuMaster *modbusRtuMaster);
};

} // namespace nymeaserver

#endif // MODBUSRTUHANDLER_H
