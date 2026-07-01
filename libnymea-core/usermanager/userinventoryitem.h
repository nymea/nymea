// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef USERINVENTORYITEM_H
#define USERINVENTORYITEM_H

#include <QObject>
#include <QUuid>
#include <QVariant>

namespace nymeaserver {

class UserInventoryItem
{
    Q_GADGET
    Q_PROPERTY(QUuid inventoryItemId READ inventoryItemId)
    Q_PROPERTY(QString username READ username)
    Q_PROPERTY(QString type READ type)
    Q_PROPERTY(QString displayName READ displayName)
    Q_PROPERTY(bool enabled READ enabled)
    Q_PROPERTY(QVariantMap payload READ payload)

public:
    UserInventoryItem();
    explicit UserInventoryItem(const QUuid &inventoryItemId);

    bool isValid() const;

    QUuid inventoryItemId() const;
    void setInventoryItemId(const QUuid &inventoryItemId);

    QString username() const;
    void setUsername(const QString &username);

    QString type() const;
    void setType(const QString &type);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    bool enabled() const;
    void setEnabled(bool enabled);

    QVariantMap payload() const;
    void setPayload(const QVariantMap &payload);

private:
    QUuid m_inventoryItemId;
    QString m_username;
    QString m_type;
    QString m_displayName;
    bool m_enabled = false;
    QVariantMap m_payload;
};

class UserInventoryItems: public QList<UserInventoryItem>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};

}

Q_DECLARE_METATYPE(nymeaserver::UserInventoryItem)
Q_DECLARE_METATYPE(nymeaserver::UserInventoryItems)

#endif // USERINVENTORYITEM_H
