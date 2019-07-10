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

#include "browseraction.h"

BrowserAction::BrowserAction(const DeviceId &deviceId, const QString &itemId):
    m_id(ActionId::createActionId()),
    m_deviceId(deviceId),
    m_itemId(itemId)
{

}

BrowserAction::BrowserAction(const BrowserAction &other):
    m_id(other.id()),
    m_deviceId(other.deviceId()),
    m_itemId(other.itemId())
{

}

ActionId BrowserAction::id() const
{
    return m_id;
}

bool BrowserAction::isValid() const
{
    return !m_id.isNull() && !m_deviceId.isNull() && !m_itemId.isNull();
}

DeviceId BrowserAction::deviceId() const
{
    return m_deviceId;
}

QString BrowserAction::itemId() const
{
    return m_itemId;
}

void BrowserAction::operator=(const BrowserAction &other)
{
    m_id = other.id();
    m_deviceId = other.deviceId();
    m_itemId = other.itemId();
}
