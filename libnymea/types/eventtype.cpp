/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
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

/*!
    \class EventType
    \brief Describes a \l{Event} for a \l{Device}.

    \ingroup nymea-types
    \inmodule libnymea

    \sa Event, EventDescriptor
*/

#include "eventtype.h"

EventType::EventType()
{

}

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

/*! Returns the name of this EventType. */
QString EventType::name() const
{
    return m_name;
}

/*! Set the name for this EventType to \a name. */
void EventType::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the displayName of this EventType, e.g. "Temperature changed". */
QString EventType::displayName() const
{
    return m_displayName;
}

/*! Set the displayName for this EventType to \a displayName, e.g. "Temperature changed". */
void EventType::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
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
ParamTypes EventType::paramTypes() const
{
    return m_paramTypes;
}

/*! Set the parameter description for this EventType to \a paramTypes,
 *  e.g. QList<ParamType>() << ParamType("temperature", QVariant::Real)). */
void EventType::setParamTypes(const ParamTypes &paramTypes)
{
    m_paramTypes = paramTypes;
}

/*! Returns true if this EventType has a valid id and name */
bool EventType::isValid() const
{
    return !m_id.isNull() && !m_name.isEmpty();
}

/*! Returns a list of all valid JSON properties a EventType JSON definition can have. */
QStringList EventType::typeProperties()
{
    return QStringList() << "id" << "name" << "displayName" << "paramTypes";
}

/*! Returns a list of mandatory JSON properties a EventType JSON definition must have. */
QStringList EventType::mandatoryTypeProperties()
{
    return QStringList() << "id" << "name" << "displayName";
}

EventTypes::EventTypes(const QList<EventType> &other)
{
    foreach (const EventType &at, other) {
        append(at);
    }
}

QVariant EventTypes::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void EventTypes::put(const QVariant &variant)
{
    append(variant.value<EventType>());
}

EventType EventTypes::findByName(const QString &name)
{
    foreach (const EventType &eventType, *this) {
        if (eventType.name() == name) {
            return eventType;
        }
    }
    return EventType(EventTypeId());
}

EventType EventTypes::findById(const EventTypeId &id)
{
    foreach (const EventType &eventType, *this) {
        if (eventType.id() == id) {
            return eventType;
        }
    }
    return EventType(EventTypeId());
}
