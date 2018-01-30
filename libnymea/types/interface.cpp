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

#include "interface.h"

Interface::Interface(const QString &name, const ActionTypes &actionTypes, const EventTypes &eventTypes, const StateTypes &stateTypes):
    m_name(name),
    m_actionTypes(actionTypes),
    m_eventTypes(eventTypes),
    m_stateTypes(stateTypes)
{

}

QString Interface::name() const
{
    return m_name;
}

ActionTypes Interface::actionTypes() const
{
    return m_actionTypes;
}

EventTypes Interface::eventTypes() const
{
    return m_eventTypes;
}

StateTypes Interface::stateTypes() const
{
    return m_stateTypes;
}

bool Interface::isValid() const
{
    return !m_name.isEmpty();
}

Interfaces::Interfaces(const QList<Interface> &other)
{
    foreach (const Interface &iface, other) {
        append(iface);
    }
}

Interface Interfaces::findByName(const QString &name)
{
    foreach (const Interface &interface, *this) {
        if (interface.name() == name) {
            return interface;
        }
    }
    return Interface();
}
