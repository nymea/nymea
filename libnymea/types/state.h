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

#ifndef STATE_H
#define STATE_H

#include "libnymea.h"
#include "typeutils.h"

#include <QDebug>
#include <QVariant>

class LIBNYMEA_EXPORT State
{
    Q_GADGET
    Q_PROPERTY(QUuid stateTypeId READ stateTypeId)
    Q_PROPERTY(QVariant value READ value)
    Q_PROPERTY(Types::StateValueFilter filter READ filter)
    Q_PROPERTY(QVariant minValue READ minValue USER true)
    Q_PROPERTY(QVariant maxValue READ maxValue USER true)
    Q_PROPERTY(QVariantList possibleValues READ possibleValues USER true)

public:
    State();
    State(const StateTypeId &stateTypeId, const ThingId &thingId);

    StateTypeId stateTypeId() const;
    ThingId thingId() const;

    QVariant value() const;

    QVariant minValue() const;
    QVariant maxValue() const;

    QVariantList possibleValues() const;

    Types::StateValueFilter filter() const;

private:
    friend class Thing;
    void setValue(const QVariant &value);
    void setMinValue(const QVariant &minValue);
    void setMaxValue(const QVariant &maxValue);
    void setPossibleValues(const QVariantList &values);
    void setFilter(Types::StateValueFilter filter);

private:
    StateTypeId m_stateTypeId;
    ThingId m_thingId;
    QVariant m_value;
    QVariant m_minValue;
    QVariant m_maxValue;
    QVariantList m_possibleValues;
    Types::StateValueFilter m_filter = Types::StateValueFilterNone;
};
Q_DECLARE_METATYPE(State)

class States : public QList<State>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    States();
    States(const QList<State> &other);
    States(std::initializer_list<State> args)
        : QList(args)
    {}
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
    Q_INVOKABLE QVariant stateValue(const StateTypeId &stateTypeId);
};
Q_DECLARE_METATYPE(States)

QDebug operator<<(QDebug dbg, const State &event);
QDebug operator<<(QDebug dbg, const QList<State> &events);

#endif // STATE_H
