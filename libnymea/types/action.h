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

#ifndef ACTION_H
#define ACTION_H

#include "libnymea.h"
#include "param.h"
#include "typeutils.h"

#include <QVariantList>

class LIBNYMEA_EXPORT Action
{
    Q_GADGET
    Q_PROPERTY(QUuid actionTypeId READ actionTypeId WRITE setActionTypeId)
    Q_PROPERTY(QUuid thingId READ thingId WRITE setThingId)
    Q_PROPERTY(ParamList params READ params WRITE setParams USER true)

public:
    enum TriggeredBy { TriggeredByUser, TriggeredByRule, TriggeredByScript };
    Q_ENUM(TriggeredBy)

    explicit Action(const ActionTypeId &actionTypeId = ActionTypeId(), const ThingId &thingId = ThingId(), TriggeredBy triggeredBy = TriggeredByUser);
    Action(const Action &other);

    bool isValid() const;

    ActionTypeId actionTypeId() const;
    void setActionTypeId(const ActionTypeId &actionTypeId);
    ThingId thingId() const;
    void setThingId(const ThingId &thingId);

    ParamList params() const;
    void setParams(const ParamList &params);
    Param param(const ParamTypeId &paramTypeId) const;
    QVariant paramValue(const ParamTypeId &paramTypeId) const;

    TriggeredBy triggeredBy() const;

    void operator=(const Action &other);

private:
    ActionTypeId m_actionTypeId;
    ThingId m_thingId;
    ParamList m_params;
    TriggeredBy m_triggeredBy = TriggeredByUser;
};

Q_DECLARE_METATYPE(Action::TriggeredBy)

#endif // ACTION_H
