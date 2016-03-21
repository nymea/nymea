/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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
    \class EventDescriptor
    \brief Describes a certain \l{Event}.

    \ingroup guh-types
    \ingroup rules
    \inmodule libguh

    An EventDescriptor describes an \l{Event} in order to match it with a \l{guhserver::Rule}.

    \sa Event, EventType, guhserver::Rule
*/

#include "eventdescriptor.h"

/*! Constructs an EventDescriptor describing an \l{Event} with the given \a eventTypeId, \a deviceId and the given \a paramDescriptors. */
EventDescriptor::EventDescriptor(const EventTypeId &eventTypeId, const DeviceId &deviceId, const QList<ParamDescriptor> &paramDescriptors):
    m_eventTypeId(eventTypeId),
    m_deviceId(deviceId),
    m_paramDescriptors(paramDescriptors)
{
}

/*! Returns the id of the \l{EventType} which describes this Event. */
EventTypeId EventDescriptor::eventTypeId() const
{
    return m_eventTypeId;
}

/*! Returns the id of the \l{Device} associated with this Event. */
DeviceId EventDescriptor::deviceId() const
{
    return m_deviceId;
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

/*! Set a parameters of this Event to \a paramDescriptorName. */
ParamDescriptor EventDescriptor::paramDescriptor(const QString &paramDescriptorName) const
{
    foreach (const ParamDescriptor &paramDescriptor, m_paramDescriptors) {
        if (paramDescriptor.name() == paramDescriptorName) {
            return paramDescriptor;
        }
    }
    return ParamDescriptor(QString());
}

/*! Compare this Event to the Event given by \a other.
 *  Events are equal (returns true) if eventTypeId, deviceId and params match. */
bool EventDescriptor::operator ==(const EventDescriptor &other) const
{
    bool paramsMatch = true;
    foreach (const ParamDescriptor &otherParamDescriptor, other.paramDescriptors()) {
        ParamDescriptor paramDescriptor = this->paramDescriptor(otherParamDescriptor.name());
        if (!paramDescriptor.isValid() || paramDescriptor.value() != otherParamDescriptor.value()) {
            paramsMatch = false;
            break;
        }
    }

    return m_eventTypeId == other.eventTypeId()
            && m_deviceId == other.deviceId()
            && paramsMatch;
}

/*! Compare this EventDescriptor to the Event given by \a event.
 *  Events are equal (returns true) if eventTypeId, deviceId and params match. */
bool EventDescriptor::operator ==(const Event &event) const
{
    if (m_eventTypeId != event.eventTypeId() || m_deviceId != event.deviceId()) {
        return false;
    }

    foreach (const ParamDescriptor &paramDescriptor, m_paramDescriptors) {
        switch (paramDescriptor.operatorType()) {
        case Types::ValueOperatorEquals:
            if (event.param(paramDescriptor.name()).value() != paramDescriptor.value()) {
                return false;
            }
            break;
        case Types::ValueOperatorNotEquals:
            if (event.param(paramDescriptor.name()).value() == paramDescriptor.value()) {
                return false;
            }
            break;
        case Types::ValueOperatorGreater:
            if (event.param(paramDescriptor.name()).value() <= paramDescriptor.value()) {
                return false;
            }
            break;
        case Types::ValueOperatorGreaterOrEqual:
            if (event.param(paramDescriptor.name()).value() < paramDescriptor.value()) {
                return false;
            }
            break;
        case Types::ValueOperatorLess:
            if (event.param(paramDescriptor.name()).value() >= paramDescriptor.value()) {
                return false;
            }
            break;
        case Types::ValueOperatorLessOrEqual:
            if (event.param(paramDescriptor.name()).value() < paramDescriptor.value()) {
                return false;
            }
            break;
        }
    }
    return true;
}

/*! Writes the eventTypeId and the deviceId of the given \a eventDescriptor to \a dbg. */
QDebug operator<<(QDebug dbg, const EventDescriptor &eventDescriptor)
{
    dbg.nospace() << "EventDescriptor(EventTypeId: " << eventDescriptor.eventTypeId().toString() << ", DeviceId" << eventDescriptor.deviceId() << ")";

    return dbg.space();
}

/*! Writes each \a eventDescriptors to \a dbg. */
QDebug operator<<(QDebug dbg, const QList<EventDescriptor> &eventDescriptors)
{
    dbg.nospace() << "EventDescriptorList (count:" << eventDescriptors.count() << ")";
    for (int i = 0; i < eventDescriptors.count(); i++ ) {
        dbg.nospace() << "     " << i << ": " << eventDescriptors.at(i);
    }

    return dbg.space();
}
