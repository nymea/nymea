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

#ifndef STATE_H
#define STATE_H

#include "libnymea.h"
#include "typeutils.h"

#include <QVariant>
#include <QDebug>

class LIBNYMEA_EXPORT State
{
    Q_GADGET
    Q_PROPERTY(QUuid stateTypeId READ stateTypeId USER true REVISION 1)
    Q_PROPERTY(QUuid thingId READ thingId)
    Q_PROPERTY(QString name READ name USER true) // TODO: Make mandatory when stateTypeId is removed
    Q_PROPERTY(QVariant value READ value)
    Q_PROPERTY(Types::StateValueFilter filter READ filter)
    Q_PROPERTY(QVariant minValue READ minValue USER true)
    Q_PROPERTY(QVariant maxValue READ maxValue USER true)

public:
    State();
    State(const StateTypeId &stateTypeId, const ThingId &thingId, const QString &name);

    StateTypeId stateTypeId() const;
    ThingId thingId() const;
    QString name() const;

    QVariant value() const;

    QVariant minValue() const;
    QVariant maxValue() const;

    Types::StateValueFilter filter() const;

private:
    friend class Thing;
    void setValue(const QVariant &value);
    void setMinValue(const QVariant &minValue);
    void setMaxValue(const QVariant &maxValue);
    void setFilter(Types::StateValueFilter filter);

private:
    StateTypeId m_stateTypeId;
    ThingId m_thingId;
    QString m_name;
    QVariant m_value;
    QVariant m_minValue;
    QVariant m_maxValue;
    Types::StateValueFilter m_filter = Types::StateValueFilterNone;
};
Q_DECLARE_METATYPE(State)

class States: public QList<State>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    States();
    States(const QList<State> &other);
    States(std::initializer_list<State> args):QList(args) {}
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
    Q_INVOKABLE QVariant stateValue(const StateTypeId &stateTypeId);
};
Q_DECLARE_METATYPE(States)

QDebug operator<<(QDebug dbg, const State &event);
QDebug operator<<(QDebug dbg, const QList<State> &events);

#endif // STATE_H
