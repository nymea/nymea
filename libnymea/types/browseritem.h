#ifndef BROWSERITEM_H
#define BROWSERITEM_H

#include "libnymea.h"
#include "typeutils.h"

#include <QList>

class LIBNYMEA_EXPORT BrowserItem
{
public:
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

    QString thumbnail() const;
    void setThumbnail(const QString &thumbnail);

private:
    QString m_id;
    QString m_displayName;
    QString m_description;
    bool m_executable = false;
    bool m_browsable = false;
    QString m_thumbnail;
};


class LIBNYMEA_EXPORT BrowserItems: public QList<BrowserItem>
{
public:
    BrowserItems();
    BrowserItems(const QList<BrowserItem> &other);

};

#endif // BROWSERITEM_H
