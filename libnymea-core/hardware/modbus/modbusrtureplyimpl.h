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

}

#endif // MODBUSRTUREPLYIMPL_H
