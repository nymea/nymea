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

#ifndef BROWSERITEM_H
#define BROWSERITEM_H

#include "libnymea.h"
#include "typeutils.h"

#include <QList>
#include <QHash>
#include <QVariant>

class LIBNYMEA_EXPORT BrowserItem
{
    Q_GADGET
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

    enum ExtendedProperties {
        ExtendedPropertiesNone = 0x00,
        ExtendedPropertiesMedia = 0x01
    };
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

    BrowserIcon icon() const;
    void setIcon(BrowserIcon icon);

    QString thumbnail() const;
    void setThumbnail(const QString &thumbnail);

    ExtendedPropertiesFlags extendedPropertiesFlags() const;
    QVariant extendedProperty(const QString &propertyName) const;

private:
    QString m_id;
    QString m_displayName;
    QString m_description;
    bool m_browsable = false;
    bool m_executable = false;
    BrowserIcon m_icon = BrowserIconNone;
    QString m_thumbnail;

protected:
    ExtendedPropertiesFlags m_extendedPropertiesFlags = ExtendedPropertiesNone;
    QHash<QString, QVariant> m_extendedProperties;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(BrowserItem::ExtendedPropertiesFlags)


class LIBNYMEA_EXPORT BrowserItems: public QList<BrowserItem>
{
public:
    BrowserItems();
    BrowserItems(const QList<BrowserItem> &other);

};

#endif // BROWSERITEM_H
