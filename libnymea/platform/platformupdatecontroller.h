/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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
    explicit PlatformUpdateController(QObject *parent = nullptr);
    virtual ~PlatformUpdateController() = default;

    virtual bool updateManagementAvailable() const;

    virtual bool checkForUpdates();
    virtual bool busy() const;
    virtual bool updateRunning() const;

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
    void packageAdded(const Package &pacakge);
    void packageChanged(const Package &package);
    void packageRemoved(const QString &packageId);
    void repositoryAdded(const Repository &repository);
    void repositoryChanged(const Repository &repository);
    void repositoryRemoved(const QString &repositoryId);
};

Q_DECLARE_INTERFACE(PlatformUpdateController, "io.nymea.PlatformUpdateController")

#endif // PLATFORMUPDATECONTROLLER_H
