/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "browseraction.h"

BrowserAction::BrowserAction(const ThingId &thingId, const QString &itemId):
    m_id(ActionId::createActionId()),
    m_thingId(thingId),
    m_itemId(itemId)
{

}

BrowserAction::BrowserAction(const BrowserAction &other):
    m_id(other.id()),
    m_thingId(other.thingId()),
    m_itemId(other.itemId())
{

}

ActionId BrowserAction::id() const
{
    return m_id;
}

bool BrowserAction::isValid() const
{
    return !m_id.isNull() && !m_thingId.isNull() && !m_itemId.isNull();
}

ThingId BrowserAction::thingId() const
{
    return m_thingId;
}

QString BrowserAction::itemId() const
{
    return m_itemId;
}

void BrowserAction::operator=(const BrowserAction &other)
{
    m_id = other.id();
    m_thingId = other.thingId();
    m_itemId = other.itemId();
}
