// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class EventDescriptor
    \brief Describes a certain \l{Event}.

    \ingroup nymea-types
    \ingroup rules
    \inmodule libnymea

    An EventDescriptor describes an \l{Event} in order to match it with a \l{nymeaserver::Rule}.

    An EventDescriptor can either be bound to a certain device/eventtype, or to an interface.
    If an event is bound to a device, it will only match when the given device fires the given event.
    If an event is bound to an interface, it will match the given event for all the devices implementing
    the given interface.

    \sa Event, EventType, nymeaserver::Rule
*/

/*! \enum EventDescriptor::Type

    \value TypeDevice
        The EventDescriptor describes a device Event.
    \value TypeInterface
        The EventDescriptor describes an interface based Event.
*/

#include "eventdescriptor.h"

EventDescriptor::EventDescriptor() {}

/*! Constructs an EventDescriptor describing an \l{Event} with the given \a eventTypeId, \a deviceId and the given \a paramDescriptors. */
EventDescriptor::EventDescriptor(const EventTypeId &eventTypeId, const ThingId &thingId, const QList<ParamDescriptor> &paramDescriptors)
    : m_eventTypeId(eventTypeId)
    , m_thingId(thingId)
    , m_paramDescriptors(paramDescriptors)
{}

/*! Constructs an EventDescriptor describing an \l{Event} with the given \a interface, \a interfaceEvent and the given \a paramDescriptors. */
EventDescriptor::EventDescriptor(const QString &interface, const QString &interfaceEvent, const QList<ParamDescriptor> &paramDescriptors)
    : m_interface(interface)
    , m_interfaceEvent(interfaceEvent)
    , m_paramDescriptors(paramDescriptors)
{}

/*! Returns true \l{EventDescriptor::Type}{Type} of this descriptor. */
EventDescriptor::Type EventDescriptor::type() const
{
    return (!m_thingId.isNull() && !m_eventTypeId.isNull()) ? TypeThing : TypeInterface;
}

/*! Returns true if the EventDescriptor is valid, that is, when it has either enough data to describe a device/eventType or an interface/interfaceEvent pair. */
bool EventDescriptor::isValid() const
{
    return (!m_thingId.isNull() && !m_eventTypeId.isNull()) || (!m_interface.isEmpty() && !m_interfaceEvent.isEmpty());
}

/*! Returns the id of the \l{EventType} which describes this Event. */
EventTypeId EventDescriptor::eventTypeId() const
{
    return m_eventTypeId;
}

void EventDescriptor::setEventTypeId(const EventTypeId &eventTypeId)
{
    m_eventTypeId = eventTypeId;
}

/*! Returns the id of the \l{Thing} associated with this Event. */
ThingId EventDescriptor::thingId() const
{
    return m_thingId;
}

void EventDescriptor::setThingId(const ThingId &thingId)
{
    m_thingId = thingId;
}

/*! Returns the interface associated with this EventDescriptor. */
QString EventDescriptor::interface() const
{
    return m_interface;
}

void EventDescriptor::setInterface(const QString &interface)
{
    m_interface = interface;
}

/*! Returns the interface's event name associated with this EventDescriptor.*/
QString EventDescriptor::interfaceEvent() const
{
    return m_interfaceEvent;
}

void EventDescriptor::setInterfaceEvent(const QString &interfaceEvent)
{
    m_interfaceEvent = interfaceEvent;
}

/*! Returns the parameters of this Event. */
QList<ParamDescriptor> EventDescriptor::paramDescriptors() const
{
    return m_paramDescriptors;
}

/*! Set the parameters of this Event to \a paramDescriptors. */
void EventDescriptor::setParamDescriptors(const QList<ParamDescriptor> &paramDescriptors)
{
    m_paramDescriptors = paramDescriptors;
}

/*! Returns the ParamDescriptor with the given \a paramTypeId, otherwise an invalid ParamDescriptor. */
ParamDescriptor EventDescriptor::paramDescriptor(const ParamTypeId &paramTypeId) const
{
    foreach (const ParamDescriptor &paramDescriptor, m_paramDescriptors) {
        if (paramDescriptor.paramTypeId() == paramTypeId) {
            return paramDescriptor;
        }
    }
    return ParamDescriptor(QString());
}

/*! Compare this Event to the Event given by \a other.
 *  Events are equal (returns true) if eventTypeId, deviceId and params match. */
bool EventDescriptor::operator==(const EventDescriptor &other) const
{
    bool paramsMatch = true;
    foreach (const ParamDescriptor &otherParamDescriptor, other.paramDescriptors()) {
        ParamDescriptor paramDescriptor = this->paramDescriptor(otherParamDescriptor.paramTypeId());
        if (!paramDescriptor.isValid() || paramDescriptor.value() != otherParamDescriptor.value()) {
            paramsMatch = false;
            break;
        }
    }

    return m_eventTypeId == other.eventTypeId() && m_thingId == other.thingId() && paramsMatch;
}

/*! Print an EventDescriptor including ParamDescriptors to QDebug. */
QDebug operator<<(QDebug dbg, const EventDescriptor &eventDescriptor)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "EventDescriptor(EventTypeId: " << eventDescriptor.eventTypeId().toString() << ", ThingId:" << eventDescriptor.thingId().toString()
                  << ", Interface:" << eventDescriptor.interface() << ", InterfaceEvent:" << eventDescriptor.interfaceEvent() << ")" << '\n';
    for (int i = 0; i < eventDescriptor.paramDescriptors().count(); i++) {
        dbg.nospace() << "    " << i << ": " << eventDescriptor.paramDescriptors().at(i);
    }

    return dbg;
}

/*! Writes each \a eventDescriptors to \a dbg. */
QDebug operator<<(QDebug dbg, const QList<EventDescriptor> &eventDescriptors)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "EventDescriptorList (count:" << eventDescriptors.count() << "):" << '\n';
    for (int i = 0; i < eventDescriptors.count(); i++) {
        dbg.nospace() << "  " << i << ": " << eventDescriptors.at(i);
    }

    return dbg;
}

EventDescriptors::EventDescriptors() {}

EventDescriptors::EventDescriptors(const QList<EventDescriptor> &other)
    : QList<EventDescriptor>(other)
{}

QVariant EventDescriptors::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void EventDescriptors::put(const QVariant &variant)
{
    append(variant.value<EventDescriptor>());
}
