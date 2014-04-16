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
    \class Event
    \brief Holds information required to emit a event described by a \l{EventType}.

    \ingroup types
    \inmodule libguh

    It is bound to a \l{Device} and a \l{EventType} and holds the parameters
    for the event that happened.

    The params must match the template as described in \l{EventType}.

    \sa Device, EventType
*/

#include "event.h"

/*! Constructs a Event reflecting the \l{Event} given by \a EventTypeId, associated with
    the \l{Device} given by \a deviceId and the parameters given by \a params. The parameters must
    match the description in the reflecting \l{Event}.*/
Event::Event(const EventTypeId &eventTypeId, const DeviceId &deviceId, const QVariantMap &params):
    m_eventTypeId(eventTypeId),
    m_deviceId(deviceId),
    m_params(params)
{
}

/*! Returns the id of the \l{EventType} which describes this Event.*/
EventTypeId Event::eventTypeId() const
{
    return m_eventTypeId;
}

/*! Returns the id of the \l{Device} associated with this Event.*/
DeviceId Event::deviceId() const
{
    return m_deviceId;
}

/*! Returns the parameters of this Event.*/
QVariantMap Event::params() const
{
    return m_params;
}

/*! Set the parameters of this Event to \a params.*/
void Event::setParams(const QVariantMap &params)
{
    m_params = params;
}

/*! Compare this Event to the Event given by \a other.
    Events are equal (returns true) if eventTypeId, deviceId and params match. */
bool Event::operator ==(const Event &other) const
{
    return m_eventTypeId == other.eventTypeId()
            && m_deviceId == other.deviceId()
            && m_params == other.params();
}

QDebug operator<<(QDebug dbg, const Event &event)
{
    dbg.nospace() << "Event(EventTypeId: " << event.eventTypeId().toString() << ", DeviceId" << event.deviceId() << ")";

    return dbg.space();
}

QDebug operator<<(QDebug dbg, const QList<Event> &events)
{
    dbg.nospace() << "EventList (count:" << events.count() << ")";
    for (int i = 0; i < events.count(); i++ ) {
        dbg.nospace() << "     " << i << ": " << events.at(i);
    }

    return dbg.space();
}
