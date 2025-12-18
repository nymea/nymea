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

#ifndef INTERFACEACTIONTYPE_H
#define INTERFACEACTIONTYPE_H

#include "actiontype.h"

class InterfaceActionType : public ActionType
{
public:
    InterfaceActionType();

    bool optional() const;
    void setOptional(bool optional);

private:
    bool m_optional = false;
};

class InterfaceActionTypes : public QList<InterfaceActionType>
{
public:
    InterfaceActionTypes() = default;
    InterfaceActionTypes(const QList<InterfaceActionType> &other);
    InterfaceActionType findByName(const QString &name);
};

#endif // INTERFACEACTIONTYPE_H
