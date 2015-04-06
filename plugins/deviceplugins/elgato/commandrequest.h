/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef COMMANDREQUEST_H
#define COMMANDREQUEST_H

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

#endif // COMMANDREQUEST_H
