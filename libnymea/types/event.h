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

#ifndef EVENT_H
#define EVENT_H

#include "libnymea.h"
#include "typeutils.h"
#include "types/param.h"

#include <QString>
#include <QVariantList>
#include <QDebug>

class LIBNYMEA_EXPORT Event
{
    Q_GADGET
    Q_PROPERTY(QUuid eventTypeId READ eventTypeId)
    Q_PROPERTY(QUuid thingId READ thingId)
    Q_PROPERTY(ParamList params READ params)
public:
    Event();
    Event(const EventTypeId &eventTypeId, const ThingId &thingId, const ParamList &params = ParamList());

    EventTypeId eventTypeId() const;
    void setEventTypeId(const EventTypeId &eventTypeId);

    ThingId thingId() const;
    void setThingId(const ThingId &thingId);

    ParamList params() const;
    void setParams(const ParamList &params);
    Param param(const ParamTypeId &paramTypeId) const;
    QVariant paramValue(const ParamTypeId &paramTypeId) const;

    bool operator ==(const Event &other) const;

private:
    EventTypeId m_eventTypeId;
    ThingId m_thingId;
    ParamList m_params;
};
Q_DECLARE_METATYPE(Event)
QDebug operator<<(QDebug dbg, const Event &event);
QDebug operator<<(QDebug dbg, const QList<Event> &events);

#endif // EVENT_H
