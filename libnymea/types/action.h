/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#ifndef ACTION_H
#define ACTION_H

#include "libnymea.h"
#include "typeutils.h"
#include "param.h"

#include <QVariantList>

class LIBNYMEA_EXPORT Action
{
    Q_GADGET
    Q_PROPERTY(QUuid actionTypeId READ actionTypeId WRITE setActionTypeId)
    Q_PROPERTY(QUuid deviceId READ deviceId WRITE setDeviceId)
    Q_PROPERTY(ParamList params READ params WRITE setParams USER true)

public:
    explicit Action(const ActionTypeId &actionTypeId = ActionTypeId(), const DeviceId &deviceId = DeviceId());
    Action(const Action &other);

    ActionId id() const;

    bool isValid() const;

    ActionTypeId actionTypeId() const;
    void setActionTypeId(const ActionTypeId &actionTypeId);
    DeviceId deviceId() const;
    void setDeviceId(const DeviceId &deviceId);

    ParamList params() const;
    void setParams(const ParamList &params);
    Param param(const ParamTypeId &paramTypeId) const;

    void operator=(const Action &other);
private:
    ActionId m_id;
    ActionTypeId m_actionTypeId;
    DeviceId m_deviceId;
    ParamList m_params;
};

#endif // ACTION_H
