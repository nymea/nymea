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

#ifndef EVENTDESCRIPTOR_H
#define EVENTDESCRIPTOR_H

#include "libnymea.h"
#include "typeutils.h"
#include "paramdescriptor.h"
#include "event.h"

#include <QString>
#include <QVariantList>
#include <QDebug>

class LIBNYMEA_EXPORT EventDescriptor
{
    Q_GADGET
    Q_PROPERTY(QUuid deviceId READ deviceId WRITE setDeviceId USER true)
    Q_PROPERTY(QUuid eventTypeId READ eventTypeId WRITE setEventTypeId USER true)
    Q_PROPERTY(QString interface READ interface WRITE setInterface USER true)
    Q_PROPERTY(QString interfaceEvent READ interfaceEvent WRITE setInterfaceEvent USER true)
    Q_PROPERTY(ParamDescriptors paramDescriptors READ paramDescriptors WRITE setParamDescriptors USER true)
public:
    enum Type {
        TypeDevice,
        TypeInterface
    };

    EventDescriptor();
    EventDescriptor(const EventTypeId &eventTypeId, const DeviceId &deviceId, const QList<ParamDescriptor> &paramDescriptors = QList<ParamDescriptor>());
    EventDescriptor(const QString &interface, const QString &interfaceEvent, const QList<ParamDescriptor> &paramDescriptors = QList<ParamDescriptor>());

    Type type() const;
    bool isValid() const;

    EventTypeId eventTypeId() const;
    void setEventTypeId(const EventTypeId &eventTypeId);

    DeviceId deviceId() const;
    void setDeviceId(const DeviceId &deviceId);

    QString interface() const;
    void setInterface(const QString &interface);

    QString interfaceEvent() const;
    void setInterfaceEvent(const QString &interfaceEvent);

    QList<ParamDescriptor> paramDescriptors() const;
    void setParamDescriptors(const QList<ParamDescriptor> &paramDescriptors);
    ParamDescriptor paramDescriptor(const ParamTypeId &paramTypeId) const;

    bool operator ==(const EventDescriptor &other) const;

private:
    EventTypeId m_eventTypeId;
    DeviceId m_deviceId;
    QString m_interface;
    QString m_interfaceEvent;
    QList<ParamDescriptor> m_paramDescriptors;
};
Q_DECLARE_METATYPE(EventDescriptor)

class EventDescriptors: public QList<EventDescriptor>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    EventDescriptors();
    EventDescriptors(const QList<EventDescriptor> &other);
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
Q_DECLARE_METATYPE(EventDescriptors)


QDebug operator<<(QDebug dbg, const EventDescriptor &eventDescriptor);
QDebug operator<<(QDebug dbg, const QList<EventDescriptor> &eventDescriptors);

#endif // EVENTDESCRIPTOR_H
