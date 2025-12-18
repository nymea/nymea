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

#ifndef BROWSERITEMACTION_H
#define BROWSERITEMACTION_H

#include "types/param.h"
#include "typeutils.h"

class BrowserItemAction
{
public:
    explicit BrowserItemAction(const ThingId &thingId = ThingId(),
                               const QString &itemId = QString(),
                               const ActionTypeId &actionTypeId = ActionTypeId(),
                               const ParamList &params = ParamList());
    BrowserItemAction(const BrowserItemAction &other);

    bool isValid() const;

    ThingId thingId() const;
    QString itemId() const;
    ActionTypeId actionTypeId() const;

    ParamList params() const;
    void setParams(const ParamList &params);
    Param param(const ParamTypeId &paramTypeId) const;

    void operator=(const BrowserItemAction &other);

private:
    ThingId m_thingId;
    QString m_itemId;
    ActionTypeId m_actionTypeId;
    ParamList m_params;
};

#endif // BROWSERITEMACTION_H
