/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef ACTION_H
#define ACTION_H

#include "libguh.h"
#include "typeutils.h"
#include "param.h"

#include <QVariantList>

class LIBGUH_EXPORT Action
{
public:
    explicit Action(const ActionTypeId &actionTypeId = ActionTypeId(), const DeviceId &deviceId = DeviceId());
    Action(const Action &other);

    ActionId id() const;

    bool isValid() const;

    ActionTypeId actionTypeId() const;
    DeviceId deviceId() const;

    ParamList params() const;
    void setParams(const ParamList &params);
    Param param(const QString &paramName) const;

    void operator=(const Action &other);
private:
    ActionId m_id;
    ActionTypeId m_actionTypeId;
    DeviceId m_deviceId;
    ParamList m_params;
};

#endif // ACTION_H
