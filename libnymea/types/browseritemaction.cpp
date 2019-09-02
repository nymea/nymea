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

#include "browseritemaction.h"


BrowserItemAction::BrowserItemAction(const DeviceId &deviceId, const QString &itemId, const ActionTypeId &actionTypeId, const ParamList &params):
    m_id(ActionId::createActionId()),
    m_deviceId(deviceId),
    m_itemId(itemId),
    m_actionTypeId(actionTypeId),
    m_params(params)
{

}

BrowserItemAction::BrowserItemAction(const BrowserItemAction &other):
    m_id(other.id()),
    m_deviceId(other.deviceId()),
    m_itemId(other.itemId()),
    m_actionTypeId(other.actionTypeId()),
    m_params(other.params())
{

}

ActionId BrowserItemAction::id() const
{
    return m_id;
}

bool BrowserItemAction::isValid() const
{
    return !m_id.isNull() && !m_deviceId.isNull() && !m_itemId.isNull();
}

DeviceId BrowserItemAction::deviceId() const
{
    return m_deviceId;
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
    m_id = other.id();
    m_deviceId = other.deviceId();
    m_itemId = other.itemId();
    m_actionTypeId = other.actionTypeId();
    m_params = other.params();
}
