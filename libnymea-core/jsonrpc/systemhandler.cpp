/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "systemhandler.h"

#include "platform/platform.h"
#include "platform/platformupdatecontroller.h"
#include "platform/platformsystemcontroller.h"

namespace nymeaserver {

SystemHandler::SystemHandler(Platform *platform, QObject *parent):
    JsonHandler(parent),
    m_platform(platform)
{
    // Methods
    QVariantMap params; QVariantMap returns;
    setDescription("GetCapabilities", "Get the list of capabilites on this system. This allows reading whether things like rebooting or shutting down the system running nymea:core is supported on this host.");
    setParams("GetCapabilities", params);
    returns.insert("powerManagement", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("updateManagement", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("GetCapabilities", returns);

    params.clear(); returns.clear();
    setDescription("Reboot", "Initiate a reboot of the system. The return value will indicate whether the procedure has been initiated successfully.");
    setParams("Reboot", params);
    returns.insert("success", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("Reboot", returns);

    params.clear(); returns.clear();
    setDescription("Shutdown", "Initiate a shutdown of the system. The return value will indicate whether the procedure has been initiated successfully.");
    setParams("Shutdown", params);
    returns.insert("success", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("Shutdown", returns);

    params.clear(); returns.clear();
    setDescription("GetUpdateStatus",
                   "Get the current status of the update system. \"busy\" indicates that the system is current busy with "
                   "an operation regarding updates. This does not necessarily mean an actual update is running. When this "
                   "is true, update related functions on the client should be marked as busy and no interaction with update "
                   "components shall be allowed. An example for such a state is when the system queries the server if there "
                   "are updates available, typically after a call to CheckForUpdates. \"updateRunning\" on the other hand "
                   "indicates an actual update process is ongoing. The user should be informed about it, the system also "
                   "might restart at any point while an update is running.");
    setParams("GetUpdateStatus", params);
    returns.insert("busy", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("updateRunning", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("GetUpdateStatus", returns);

    params.clear(); returns.clear();
    setDescription("CheckForUpdates",
                   "Instruct the system to poll the server for updates. Normally the system should automatically do this "
                   "in regular intervals, however, if the client wants to allow the user to manually check for new updates "
                   "now, this can be called. Returns true if the operation has been started successfully and the update "
                   "manager will become busy. In order to know whether there are updates available, clients should walk through "
                   "the list of packages retrieved from GetPackages and check whether there are packages with the updateAvailable "
                   "flag set to true.");
    setParams("CheckForUpdates", params);
    returns.insert("success", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("CheckForUpdates", returns);

    params.clear(); returns.clear();
    setDescription("GetPackages",
                   "Get the list of packages currently available to the system. This might include installed available but "
                   "not installed packages. Installed packages will have the installedVersion set to a non-empty value.");
    setParams("GetPackages", params);
    returns.insert("packages", QVariantList() << JsonTypes::packageRef());
    setReturns("GetPackages", returns);

    params.clear(); returns.clear();
    setDescription("UpdatePackages",
                   "Starts updating/installing packages with the given ids. Returns true if the upgrade has been started "
                   "successfully. Note that it might still fail later. Before calling this method, clients should "
                   "check the packages whether they are in a state where they can either be installed (no installedVersion "
                   "set) or upgraded (updateAvailable set to true).");
    params.insert("o:packageIds", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("UpdatePackages", params);
    returns.insert("success", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("UpdatePackages", returns);

    params.clear(); returns.clear();
    setDescription("RollbackPackages",
                   "Starts a rollback. Returns true if the rollback has been started successfully. Before calling this "
                   "method, clients should check whether the package can be rolled back (canRollback set to true).");
    params.insert("packageIds", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("RollbackPackages", params);
    returns.insert("success", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("RollbackPackages", returns);

    params.clear(); returns.clear();
    setDescription("RemovePackages",
                   "Starts removing a package. Returns true if the removal has been started successfully. Before calling "
                   "this method, clients should check whether the package can be removed (canRemove set to true).");
    params.insert("packageIds", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("RemovePackages", params);
    returns.insert("success", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("RemovePackages", returns);

    params.clear(); returns.clear();
    setDescription("GetRepositories", "Get the list of repositories currently available to the system.");
    setParams("GetRepositories", params);
    returns.insert("repositories", QVariantList() << JsonTypes::repositoryRef());
    setReturns("GetRepositories", returns);

    params.clear(); returns.clear();
    setDescription("EnableRepository", "Enable or disable a repository.");
    params.insert("repositoryId", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("enabled", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setParams("EnableRepository", params);
    returns.insert("success", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("EnableRepository", returns);


    // Notifications
    params.clear();
    setDescription("CapabilitiesChanged", "Emitted whenever the system capabilities change.");
    params.insert("powerManagement", JsonTypes::basicTypeToString(JsonTypes::Bool));
    params.insert("updateManagement", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setParams("CapabilitiesChanged", params);

    params.clear();
    setDescription("UpdateStatusChanged", "Emitted whenever the update status changes.");
    params.insert("busy", JsonTypes::basicTypeToString(JsonTypes::Bool));
    params.insert("updateRunning", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setParams("UpdateStatusChanged", params);

    params.clear();
    setDescription("PackageAdded", "Emitted whenever a package is added to the list of packages.");
    params.insert("package", JsonTypes::packageRef());
    setParams("PackageAdded", params);

    params.clear();
    setDescription("PackageChanged", "Emitted whenever a package in the list of packages changes.");
    params.insert("package", JsonTypes::packageRef());
    setParams("PackageChanged", params);

    params.clear();
    setDescription("PackageRemoved", "Emitted whenever a package is removed from the list of packages.");
    params.insert("packageId", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("PackageRemoved", params);

    params.clear();
    setDescription("RepositoryAdded", "Emitted whenever a repository is added to the list of repositories.");
    params.insert("repository", JsonTypes::repositoryRef());
    setParams("RepositoryAdded", params);

    params.clear();
    setDescription("RepositoryChanged", "Emitted whenever a repository in the list of repositories changes.");
    params.insert("repository", JsonTypes::repositoryRef());
    setParams("RepositoryChanged", params);

    params.clear();
    setDescription("RepositoryRemoved", "Emitted whenever a repository is removed from the list of repositories.");
    params.insert("repositoryId", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("RepositoryRemoved", params);


    connect(m_platform->systemController(), &PlatformSystemController::availableChanged, this, &SystemHandler::onCapabilitiesChanged);
    connect(m_platform->updateController(), &PlatformUpdateController::availableChanged, this, &SystemHandler::onCapabilitiesChanged);
    connect(m_platform->updateController(), &PlatformUpdateController::busyChanged, this, [this](){
        QVariantMap params;
        params.insert("busy", m_platform->updateController()->busy());
        params.insert("updateRunning", m_platform->updateController()->updateRunning());
        emit UpdateStatusChanged(params);
    });
    connect(m_platform->updateController(), &PlatformUpdateController::updateRunningChanged, this, [this](){
        QVariantMap params;
        params.insert("busy", m_platform->updateController()->busy());
        params.insert("updateRunning", m_platform->updateController()->updateRunning());
        emit UpdateStatusChanged(params);
    });
    connect(m_platform->updateController(), &PlatformUpdateController::packageAdded, this, [this](const Package &package){
        QVariantMap params;
        params.insert("package", JsonTypes::packPackage(package));
        emit PackageAdded(params);
    });
    connect(m_platform->updateController(), &PlatformUpdateController::packageChanged, this, [this](const Package &package){
        QVariantMap params;
        params.insert("package", JsonTypes::packPackage(package));
        emit PackageChanged(params);
    });
    connect(m_platform->updateController(), &PlatformUpdateController::packageRemoved, this, [this](const QString &packageId){
        QVariantMap params;
        params.insert("packageId", packageId);
        emit PackageRemoved(params);
    });
    connect(m_platform->updateController(), &PlatformUpdateController::repositoryAdded, this, [this](const Repository &repository){
        QVariantMap params;
        params.insert("repository", JsonTypes::packRepository(repository));
        emit RepositoryAdded(params);
    });
    connect(m_platform->updateController(), &PlatformUpdateController::repositoryChanged, this, [this](const Repository &repository){
        QVariantMap params;
        params.insert("repository", JsonTypes::packRepository(repository));
        emit RepositoryChanged(params);
    });
    connect(m_platform->updateController(), &PlatformUpdateController::repositoryRemoved, this, [this](const QString &repositoryId){
        QVariantMap params;
        params.insert("repositoryId", repositoryId);
        emit RepositoryRemoved(params);
    });
}

QString SystemHandler::name() const
{
    return "System";
}

JsonReply *SystemHandler::GetCapabilities(const QVariantMap &params)
{
    Q_UNUSED(params)
    QVariantMap data;
    data.insert("powerManagement", m_platform->systemController()->powerManagementAvailable());
    data.insert("updateManagement", m_platform->updateController()->updateManagementAvailable());
    return createReply(data);
}

JsonReply *SystemHandler::Reboot(const QVariantMap &params) const
{
    Q_UNUSED(params);
    bool status = m_platform->systemController()->reboot();
    QVariantMap returns;
    returns.insert("success", status);
    return createReply(returns);
}

JsonReply *SystemHandler::Shutdown(const QVariantMap &params) const
{
    Q_UNUSED(params);
    bool status = m_platform->systemController()->shutdown();
    QVariantMap returns;
    returns.insert("success", status);
    return createReply(returns);
}

JsonReply *SystemHandler::GetUpdateStatus(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap ret;
    ret.insert("busy", m_platform->updateController()->updateRunning());
    ret.insert("updateRunning", m_platform->updateController()->updateRunning());
    return createReply(ret);
}

JsonReply *SystemHandler::CheckForUpdates(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap ret;
    bool success = m_platform->updateController()->checkForUpdates();
    ret.insert("success", success);
    return createReply(ret);
}

JsonReply *SystemHandler::GetPackages(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantList packagelist;
    foreach (const Package &package, m_platform->updateController()->packages()) {
        packagelist.append(JsonTypes::packPackage(package));
    }
    QVariantMap returns;
    returns.insert("packages", packagelist);
    return createReply(returns);
}

JsonReply *SystemHandler::UpdatePackages(const QVariantMap &params) const
{
    bool success = m_platform->updateController()->startUpdate(params.value("packageIds").toStringList());
    QVariantMap returns;
    returns.insert("success", success);
    return createReply(returns);
}

JsonReply *SystemHandler::RollbackPackages(const QVariantMap &params) const
{
    bool success = m_platform->updateController()->rollback(params.value("packageIds").toStringList());
    QVariantMap returns;
    returns.insert("success", success);
    return createReply(returns);
}

JsonReply *SystemHandler::RemovePackages(const QVariantMap &params) const
{
    bool success = m_platform->updateController()->removePackages(params.value("packageIds").toStringList());
    QVariantMap returns;
    returns.insert("success", success);
    return createReply(returns);
}

JsonReply *SystemHandler::GetRepositories(const QVariantMap &params) const
{
    Q_UNUSED(params);
    QVariantList repos;
    foreach (const Repository &repository, m_platform->updateController()->repositories()) {
        repos.append(JsonTypes::packRepository(repository));
    }
    QVariantMap returns;
    returns.insert("repositories", repos);
    return createReply(returns);

}

JsonReply *SystemHandler::EnableRepository(const QVariantMap &params) const
{
    bool success = m_platform->updateController()->enableRepository(params.value("repositoryId").toString(), params.value("enabled").toBool());
    QVariantMap returns;
    returns.insert("success", success);
    return createReply(returns);
}

void SystemHandler::onCapabilitiesChanged()
{
    QVariantMap caps;
    caps.insert("powerManagement", m_platform->systemController()->powerManagementAvailable());
    caps.insert("updateManagement", m_platform->updateController()->updateManagementAvailable());
    emit CapabilitiesChanged(caps);
}

}
