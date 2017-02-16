/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef COMMANDREQUEST_H
#define COMMANDREQUEST_H

#ifdef BLUETOOTH_LE

#include <QLowEnergyService>
#include <QLowEnergyCharacteristic>
#include <QByteArray>

class CommandRequest
{

public:
    CommandRequest();
    CommandRequest(QLowEnergyService *service, const QLowEnergyCharacteristic &characteristic, const QByteArray &value);

    QLowEnergyService *service();
    QLowEnergyCharacteristic characteristic() const;
    QByteArray value() const;

private:
    QLowEnergyService *m_service;
    QLowEnergyCharacteristic m_characteristic;
    QByteArray m_value;

};

#endif // BLUETOOTH_LE

#endif // COMMANDREQUEST_H
