/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

    \ingroup guh-types
    \inmodule libguh

    \sa Event, EventDescriptor
*/

#include "eventtype.h"

/*! Constructs a EventType object with the given \a id. */
EventType::EventType(const EventTypeId &id):
    m_id(id),
    m_index(0)
{

}

/*! Returns the id. */
EventTypeId EventType::id() const
{
    return m_id;
}

/*! Returns the name of this EventType, e.g. "Temperature changed". */
QString EventType::name() const
{
    return m_name;
}

/*! Set the name for this EventType to \a name, e.g. "Temperature changed". */
void EventType::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the index of this \l{EventType}. The index of an \l{EventType} indicates the order in the \l{DeviceClass}.
 *  This guarantees that a \l{Device} will look always the same (\l{Event} order). */
int EventType::index() const
{
    return m_index;
}

/*! Set the \a index of this \l{EventType}. */
void EventType::setIndex(const int &index)
{
    m_index = index;
}

/*! Holds a List describing possible parameters for a \l{Event} of this EventType.
 *  e.g. QList(ParamType("temperature", QVariant::Real)). */
QList<ParamType> EventType::paramTypes() const
{
    return m_paramTypes;
}

/*! Set the parameter description for this EventType to \a paramTypes,
 *  e.g. QList<ParamType>() << ParamType("temperature", QVariant::Real)). */
void EventType::setParamTypes(const QList<ParamType> &paramTypes)
{
    m_paramTypes = paramTypes;
}
