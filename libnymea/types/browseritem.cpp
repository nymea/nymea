#include "browseritem.h"


BrowserItem::BrowserItem(const QString &id, const QString &displayName, bool browsable):
    m_id(id),
    m_displayName(displayName),
    m_browsable(browsable)
{

}

QString BrowserItem::id() const
{
    return m_id;
}

QString BrowserItem::displayName() const
{
    return m_displayName;
}

void BrowserItem::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

QString BrowserItem::description() const
{
    return m_description;
}

void BrowserItem::setDescription(const QString &description)
{
    m_description = description;
}

bool BrowserItem::executable() const
{
    return m_executable;
}

void BrowserItem::setExecutable(bool executable)
{
    m_executable = executable;
}

bool BrowserItem::browsable() const
{
    return m_browsable;
}

void BrowserItem::setBrowsable(bool browsable)
{
    m_browsable = browsable;
}

BrowserItem::BrowserIcon BrowserItem::icon() const
{
    return m_icon;
}

void BrowserItem::setIcon(BrowserIcon icon)
{
    m_icon = icon;
}

QString BrowserItem::thumbnail() const
{
    return m_thumbnail;
}

void BrowserItem::setThumbnail(const QString &thumbnail)
{
    m_thumbnail = thumbnail;
}

BrowserItem::ExtendedPropertiesFlags BrowserItem::extendedPropertiesFlags() const
{
    return m_extendedPropertiesFlags;
}

QVariant BrowserItem::extendedProperty(const QString &propertyName) const
{
    return m_extendedProperties[propertyName];
}

BrowserItems::BrowserItems()
{

}

BrowserItems::BrowserItems(const QList<BrowserItem> &other): QList<BrowserItem>(other)
{

}
