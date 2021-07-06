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

#ifndef MODBUSRTUHANDLER_H
#define MODBUSRTUHANDLER_H

#include <QObject>

#include "jsonrpc/jsonhandler.h"
#include "hardware/modbus/modbusrtumaster.h"

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

}

#endif // MODBUSRTUHANDLER_H
