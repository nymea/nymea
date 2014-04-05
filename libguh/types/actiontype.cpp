/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

/*!
    \class ActionType
    \brief Describes an \l{Action} for a \l{Device}.

    \ingroup types
    \inmodule libguh

    ActionTypes are contained in \l{DeviceClass} templates returned
    by \l{DevicePlugin}{DevicePlugins} in order to describe the hardware supported
    by the plugin.

    All Actions must have valid a ActionType in order to be usful.
    \sa Action
*/

#include "actiontype.h"

/*! Constructs an ActionType with the given \a id.*/
ActionType::ActionType(const ActionTypeId &id):
    m_id(id)
{
}

/*! Returns the id of this ActionType.*/
ActionTypeId ActionType::id() const
{
    return m_id;
}

/*! Returns the name of this ActionType */
QString ActionType::name() const
{
    return m_name;
}

/*! Set the \a name for this Action. This will be visible to to the user.*/
void ActionType::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the parameter description of this ActionType. \l{Action}{Actions} created
    from this ActionType must have their parameters matching to this template. */
QVariantList ActionType::parameters() const
{
    return m_parameters;
}

/*! Set the parameter description of this ActionType. \l{Action}{Actions} created
    from this ActionType must have their \a parameters matching to this template. */
void ActionType::setParameters(const QVariantList &parameters)
{
    m_parameters = parameters;
}
