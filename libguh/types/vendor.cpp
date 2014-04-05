#include "vendor.h"

Vendor::Vendor(const VendorId &id, const QString &name):
    m_id(id),
    m_name(name)
{
}

VendorId Vendor::id() const
{
    return m_id;
}

void Vendor::setId(const VendorId &id)
{
    m_id = id;
}

QString Vendor::name() const
{
    return m_name;
}

void Vendor::setName(const QString &name)
{
    m_name = name;
}
