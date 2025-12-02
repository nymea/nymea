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

#ifndef INTERFACE_H
#define INTERFACE_H

#include "interfaceparamtype.h"
#include "interfaceeventtype.h"
#include "interfaceactiontype.h"
#include "interfacestatetype.h"

class Interface{
public:
    Interface() = default;
    Interface(const QString &name, const InterfaceParamTypes &paramTypes, const InterfaceActionTypes &actionTypes, const InterfaceEventTypes &eventTypes, const InterfaceStateTypes &stateTypes);

    QString name() const;

    InterfaceParamTypes paramTypes() const;
    InterfaceActionTypes actionTypes() const;
    InterfaceEventTypes eventTypes() const;
    InterfaceStateTypes stateTypes() const;

    bool isValid() const;

private:
    QString m_name;
    InterfaceParamTypes m_paramTypes;
    InterfaceActionTypes m_actionTypes;
    InterfaceEventTypes m_eventTypes;
    InterfaceStateTypes m_stateTypes;
    bool m_optional = false;
};

class LIBNYMEA_EXPORT Interfaces: public QList<Interface>
{
public:
    Interfaces() = default;
    Interfaces(const QList<Interface> &other);
    Interfaces(std::initializer_list<Interface> args):QList(args) {}
    Interface findByName(const QString &name);
};

#endif // INTERFACE_H
