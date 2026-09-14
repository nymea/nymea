// SPDX-License-Identifier: LGPL-3.0-or-later

#include "userinventoryitem.h"

namespace nymeaserver {

UserInventoryItem::UserInventoryItem()
{

}

UserInventoryItem::UserInventoryItem(const QUuid &inventoryItemId):
    m_inventoryItemId(inventoryItemId)
{

}

bool UserInventoryItem::isValid() const
{
    return !m_inventoryItemId.isNull();
}

QUuid UserInventoryItem::inventoryItemId() const
{
    return m_inventoryItemId;
}

void UserInventoryItem::setInventoryItemId(const QUuid &inventoryItemId)
{
    m_inventoryItemId = inventoryItemId;
}

QString UserInventoryItem::username() const
{
    return m_username;
}

void UserInventoryItem::setUsername(const QString &username)
{
    m_username = username;
}

QString UserInventoryItem::type() const
{
    return m_type;
}

void UserInventoryItem::setType(const QString &type)
{
    m_type = type;
}

QString UserInventoryItem::displayName() const
{
    return m_displayName;
}

void UserInventoryItem::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

bool UserInventoryItem::enabled() const
{
    return m_enabled;
}

void UserInventoryItem::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

QVariantMap UserInventoryItem::payload() const
{
    return m_payload;
}

void UserInventoryItem::setPayload(const QVariantMap &payload)
{
    m_payload = payload;
}

QVariant UserInventoryItems::get(int index) const
{
    if (index < 0 || index >= count())
        return QVariant();

    return QVariant::fromValue(at(index));
}

void UserInventoryItems::put(const QVariant &variant)
{
    append(variant.value<UserInventoryItem>());
}

}
