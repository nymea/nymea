/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
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

#ifndef DEVICEUTILS_H
#define DEVICEUTILS_H

#include "device.h"
#include "pluginmetadata.h"

#include "types/paramtype.h"
#include "types/interface.h"

class DeviceUtils
{
public:
    DeviceUtils();

    static Device::DeviceError verifyParams(const QList<ParamType> paramTypes, ParamList &params, bool requireAll = true);
    static Device::DeviceError verifyParam(const QList<ParamType> paramTypes, const Param &param);
    static Device::DeviceError verifyParam(const ParamType &paramType, const Param &param);

    static Interfaces allInterfaces();
    static Interface loadInterface(const QString &name);
    static Interface mergeInterfaces(const Interface &iface1, const Interface &iface2);
    static QStringList generateInterfaceParentList(const QString &interface);

};

#endif // DEVICEUTILS_H
