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

#ifndef MODBUSRTUREPLY_H
#define MODBUSRTUREPLY_H

#include <QObject>
#include <QVector>

class ModbusRtuReply : public QObject
{
    Q_OBJECT
public:
    enum Error {
        NoError,
        ReadError,
        WriteError,
        ConnectionError,
        ConfigurationError,
        TimeoutError,
        ProtocolError,
        ReplyAbortedError,
        UnknownError
    };
    Q_ENUM(Error)

    virtual bool isFinished() const = 0;

    virtual int slaveAddress() const = 0;
    virtual int registerAddress() const = 0;

    virtual QString errorString() const = 0;
    virtual ModbusRtuReply::Error error() const = 0;

    virtual QVector<quint16> result() const = 0;

protected:
    explicit ModbusRtuReply(QObject *parent = nullptr) : QObject(parent) { };
    virtual ~ModbusRtuReply() = default;

signals:
    void finished();
    void errorOccurred(ModbusRtuReply::Error error);

};

#endif // MODBUSRTUREPLY_H
