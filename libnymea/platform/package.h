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

#ifndef PACKAGE_H
#define PACKAGE_H

#include <QString>
#include <QMetaObject>
#include <QList>
#include <QVariant>

class Package
{
    Q_GADGET
    Q_PROPERTY(QString id READ packageId)
    Q_PROPERTY(QString displayName READ displayName)
    Q_PROPERTY(QString summary READ summary)
    Q_PROPERTY(QString installedVersion READ installedVersion)
    Q_PROPERTY(QString candidateVersion READ candidateVersion)
    Q_PROPERTY(QString changelog READ changelog)
    Q_PROPERTY(bool updateAvailable READ updateAvailable)
    Q_PROPERTY(bool rollbackAvailable READ rollbackAvailable)
    Q_PROPERTY(bool canRemove READ canRemove)

public:
    explicit Package(const QString &packageId = QString(), const QString &displayName = QString(), const QString &installedVersion = QString(), const QString &candidateVersion = QString(), const QString &changelog = QString());

    QString packageId() const;
    QString displayName() const;

    QString summary() const;
    void setSummary(const QString &summary);

    QString installedVersion() const;
    void setInstalledVersion(const QString &installedVersion);

    QString candidateVersion() const;
    void setCandidateVersion(const QString &candidateVersion);

    QString changelog() const;
    void setChangelog(const QString &changelog);

    bool updateAvailable() const;
    void setUpdateAvailable(bool updateAvailable);

    bool rollbackAvailable() const;
    void setRollbackAvailable(bool rollbackAvailable);

    bool canRemove() const;
    void setCanRemove(bool canRemove);

    bool operator==(const Package &other) const;
    bool operator!=(const Package &other) const;

private:
    QString m_packageId;
    QString m_displayName;
    QString m_summary;
    QString m_installedVersion;
    QString m_candidateVersion;
    QString m_changeLog;

    bool m_updateAvailable = false;
    bool m_rollbackAvailable = false;
    bool m_canRemove = false;
};
Q_DECLARE_METATYPE(Package)

class Packages: public QList<Package>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    Packages();
    Packages(const QList<Package> &other);
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
Q_DECLARE_METATYPE(Packages)

#endif // PACKAGE_H
