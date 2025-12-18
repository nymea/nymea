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

#include "interfacestatetype.h"

InterfaceStateType::InterfaceStateType() {}

bool InterfaceStateType::optional() const
{
    return m_optional;
}

void InterfaceStateType::setOptional(bool optional)
{
    m_optional = optional;
}

bool InterfaceStateType::loggingOverride() const
{
    return m_loggingOverride;
}

void InterfaceStateType::setLoggingOverride(bool loggingOverride)
{
    m_loggingOverride = loggingOverride;
}

InterfaceStateTypes::InterfaceStateTypes(const QList<InterfaceStateType> &other)
    : QList<InterfaceStateType>(other)
{}

InterfaceStateType InterfaceStateTypes::findByName(const QString &name)
{
    foreach (const InterfaceStateType &ist, *this) {
        if (ist.name() == name) {
            return ist;
        }
    }
    return InterfaceStateType();
}
