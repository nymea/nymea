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
    \class Event
    \brief Holds information required to emit a event described by a \l{EventType}.

    \ingroup nymea-types
    \inmodule libnymea

    It is bound to a \l{Device} and a \l{EventType} and holds the parameters
    for the event that happened.

    The params must match the template as described in \l{EventType}.

    \sa Device, EventType, EventDescriptor
*/

#include "event.h"

/*! Constructs an Event. */
Event::Event()
{
}

/*! Constructs an Event reflecting the \l{EventType} given by \a eventTypeId, associated with
 *  the \l{Device} given by \a deviceId and the parameters given by \a params. The parameters must
 *  match the description in the reflecting \l{EventType}.  */
Event::Event(const EventTypeId &eventTypeId, const ThingId &thingId, const ParamList &params):
    m_eventTypeId(eventTypeId),
    m_thingId(thingId),
    m_params(params)
{
}

Event::Event(const ThingId &thingId, const QString &name, const ParamList &params):
    m_thingId(thingId),
    m_name(name),
    m_params(params)
{

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

/*! Returns the id of the \l{Thing} associated with this Event. */
ThingId Event::thingId() const
{
    return m_thingId;
}

/*! Set the \l {ThingId} for this Event. */
void Event::setThingId(const ThingId &thingId)
{
    m_thingId = thingId;
}

QString Event::name() const
{
    return m_name;
}

void Event::setName(const QString &name)
{
    m_name = name;
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

/*! Returns the parameter for the given \a paramTypeId. The returned \l{Param} will be invalid if this Event does not have such a \l{Param}. */
Param Event::param(const ParamTypeId &paramTypeId) const
{
    foreach (const Param &param, m_params) {
        if (param.paramTypeId() == paramTypeId) {
            return param;
        }
    }
    return Param(paramTypeId);
}

/*! Returns the parameter value for the given \a paramTypeId. The returned \l{QVariant} will be null if this Event does not have such a \l{Param}. */
QVariant Event::paramValue(const ParamTypeId &paramTypeId) const
{
    foreach (const Param &param, m_params) {
        if (param.paramTypeId() == paramTypeId) {
            return param.value();
        }
    }
    return QVariant();
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
            && m_thingId == other.thingId()
            && m_name == other.name()
            && paramsMatch;
}

/*! Writes the eventTypeId and the deviceId of the given \a event to \a dbg. */
QDebug operator<<(QDebug dbg, const Event &event)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Event(EventTypeId: " << event.eventTypeId().toString() << ", ThingId: " << event.thingId().toString() << ", Name: " << event.name() << ", Params: " << event.params() << ")";
    return dbg;
}

/*! Writes the each \l{Event} of the given \a events to \a dbg. */
QDebug operator<<(QDebug dbg, const QList<Event> &events)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "EventList (count:" << events.count() << ")";
    for (int i = 0; i < events.count(); i++ ) {
        dbg.nospace() << "     " << i << ": " << events.at(i);
    }

    return dbg;
}
