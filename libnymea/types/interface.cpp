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
