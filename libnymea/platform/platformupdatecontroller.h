// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2026, chargebyte austria GmbH
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

#ifndef PLATFORMUPDATECONTROLLER_H
#define PLATFORMUPDATECONTROLLER_H

#include "package.h"
#include "repository.h"

#include <QObject>

class PlatformUpdateController : public QObject
{
    Q_OBJECT
public:
    enum UpdateType {
        UpdateTypeNone,
        UpdateTypeSystem,
        UpdateTypePackageManager
    };
    Q_ENUM(UpdateType)

    explicit PlatformUpdateController(QObject *parent = nullptr);
    virtual ~PlatformUpdateController() = default;

    virtual PlatformUpdateController::UpdateType updateType() const;

    virtual bool updateManagementAvailable() const;

    virtual bool checkForUpdates();
    virtual bool busy() const;
    virtual bool updateRunning() const;
    virtual int updateProgress() const;

    virtual QList<Package> packages() const;
    virtual QList<Repository> repositories() const;

    virtual bool startUpdate(const QStringList &packageIds = QStringList());
    virtual bool rollback(const QStringList &packageIds);
    virtual bool removePackages(const QStringList &packageIds);

    virtual bool enableRepository(const QString &repositoryId, bool enabled);

signals:
    void availableChanged();
    void busyChanged();
    void updateRunningChanged();
    void updateProgressChanged();
    void packageAdded(const Package &pacakge);
    void packageChanged(const Package &package);
    void packageRemoved(const QString &packageId);
    void repositoryAdded(const Repository &repository);
    void repositoryChanged(const Repository &repository);
    void repositoryRemoved(const QString &repositoryId);
};

Q_DECLARE_INTERFACE(PlatformUpdateController, "io.nymea.PlatformUpdateController")

#endif // PLATFORMUPDATECONTROLLER_H
