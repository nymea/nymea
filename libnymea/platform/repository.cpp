#include "repository.h"

Repository::Repository(const QString &id, const QString &displayName, bool enabled):
    m_id(id),
    m_displayName(displayName),
    m_enabled(enabled)
{

}

Repository::Repository()
{

}

QString Repository::id() const
{
    return m_id;
}

QString Repository::displayName() const
{
    return m_displayName;
}

bool Repository::enabled() const
{
    return m_enabled;
}

void Repository::setEnabled(bool enabled)
{
    m_enabled = enabled;
}
