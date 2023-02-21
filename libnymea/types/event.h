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
    Q_PROPERTY(QUuid eventTypeId READ eventTypeId USER true REVISION 1)
    Q_PROPERTY(QUuid thingId READ thingId)
    Q_PROPERTY(QString name READ name USER true) // TODO: Make mandatory when eventTypeId is removed
    Q_PROPERTY(ParamList params READ params)
public:
    Event();
    Event(const EventTypeId &eventTypeId, const ThingId &thingId, const ParamList &params = ParamList());
    Event(const ThingId &thingId, const QString &name, const ParamList &params = ParamList());

    EventTypeId eventTypeId() const;
    void setEventTypeId(const EventTypeId &eventTypeId);

    ThingId thingId() const;
    void setThingId(const ThingId &thingId);

    QString name() const;
    void setName(const QString &name);

    ParamList params() const;
    void setParams(const ParamList &params);
    Param param(const ParamTypeId &paramTypeId) const;
    QVariant paramValue(const ParamTypeId &paramTypeId) const;

    bool operator ==(const Event &other) const;

private:
    EventTypeId m_eventTypeId;
    ThingId m_thingId;
    QString m_name;
    ParamList m_params;
};
Q_DECLARE_METATYPE(Event)
QDebug operator<<(QDebug dbg, const Event &event);
QDebug operator<<(QDebug dbg, const QList<Event> &events);

#endif // EVENT_H
