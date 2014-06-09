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

#include "statedescriptor.h"

StateDescriptor::StateDescriptor():
    m_id(StateDescriptorId::createStateDescriptorId()),
    m_operatorType(ValueOperatorEquals)
{

}

StateDescriptor::StateDescriptor(const StateDescriptorId &id, const StateTypeId &stateTypeId, const DeviceId &deviceId, const QVariant &stateValue, ValueOperator operatorType):
    m_id(id),
    m_stateTypeId(stateTypeId),
    m_deviceId(deviceId),
    m_stateValue(stateValue),
    m_operatorType(operatorType)
{

}

StateDescriptorId StateDescriptor::id() const
{
    return m_id;
}

StateTypeId StateDescriptor::stateTypeId() const
{
    return m_stateTypeId;
}

DeviceId StateDescriptor::deviceId() const
{
    return m_deviceId;
}

QVariant StateDescriptor::stateValue() const
{
    return m_stateValue;
}

ValueOperator StateDescriptor::operatorType() const
{
    return m_operatorType;
}

bool StateDescriptor::operator ==(const StateDescriptor &other) const
{
    return m_stateTypeId == other.stateTypeId() &&
            m_deviceId == other.deviceId() &&
            m_stateValue == other.stateValue() &&
            m_operatorType == other.operatorType();
}

bool StateDescriptor::operator ==(const State &state) const
{
    if ((m_stateTypeId != state.stateTypeId()) || (m_deviceId != state.deviceId())) {
        return false;
    }
    switch (m_operatorType) {
    case ValueOperatorEquals:
        return m_stateValue == state.value();
    case ValueOperatorGreater:
        return m_stateValue > state.value();
    case ValueOperatorGreaterOrEqual:
        return m_stateValue >= state.value();
    case ValueOperatorLess:
        return m_stateValue < state.value();
    case ValueOperatorLessOrEqual:
        return m_stateValue <= state.value();
    case ValueOperatorNotEquals:
        return m_stateValue != state.value();
    }
    return false;
}

bool StateDescriptor::operator !=(const State &state) const
{
    return !(operator==(state));
}
