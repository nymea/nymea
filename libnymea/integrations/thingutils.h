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

#ifndef THINGUTILS_H
#define THINGUTILS_H

#include "pluginmetadata.h"
#include "thing.h"

#include "types/interface.h"
#include "types/paramtype.h"

class ThingUtils
{
public:
    ThingUtils();

    static Thing::ThingError verifyParams(const QList<ParamType> paramTypes, const ParamList &params);
    static Thing::ThingError verifyParam(const QList<ParamType> paramTypes, const Param &param);
    static Thing::ThingError verifyParam(const ParamType &paramType, const Param &param);

    static Interfaces allInterfaces();
    static Interface loadInterface(const QString &name);
    static Interface mergeInterfaces(const Interface &iface1, const Interface &iface2);
    static QStringList generateInterfaceParentList(const QString &interface);

    static bool variantLessThan(const QVariant &leftHandSide, const QVariant &rightHandSide);
    static bool variantGreaterThan(const QVariant &leftHandSide, const QVariant &rightHandSide);
};

#endif // THINGUTILS_H
