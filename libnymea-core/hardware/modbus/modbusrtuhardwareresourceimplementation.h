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

#ifndef MODBUSRTUHARDWARERESOURCEIMPLEMENTATION_H
#define MODBUSRTUHARDWARERESOURCEIMPLEMENTATION_H

#include <QObject>

#include "hardware/modbus/modbusrtumanager.h"
#include "hardware/modbus/modbusrtumaster.h"
#include "hardware/modbus/modbusrtuhardwareresource.h"

namespace nymeaserver {

class ModbusRtuHardwareResourceImplementation : public ModbusRtuHardwareResource
{
    Q_OBJECT
public:
    explicit ModbusRtuHardwareResourceImplementation(ModbusRtuManager *modbusRtuManager, QObject *parent = nullptr);
    ~ModbusRtuHardwareResourceImplementation() override = default;

    QList<ModbusRtuMaster *> modbusRtuMasters() const override;
    bool hasModbusRtuMaster(const QUuid &modbusUuid) const override;
    ModbusRtuMaster *getModbusRtuMaster(const QUuid &modbusUuid) const override;

    bool available() const override;
    bool enabled() const override;

public slots:
    bool enable();
    bool disable();

protected:
    void setEnabled(bool enabled) override;

private:
    ModbusRtuManager *m_modbusRtuManager = nullptr;
    bool m_available = false;
    bool m_enabled = false;

};

}

#endif // MODBUSRTUHARDWARERESOURCEIMPLEMENTATION_H
