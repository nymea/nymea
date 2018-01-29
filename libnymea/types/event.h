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

#ifndef EVENT_H
#define EVENT_H

#include "libguh.h"
#include "typeutils.h"
#include "types/param.h"

#include <QString>
#include <QVariantList>
#include <QDebug>

class LIBGUH_EXPORT Event
{
public:
    Event();
    Event(const EventTypeId &eventTypeId, const DeviceId &deviceId, const ParamList &params = ParamList(), bool isStateChangeEvent = false);

    EventId eventId() const;

    EventTypeId eventTypeId() const;
    void setEventTypeId(const EventTypeId &eventTypeId);

    DeviceId deviceId() const;
    void setDeviceId(const DeviceId &deviceId);

    ParamList params() const;
    void setParams(const ParamList &params);
    Param param(const ParamTypeId &paramTypeId) const;

    bool operator ==(const Event &other) const;

    bool isStateChangeEvent() const;

private:
    EventId m_id;
    EventTypeId m_eventTypeId;
    DeviceId m_deviceId;
    ParamList m_params;

    bool m_isStateChangeEvent;
};
Q_DECLARE_METATYPE(Event)
QDebug operator<<(QDebug dbg, const Event &event);
QDebug operator<<(QDebug dbg, const QList<Event> &events);

#endif // EVENT_H
