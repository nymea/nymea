#include "package.h"

Package::Package(const QString &packageId, const QString &displayName, const QString &installedVersion, const QString &candidateVersion, const QString &changelog):
    m_packageId(packageId),
    m_displayName(displayName),
    m_installedVersion(installedVersion),
    m_candidateVersion(candidateVersion),
    m_changeLog(changelog)
{

}

QString Package::packageId() const
{
    return m_packageId;
}

QString Package::displayName() const
{
    return m_displayName;
}

QString Package::installedVersion() const
{
    return m_installedVersion;
}

void Package::setInstalledVersion(const QString &installedVersion)
{
    m_installedVersion = installedVersion;
}

QString Package::candidateVersion() const
{
    return m_candidateVersion;
}

void Package::setCandidateVersion(const QString &candidateVersion)
{
    m_candidateVersion = candidateVersion;
}

QString Package::changelog() const
{
    return m_changeLog;
}

void Package::setChangelog(const QString &changelog)
{
    m_changeLog = changelog;
}

bool Package::updateAvailable() const
{
    return m_updateAvailable;
}

void Package::setUpdateAvailable(bool updateAvailable)
{
    m_updateAvailable = updateAvailable;
}

bool Package::rollbackAvailable() const
{
    return m_rollbackAvailable;
}

void Package::setRollbackAvailable(bool rollbackAvailable)
{
    m_rollbackAvailable = rollbackAvailable;
}

bool Package::canRemove() const
{
    return m_canRemove;
}

void Package::setCanRemove(bool canRemove)
{
    m_canRemove = canRemove;
}

bool Package::operator==(const Package &other) const
{
    return m_packageId == other.packageId() &&
            m_displayName == other.displayName() &&
            m_installedVersion == other.installedVersion() &&
            m_candidateVersion == other.candidateVersion() &&
            m_changeLog == other.changelog() &&
            m_updateAvailable == other.updateAvailable() &&
            m_rollbackAvailable == other.rollbackAvailable() &&
            m_canRemove == other.canRemove();
}

bool Package::operator!=(const Package &other) const
{
    return !operator==(other);
}
