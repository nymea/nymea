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

#ifndef EVENT_H
#define EVENT_H

#include "typeutils.h"
#include "types/param.h"

#include <QString>
#include <QVariantList>
#include <QDebug>

class Event
{
public:
    Event();
    Event(const EventTypeId &eventTypeId, const DeviceId &deviceId, const QList<Param> &params = QList<Param>());

    EventId eventId() const;

    EventTypeId eventTypeId() const;
    void setEventTypeId(const EventTypeId &eventTypeId);

    DeviceId deviceId() const;
    void setDeviceId(const DeviceId &deviceId);

    QList<Param> params() const;
    void setParams(const QList<Param> &params);
    Param param(const QString &paramName) const;

    bool operator ==(const Event &other) const;

private:
    EventId m_id;
    EventTypeId m_eventTypeId;
    DeviceId m_deviceId;
    QList<Param> m_params;
};
Q_DECLARE_METATYPE(Event)
QDebug operator<<(QDebug dbg, const Event &event);
QDebug operator<<(QDebug dbg, const QList<Event> &events);

#endif // EVENT_H
