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

#include "platformupdatecontroller.h"

PlatformUpdateController::PlatformUpdateController(QObject *parent) : QObject(parent)
{

}

/*! Whether or not the update management is available. Returns true if the system is ready
    to perform any upgrade tasks. When the return value is true, it is assumed that the
    system is capable of updating and nymea has permissions to do so.

    A backend plugin should override this to indicate whether update management is available. Defaults to false.
    If the update mechanism becomes available during runtime, \l{availableChanged()} should be emitted
    to indicate the change.
    */
bool PlatformUpdateController::updateManagementAvailable() const
{
    return false;
}

/*! Triggers a cache update on the update system, e.g. call "apt-get update" on the system.

    A backend plugin may override this to to allow the system triggering package manager cache updates.
    */
bool PlatformUpdateController::checkForUpdates()
{
    return false;
}

/*! Indicates whether the update system is busy. This might be the case when \l{checkForUpdates()} or
    \l{enableRepository()} are called. In general, this indicates a state where the update system
    is working and might not be ready to take additional requests. Clients should not perform any operations
    on the upgrade system while this is true.

    A backend plugin may override this and return true when appropriate. For instance if \l{checkForUpdates()}
    is a long-running task and the client should be informed about it being worked on. Additionally,
    whenever the value changes \l{busyChanged()} should be emitted.
    */
bool PlatformUpdateController::busy() const
{
    return false;
}

/*! Indicates whether an update is running. While this is true, clients should not perform
    any operations on the upgrade system and in general expect the system to go down for
    restart any moment. This flag can be used by clients to indicate the running update
    and may restrict interaction with the system.

    A backend plugin should override this and return true when an update is running. Additionally.
    whenever the value changes, \l{updateRunningChanged()} should be emitted.
    */
bool PlatformUpdateController::updateRunning() const
{
    return false;
}

/*! Returns a list of packages availabe in the system. If a backend supports installation of new packages,
    the list of packages may contain not installed packages. Such packages are marked by
    returning an empty \l{Package::installedVersion()}. If the backend supports removal
    of installed packages, uninstallable packages are marked with \l{Package::canRemove()}.
    Packages that can be updated or rolled back will be marked with \l{Package::updateAvailable()}
    and \l{Package::rollbackAvailable()}.

    A backend plugin should override this and return the list of all packages available in the system
    filling in \l{Package} details as described above.

    */
QList<Package> PlatformUpdateController::packages() const
{
    return {};
}

/*! Returns a list of all optional repositories available in the system. Such repositories
    can be enabled or disabled in order to follow prereleases.

    A backend plugin may override this if enabling different repositories is supported.
    */
QList<Repository> PlatformUpdateController::repositories() const
{
    return {};
}

/*! Starts upgrading the packages with the given \a packageIds. If \a packageIds is an empty
    list, a full system upgrade will be performed. Use \l {Package::updateAvailable()}
    to find packages with available upgrades. Passing packageIds for packages which are not
    installed at this time will install the packages, provided the backend supports package
    installation. If \l{canInstallPackages()} is false, only installed packages should
    be provided for upgrades. The return value indicates whether the update has been started or not.

    A backend plugin should override this and start the system upgrades indicating success in
    the return value. Additionally, once the update procedure has started, it should emit
    \l {updateRunningChanged()} and \l {updateRunning()} should return true.
    */
bool PlatformUpdateController::startUpdate(const QStringList &packageIds)
{
    Q_UNUSED(packageIds)
    return false;
}

/*! Starts a rollback of the packages with the given \a packageIds. Use \l {Package::rollbackAvailable()}
    to find packages with available rollbacks.

    A backend plugin should override this and start the rollback process indicating success in
    the return value. Once the rollback procedure has started, it should emit
    \l {updateRunningChanged()} and \l {updateRunning()} should return true.
    */
bool PlatformUpdateController::rollback(const QStringList &packageIds)
{
    Q_UNUSED(packageIds)
    return false;
}

/*! Starts the removal of the packages with the given \a packageIds. Use \l {Package::installedVersion()}
    to find packages which are currently installed. Check \l{Package::canRemove()} to see whether a package
    can be removed or not.

    A backend plugin should override this and start the removal process indicating success in
    the return value. Once the removal procedure has started, it should emit
    \l {updateRunningChanged()} and \l {updateRunning()} should return true.
    */
bool PlatformUpdateController::removePackages(const QStringList &packageIds)
{
    Q_UNUSED(packageIds)
    return false;
}

/*! Enables or disables the given \l{Repository} in the system. The return value indicates whether
    the request has been started successfully or not. Upon successful completion of the request,
    \l{repositoryChanged()} signals should be emitted.

    A backend plugin should override this if multiple install repositories are supported (e.g. if
    \l{repositories()} returns a non-empty list) and perform the requested operation. If enabling/disabling
    a repository implies the immediate upgrade/downgrade of packages, \l{updateRunning()} should be
    marked accordingly.
    */
bool PlatformUpdateController::enableRepository(const QString &repositoryId, bool enabled)
{
    Q_UNUSED(repositoryId)
    Q_UNUSED(enabled)
    return false;
}

