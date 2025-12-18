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

#ifndef BROWSERITEM_H
#define BROWSERITEM_H

#include "libnymea.h"
#include "typeutils.h"

#include <QHash>
#include <QList>
#include <QVariant>

class LIBNYMEA_EXPORT BrowserItem
{
    Q_GADGET
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QString displayName READ displayName)
    Q_PROPERTY(QString description READ description)
    Q_PROPERTY(BrowserIcon icon READ icon)
    Q_PROPERTY(QString thumbnail READ thumbnail)
    Q_PROPERTY(bool executable READ executable)
    Q_PROPERTY(bool browsable READ browsable)
    Q_PROPERTY(bool disabled READ disabled)
    Q_PROPERTY(QList<ActionTypeId> actionTypeIds READ actionTypeIds)

public:
    enum BrowserIcon {
        BrowserIconNone,
        BrowserIconFolder,
        BrowserIconFile,
        BrowserIconMusic,
        BrowserIconVideo,
        BrowserIconPictures,
        BrowserIconApplication,
        BrowserIconDocument,
        BrowserIconPackage,
        BrowserIconFavorites,
    };
    Q_ENUM(BrowserIcon)

    enum ExtendedProperties { ExtendedPropertiesNone = 0x00, ExtendedPropertiesMedia = 0x01 };
    Q_ENUM(ExtendedProperties)
    Q_DECLARE_FLAGS(ExtendedPropertiesFlags, ExtendedProperties)

    BrowserItem(const QString &id = QString(), const QString &displayName = QString(), bool browsable = false, bool executable = false);

    QString id() const;
    void setId(const QString &id);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    QString description() const;
    void setDescription(const QString &description);

    bool executable() const;
    void setExecutable(bool executable);

    bool browsable() const;
    void setBrowsable(bool browsable);

    bool disabled() const;
    void setDisabled(bool disabled);

    BrowserIcon icon() const;
    void setIcon(BrowserIcon icon);

    QString thumbnail() const;
    void setThumbnail(const QString &thumbnail);

    QList<ActionTypeId> actionTypeIds() const;
    void setActionTypeIds(const QList<ActionTypeId> &actionTypeIds);

    ExtendedPropertiesFlags extendedPropertiesFlags() const;
    QVariant extendedProperty(const QString &propertyName) const;

private:
    QString m_id;
    QString m_displayName;
    QString m_description;
    bool m_browsable = false;
    bool m_executable = false;
    bool m_disabled = false;
    BrowserIcon m_icon = BrowserIconNone;
    QString m_thumbnail;

protected:
    ExtendedPropertiesFlags m_extendedPropertiesFlags = ExtendedPropertiesNone;
    QHash<QString, QVariant> m_extendedProperties;
    QList<ActionTypeId> m_actionTypeIds;
};

Q_DECLARE_METATYPE(BrowserItem)
Q_DECLARE_OPERATORS_FOR_FLAGS(BrowserItem::ExtendedPropertiesFlags)

class LIBNYMEA_EXPORT BrowserItems : public QList<BrowserItem>
{
public:
    BrowserItems();
    BrowserItems(const QList<BrowserItem> &other);
};

#endif // BROWSERITEM_H
