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

#include "interface.h"

Interface::Interface(const QString &name, const InterfaceParamTypes &paramTypes, const InterfaceActionTypes &actionTypes, const InterfaceEventTypes &eventTypes, const InterfaceStateTypes &stateTypes):
    m_name{name},
    m_paramTypes{paramTypes},
    m_actionTypes{actionTypes},
    m_eventTypes{eventTypes},
    m_stateTypes{stateTypes}
{

}

QString Interface::name() const
{
    return m_name;
}

InterfaceParamTypes Interface::paramTypes() const
{
    return m_paramTypes;
}

InterfaceActionTypes Interface::actionTypes() const
{
    return m_actionTypes;
}

InterfaceEventTypes Interface::eventTypes() const
{
    return m_eventTypes;
}

InterfaceStateTypes Interface::stateTypes() const
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
