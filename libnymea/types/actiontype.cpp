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
    \class ActionType
    \brief Describes an \l{Action} for a \l{Device}.

    \ingroup nymea-types
    \inmodule libnymea

    ActionTypes are contained in \l{DeviceClass} templates returned
    by \l{DevicePlugin}{DevicePlugins} in order to describe the hardware supported
    by the plugin.

    All Actions must have valid a ActionType in order to be useful.

    \sa Action
*/

#include "actiontype.h"

/*! Constructs an \l{ActionType} with the given \a id. */
ActionType::ActionType(const ActionTypeId &id):
    m_id(id),
    m_index(0)
{

}

/*! Returns the id of this \l{ActionType}. */
ActionTypeId ActionType::id() const
{
    return m_id;
}

/*! Returns the name of this \l{ActionType}. */
QString ActionType::name() const
{
    return m_name;
}

/*! Set the \a name for this \l{ActionType}. */
void ActionType::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the display name of this \l{ActionType}. */
QString ActionType::displayName() const
{
    return m_displayName;
}

/*! Set the \a displayName for this \l{ActionType}. This will be visible to the user. */
void ActionType::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

/*! Returns the index of this \l{ActionType}. The index of an \l{ActionType} indicates the order in the \l{DeviceClass}.
 *  This guarantees that a \l{Device} will look always the same (\l{Action} order). */
int ActionType::index() const
{
    return m_index;
}

/*! Set the \a index of this \l{ActionType}. */
void ActionType::setIndex(const int &index)
{
    m_index = index;
}

/*! Returns the parameter description of this \l{ActionType}. \l{Action}{Actions} created
 *  from this \l{ActionType} must have their parameters matching to this template. */
ParamTypes ActionType::paramTypes() const
{
    return m_paramTypes;
}

/*! Set the parameter description of this \l{ActionType}. \l{Action}{Actions} created
 *  from this \l{ActionType} must have their \a paramTypes matching to this template. */
void ActionType::setParamTypes(const ParamTypes &paramTypes)
{
    m_paramTypes = paramTypes;
}

/*! Returns a list of all valid properties a ActionType definition can have. */
QStringList ActionType::typeProperties()
{
    return QStringList() << "id" << "name" << "displayName" << "paramTypes";
}

/*! Returns a list of mandatory properties a ActionType definition must have. */
QStringList ActionType::mandatoryTypeProperties()
{
    return QStringList() << "id" << "name" << "displayName";
}

ActionTypes::ActionTypes(const QList<ActionType> &other)
{
    foreach (const ActionType &at, other) {
        append(at);
    }
}

bool ActionTypes::contains(const ActionTypeId &id) const
{
    foreach (const ActionType &actionType, *this) {
        if (actionType.id() == id) {
            return true;
        }
    }
    return false;
}

bool ActionTypes::contains(const QString &name) const
{
    foreach (const ActionType &actionType, *this) {
        if (actionType.name() == name) {
            return true;
        }
    }
    return false;
}

QVariant ActionTypes::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void ActionTypes::put(const QVariant &variant)
{
    append(variant.value<ActionType>());
}

ActionType ActionTypes::findByName(const QString &name)
{
    foreach (const ActionType &actionType, *this) {
        if (actionType.name() == name) {
            return actionType;
        }
    }
    return ActionType(ActionTypeId());
}

ActionType ActionTypes::findById(const ActionTypeId &id)
{
    foreach (const ActionType &actionType, *this) {
        if (actionType.id() == id) {
            return actionType;
        }
    }
    return ActionType(ActionTypeId());
}

ActionType &ActionTypes::operator[](const QString &name)
{
    int index = -1;
    for (int i = 0; i < count(); i++) {
        if (at(i).name() == name) {
            index = i;
            break;
        }
    }
    return QList::operator[](index);
}

QDebug operator<<(QDebug dbg, const ActionType &actionType)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace().noquote() << "ActionType(" << actionType.name() << ", " << actionType.displayName() << ", " << actionType.id() << ") ";
    return dbg;
}

