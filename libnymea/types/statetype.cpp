/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
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


/*!
    \class StateType
    \brief Describes the Type of a \l{State} from \l{Device}.

    \ingroup nymea-types
    \inmodule libnymea

    \sa State, StateDescriptor
*/

#include "statetype.h"

StateType::StateType()
{

}

/*! Constructs a StateType with the given \a id.
 *  When creating a \l{DevicePlugin} generate a new uuid for each StateType you define and
 *  hardcode it into the plugin json file. */
StateType::StateType(const StateTypeId &id):
    m_id(id)
{

}

/*! Returns the id of the StateType. */
StateTypeId StateType::id() const
{
    return m_id;
}

/*! Returns the name of the StateType. This is used internally, e.g. to match \l{Interfaces for DeviceClasses}{interfaces}. */
QString StateType::name() const
{
    return m_name;
}

/*! Set the name of the StateType to \a name. This is used internally, e.g. to match \l{Interfaces for DeviceClasses}{interfaces}. */
void StateType::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the displayName of the StateType. This is visible to the user (e.g. "Color temperature"). */
QString StateType::displayName() const
{
    return m_displayName;
}

/*! Set the displayName of the StateType to \a displayName. This is visible to the user (e.g. "Color temperature"). */
void StateType::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

/*! Returns the index of this \l{StateType}. The index of an \l{StateType} indicates the order in the \l{DeviceClass}.
 *  This guarantees that a \l{Device} will look always the same (\l{State} order). */
int StateType::index() const
{
    return m_index;
}

/*! Set the \a index of this \l{StateType}. */
void StateType::setIndex(const int &index)
{
    m_index = index;
}

/*! Returns the Type of the StateType (e.g. QVariant::Real). */
QVariant::Type StateType::type() const
{
    return m_type;
}

/*! Set the type fo the StateType to \a type (e.g. QVariant::Real). */
void StateType::setType(const QVariant::Type &type)
{
    m_type = type;
}

/*! Returns the default value of this StateType (e.g. 21.5). */
QVariant StateType::defaultValue() const
{
    return m_defaultValue;
}

/*! Set the default value of this StateType to \a defaultValue (e.g. 21.5). */
void StateType::setDefaultValue(const QVariant &defaultValue)
{
    m_defaultValue = defaultValue;
}

/*! Returns the minimum value of this StateType. If this value is not set, the QVariant will be invalid. */
QVariant StateType::minValue() const
{
    return m_minValue;
}

/*! Set the minimum value of this StateType to \a minValue. If this value is not set,
 *  there is now lower limit. */
void StateType::setMinValue(const QVariant &minValue)
{
    m_minValue = minValue;
}

/*! Returns the maximum value of this StateType. If this value is not set, the QVariant will be invalid. */
QVariant StateType::maxValue() const
{
    return m_maxValue;
}

/*! Set the maximum value of this StateType to \a maxValue. If this value is not set,
 *  there is now upper limit. */
void StateType::setMaxValue(const QVariant &maxValue)
{
    m_maxValue = maxValue;
}

/*! Returns the list of possible values of this StateType. If the list is empty or invalid the \l{State} value can take every value. */
QVariantList StateType::possibleValues() const
{
    return m_possibleValues;
}

/*! Set the list of possible values of this StateType to \a possibleValues. */
void StateType::setPossibleValues(const QVariantList &possibleValues)
{
    m_possibleValues = possibleValues;
}

/*! Returns the unit of this StateType. */
Types::Unit StateType::unit() const
{
    return m_unit;
}

/*! Sets the unit of this StateType to the given \a unit. */
void StateType::setUnit(const Types::Unit &unit)
{
    m_unit = unit;
}

/*! Returns the IO type of this StateType. */
Types::IOType StateType::ioType() const
{
    return m_ioType;
}

/*! Sets the IO type of this StateType. */
void StateType::setIOType(Types::IOType ioType)
{
    m_ioType = ioType;
}

/*! Returns whether the StateType is writable or not. A writable StateType will have an according ActionType defined.*/
bool StateType::writable() const
{
    return m_writable;
}

/*! Sets the writable property to true */
void StateType::setWritable(bool writable)
{
    m_writable = writable;
}

/*! Returns true if this StateType is to be cached. This means, the last state value will be stored to disk upon shutdown and restored on reboot. If this is false, states will be initialized with the default value on each boot. By default all states are cached by the system. */
bool StateType::cached() const
{
    return m_cached;
}

/*! Sets whether this StateType should be \a cached or not. If a state value gets cached, the state will be initialized with the cached value on start.*/
void StateType::setCached(bool cached)
{
    m_cached = cached;
}

/*! Returns a list of all valid properties a DeviceClass definition can have. */
QStringList StateType::typeProperties()
{
    return QStringList() << "id" << "name" << "displayName" << "displayNameEvent" << "type" << "defaultValue"
                         << "cached" << "unit" << "minValue" << "maxValue" << "possibleValues" << "writable"
                         << "displayNameAction" << "ioType";
}

/*! Returns a list of mandatory properties a DeviceClass definition must have. */
QStringList StateType::mandatoryTypeProperties()
{
    return QStringList() << "id" << "name" << "displayName" << "displayNameEvent" << "type" << "defaultValue";
}

/*! Returns true if this state type has an ID, a type and a name set. */
bool StateType::isValid() const
{
    return !m_id.isNull() && m_type != QVariant::Invalid && !m_name.isEmpty();
}

StateTypes::StateTypes(const QList<StateType> &other)
{
    foreach (const StateType &st, other) {
        append(st);
    }
}

bool StateTypes::contains(const StateTypeId &stateTypeId)
{
    foreach (const StateType &stateType, *this) {
        if (stateType.id() == stateTypeId) {
            return true;
        }
    }
    return false;
}

QVariant StateTypes::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void StateTypes::put(const QVariant &variant)
{
    append(variant.value<StateType>());
}

StateType StateTypes::findByName(const QString &name)
{
    foreach (const StateType &stateType, *this) {
        if (stateType.name() == name) {
            return stateType;
        }
    }
    return StateType(StateTypeId());
}

StateType StateTypes::findById(const StateTypeId &id)
{
    foreach (const StateType &stateType, *this) {
        if (stateType.id() == id) {
            return stateType;
        }
    }
    return StateType(StateTypeId());
}
