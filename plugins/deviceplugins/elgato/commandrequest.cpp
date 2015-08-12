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

#ifdef BLUETOOTH_LE

#include "commandrequest.h"

CommandRequest::CommandRequest() :
    m_service(0),
    m_characteristic(QLowEnergyCharacteristic()),
    m_value(QByteArray())
{
}

CommandRequest::CommandRequest(QLowEnergyService *service, const QLowEnergyCharacteristic &characteristic, const QByteArray &value) :
    m_service(service),
    m_characteristic(characteristic),
    m_value(value)
{
}

QLowEnergyService *CommandRequest::service()
{
    return m_service;
}

QLowEnergyCharacteristic CommandRequest::characteristic() const
{
    return m_characteristic;
}

QByteArray CommandRequest::value() const
{
    return m_value;
}

#endif // BLUETOOTH_LE
