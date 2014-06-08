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
    \class EventType
    \brief Describes a \l{Event} for a \l{Device}.

    \ingroup types
    \inmodule libguh

    \sa Event
*/

#include "eventtype.h"

/*! Constructs a EventType object with the given \a id. */
EventType::EventType(const EventTypeId &id):
    m_id(id)
{
}

/*! Returns the id. */
EventTypeId EventType::id() const
{
    return m_id;
}

/*! Returns the name of this EventType, e.g. "Temperature changed" */
QString EventType::name() const
{
    return m_name;
}

/*! Set the name for this EventType to \a name, e.g. "Temperature changed" */
void EventType::setName(const QString &name)
{
    m_name = name;
}

/*!
  Holds a List describing possible parameters for a \l{Event} of this EventType.
  e.g. QList(ParamType("temperature", QVariant::Real))
  */
QList<ParamType> EventType::parameters() const
{
    return m_parameters;
}

/*!
  Set the parameter description for this EventType to \a parameters,
  e.g. QList<ParamType>() << ParamType("temperature", QVariant::Real))
  */
void EventType::setParameters(const QList<ParamType> &parameters)
{
    m_parameters = parameters;
}
