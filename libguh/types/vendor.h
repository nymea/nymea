#ifndef VENDOR_H
#define VENDOR_H

#include <QUuid>
#include <QString>

class Vendor
{
public:
    Vendor(const QUuid &id, const QString &name = QString());

    QUuid id() const;
    void setId(const QUuid &id);

    QString name() const;
    void setName(const QString &name);

private:
    QUuid m_id;
    QString m_name;
};

#endif // VENDOR_H
