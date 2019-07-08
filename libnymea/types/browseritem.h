#ifndef BROWSERITEM_H
#define BROWSERITEM_H

#include "libnymea.h"
#include "typeutils.h"

#include <QList>

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


    BrowserItem(const QString &id = QString(), const QString &displayName = QString(), bool browsable = false);

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
    bool m_executable = false;
    bool m_browsable = false;
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
