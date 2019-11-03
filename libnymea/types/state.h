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

#ifndef STATE_H
#define STATE_H

#include "libnymea.h"
#include "typeutils.h"

#include <QVariant>
#include <QDebug>

class LIBNYMEA_EXPORT State
{
    Q_GADGET
    Q_PROPERTY(QUuid stateTypeId READ stateTypeId)
    Q_PROPERTY(QVariant value READ value)

public:
    State();
    State(const StateTypeId &stateTypeId, const DeviceId &deviceId);

    StateId id() const;

    StateTypeId stateTypeId() const;
    DeviceId deviceId() const;

    QVariant value() const;
    void setValue(const QVariant &value);

private:
    StateId m_id;
    StateTypeId m_stateTypeId;
    DeviceId m_deviceId;
    QVariant m_value;
};
Q_DECLARE_METATYPE(State)

class States: public QList<State>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    States();
    States(const QList<State> &other);
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
Q_DECLARE_METATYPE(States)

QDebug operator<<(QDebug dbg, const State &event);
QDebug operator<<(QDebug dbg, const QList<State> &events);

#endif // STATE_H
