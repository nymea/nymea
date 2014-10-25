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

/*!
    \class StateDescriptor
    \brief Describes a certain \l{State}.

    \ingroup types
    \inmodule libguh

    An StateDescriptor describes a \l{State} in order to match it with a \l{Rule}.

    \sa State, Rule
*/


#include "statedescriptor.h"

StateDescriptor::StateDescriptor():
    m_operatorType(Types::ValueOperatorEquals)
{

}

StateDescriptor::StateDescriptor(const StateTypeId &stateTypeId, const DeviceId &deviceId, const QVariant &stateValue, Types::ValueOperator operatorType):
    m_stateTypeId(stateTypeId),
    m_deviceId(deviceId),
    m_stateValue(stateValue),
    m_operatorType(operatorType)
{

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

Types::ValueOperator StateDescriptor::operatorType() const
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
    case Types::ValueOperatorEquals:
        return m_stateValue == state.value();
    case Types::ValueOperatorGreater:
        return state.value() > m_stateValue;
    case Types::ValueOperatorGreaterOrEqual:
        return state.value() >= m_stateValue;
    case Types::ValueOperatorLess:
        return state.value() < m_stateValue;
    case Types::ValueOperatorLessOrEqual:
        return state.value() <= m_stateValue;
    case Types::ValueOperatorNotEquals:
        return m_stateValue != state.value();
    }
    return false;
}

bool StateDescriptor::operator !=(const State &state) const
{
    return !(operator==(state));
}

bool StateDescriptor::isValid() const
{
    return !m_deviceId.isNull() && !m_stateTypeId.isNull() && m_stateValue.isValid();
}
