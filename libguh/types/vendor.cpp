#include "vendor.h"

Vendor::Vendor(const QUuid &id, const QString &name):
    m_id(id),
    m_name(name)
{
}

QUuid Vendor::id() const
{
    return m_id;
}

void Vendor::setId(const QUuid &id)
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
