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

QString BrowserItem::thumbnail() const
{
    return m_thumbnail;
}

void BrowserItem::setThumbnail(const QString &thumbnail)
{
    m_thumbnail = thumbnail;
}

BrowserItems::BrowserItems()
{

}

BrowserItems::BrowserItems(const QList<BrowserItem> &other): QList<BrowserItem>(other)
{

}
