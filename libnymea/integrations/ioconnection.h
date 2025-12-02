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

#ifndef IOCONNECTION_H
#define IOCONNECTION_H

#include <QObject>
#include <QList>
#include <QVariant>

#include "typeutils.h"
#include "thing.h"

struct IOConnectionResult {
    Thing::ThingError error = Thing::ThingErrorNoError;
    IOConnectionId ioConnectionId;
};

class IOConnection
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QUuid inputThingId READ inputThingId)
    Q_PROPERTY(QUuid inputStateTypeId READ inputStateTypeId)
    Q_PROPERTY(QUuid outputThingId READ outputThingId)
    Q_PROPERTY(QUuid outputStateTypeId READ outputStateTypeId)
    Q_PROPERTY(bool inverted READ inverted)

public:
    IOConnection();
    IOConnection(const IOConnectionId &id, const ThingId &inputThingId, const StateTypeId &inputStateTypeId, const ThingId &outputThingId, const StateTypeId &outputStateTypeId, bool inverted = false);

    IOConnectionId id() const;

    ThingId inputThingId() const;
    StateTypeId inputStateTypeId() const;

    ThingId outputThingId() const;
    StateTypeId outputStateTypeId() const;

    bool inverted() const;

private:
    IOConnectionId m_id;
    ThingId m_inputThingId;
    StateTypeId m_inputStateTypeId;
    ThingId m_outputThingId;
    StateTypeId m_outputStateTypeId;
    bool m_inverted = false;
};

class IOConnections: public QList<IOConnection>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    IOConnections() {}
    inline IOConnections(std::initializer_list<IOConnection> args): QList(args) {}
    IOConnections(const QList<IOConnection> &other): QList<IOConnection>(other) {}
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};

Q_DECLARE_METATYPE(IOConnection)
Q_DECLARE_METATYPE(IOConnections)


#endif // IOCONNECTION_H
