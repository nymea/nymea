/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
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

#ifndef INTERFACE_H
#define INTERFACE_H

#include "eventtype.h"
#include "actiontype.h"
#include "statetype.h"

class Interface{
public:
    Interface() = default;
    Interface(const QString &name, const ActionTypes &actionTypes, const EventTypes &eventTypes, const StateTypes &stateTypes);

    QString name() const;

    ActionTypes actionTypes() const;
    EventTypes eventTypes() const;
    StateTypes stateTypes() const;

    bool isValid() const;
private:
    QUuid m_id;
    QString m_name;
    ActionTypes m_actionTypes;
    EventTypes m_eventTypes;
    StateTypes m_stateTypes;
};

class Interfaces: public QList<Interface>
{
public:
    Interfaces() = default;
    Interfaces(const QList<Interface> &other);
    Interface findByName(const QString &name);
};

#endif // INTERFACE_H
