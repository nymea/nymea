/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
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
    \class Event
    \brief Holds information required to emit a event described by a \l{EventType}.

    \ingroup guh-types
    \inmodule libnymea

    It is bound to a \l{Device} and a \l{EventType} and holds the parameters
    for the event that happened.

    The params must match the template as described in \l{EventType}.

    \sa Device, EventType, EventDescriptor
*/

#include "event.h"

/*! Constructs an Event. */
Event::Event():
    m_id(EventId::createEventId())
{
}

/*! Constructs an Event reflecting the \l{Event} given by \a eventTypeId, associated with
 *  the \l{Device} given by \a deviceId and the parameters given by \a params. The parameter \a isStateChangeEvent
 *  specifies if the \l{Event} will be autogeneratet or not. The parameters must
 *  match the description in the reflecting \l{Event}.  */
Event::Event(const EventTypeId &eventTypeId, const DeviceId &deviceId, const ParamList &params, bool isStateChangeEvent):
    m_id(EventId::createEventId()),
    m_eventTypeId(eventTypeId),
    m_deviceId(deviceId),
    m_params(params),
    m_isStateChangeEvent(isStateChangeEvent)
{
}

/*! Returns the Id of this Event. Each newly created Event will have a new UUID generated. The id will be copied
 *  in the copy constructor. */
EventId Event::eventId() const
{
    return m_id;
}

/*! Returns the id of the \l{EventType} which describes this Event. */
EventTypeId Event::eventTypeId() const
{
    return m_eventTypeId;
}

/*! Set the EventTypeId for this Event to the given \a eventTypeId. */
void Event::setEventTypeId(const EventTypeId &eventTypeId)
{
    m_eventTypeId = eventTypeId;
}

/*! Returns the id of the \l{Device} associated with this Event. */
DeviceId Event::deviceId() const
{
    return m_deviceId;
}

/*! Set the \a deviceId for this Event. */
void Event::setDeviceId(const DeviceId &deviceId)
{
    m_deviceId = deviceId;
}

/*! Returns the parameters of this Event. */
ParamList Event::params() const
{
    return m_params;
}

/*! Set the parameters of this Event to \a params. */
void Event::setParams(const ParamList &params)
{
    m_params = params;
}

/*! Returns the parameter of this Event with the given \a paramTypeId. */
Param Event::param(const ParamTypeId &paramTypeId) const
{
    foreach (const Param &param, m_params) {
        if (param.paramTypeId() == paramTypeId) {
            return param;
        }
    }
    return Param(QString());
}

/*! Returns true if this event is autogenerated by a state change. */
bool Event::isStateChangeEvent() const
{
    return m_isStateChangeEvent;
}

/*! Compare this Event to the Event given by \a other.
 *  Events are equal (returns true) if eventTypeId, deviceId and params match. */
bool Event::operator ==(const Event &other) const
{
    bool paramsMatch = true;
    foreach (const Param &otherParam, other.params()) {
        Param param = this->param(otherParam.paramTypeId());
        if (!param.isValid() || param.value() != otherParam.value()) {
            paramsMatch = false;
            break;
        }
    }

    return m_eventTypeId == other.eventTypeId()
            && m_deviceId == other.deviceId()
            && paramsMatch;
}

/*! Writes the eventTypeId and the deviceId of the given \a event to \a dbg. */
QDebug operator<<(QDebug dbg, const Event &event)
{
    dbg.nospace() << "Event(EventTypeId: " << event.eventTypeId().toString() << ", DeviceId" << event.deviceId().toString() << ")";
    return dbg.space();
}

/*! Writes the each \l{Event} of the given \a events to \a dbg. */
QDebug operator<<(QDebug dbg, const QList<Event> &events)
{
    dbg.nospace() << "EventList (count:" << events.count() << ")";
    for (int i = 0; i < events.count(); i++ ) {
        dbg.nospace() << "     " << i << ": " << events.at(i);
    }

    return dbg.space();
}