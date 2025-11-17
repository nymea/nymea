// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

QString Package::summary() const
{
    return m_summary;
}

void Package::setSummary(const QString &summary)
{
    m_summary = summary;
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
            m_summary == other.summary() &&
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

Packages::Packages()
{

}

Packages::Packages(const QList<Package> &other): QList<Package>(other)
{

}

QVariant Packages::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void Packages::put(const QVariant &variant)
{
    append(variant.value<Package>());
}
