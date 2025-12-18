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

#include "browseritemaction.h"

BrowserItemAction::BrowserItemAction(const ThingId &thingId, const QString &itemId, const ActionTypeId &actionTypeId, const ParamList &params)
    : m_thingId(thingId)
    , m_itemId(itemId)
    , m_actionTypeId(actionTypeId)
    , m_params(params)
{}

BrowserItemAction::BrowserItemAction(const BrowserItemAction &other)
    : m_thingId(other.thingId())
    , m_itemId(other.itemId())
    , m_actionTypeId(other.actionTypeId())
    , m_params(other.params())
{}

bool BrowserItemAction::isValid() const
{
    return !m_thingId.isNull() && !m_itemId.isNull();
}

ThingId BrowserItemAction::thingId() const
{
    return m_thingId;
}

QString BrowserItemAction::itemId() const
{
    return m_itemId;
}

ActionTypeId BrowserItemAction::actionTypeId() const
{
    return m_actionTypeId;
}

ParamList BrowserItemAction::params() const
{
    return m_params;
}

void BrowserItemAction::operator=(const BrowserItemAction &other)
{
    m_thingId = other.thingId();
    m_itemId = other.itemId();
    m_actionTypeId = other.actionTypeId();
    m_params = other.params();
}
