#ifndef VENDOR_H
#define VENDOR_H

#include "typeutils.h"

#include <QString>

class Vendor
{
public:
    Vendor(const VendorId &id, const QString &name = QString());

    VendorId id() const;
    void setId(const VendorId &id);

    QString name() const;
    void setName(const QString &name);

private:
    VendorId m_id;
    QString m_name;
};

#endif // VENDOR_H
