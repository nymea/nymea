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

#include "systemhandler.h"

#include "platform/platform.h"
#include "platform/platformsystemcontroller.h"
#include "platform/platformupdatecontroller.h"

namespace nymeaserver {

SystemHandler::SystemHandler(Platform *platform, QObject *parent)
    : JsonHandler(parent)
    , m_platform(platform)
{
    // Objects
    registerObject<Package, Packages>();
    registerObject<Repository, Repositories>();

    // Methods
    QString description;
    QVariantMap params;
    QVariantMap returns;
    description = "Get the list of capabilites on this system. The property \"powerManagement\" indicates whether "
                  "restarting nymea and rebooting or shutting down is supported on this system. The property \"updateManagement indicates "
                  "whether system update features are available in this system. The property \"timeManagement\" "
                  "indicates whether the system time can be configured on this system. Note that GetTime will be "
                  "available in any case.";
    returns.insert("powerManagement", enumValueName(Bool));
    returns.insert("updateManagement", enumValueName(Bool));
    returns.insert("timeManagement", enumValueName(Bool));
    registerMethod("GetCapabilities", description, params, returns);

    params.clear();
    returns.clear();
    description = "Initiate a restart of the nymea service. The return value will indicate whether the procedure has been initiated successfully.";
    returns.insert("success", enumValueName(Bool));
    registerMethod("Restart", description, params, returns);

    params.clear();
    returns.clear();
    description = "Initiate a reboot of the system. The return value will indicate whether the procedure has been initiated successfully.";
    returns.insert("success", enumValueName(Bool));
    registerMethod("Reboot", description, params, returns);

    params.clear();
    returns.clear();
    description = "Initiate a shutdown of the system. The return value will indicate whether the procedure has been initiated successfully.";
    returns.insert("success", enumValueName(Bool));
    registerMethod("Shutdown", description, params, returns);

    params.clear();
    returns.clear();
    description = "Get the current status of the update system. \"busy\" indicates that the system is current busy with "
                  "an operation regarding updates. This does not necessarily mean an actual update is running. When this "
                  "is true, update related functions on the client should be marked as busy and no interaction with update "
                  "components shall be allowed. An example for such a state is when the system queries the server if there "
                  "are updates available, typically after a call to CheckForUpdates. \"updateRunning\" on the other hand "
                  "indicates an actual update process is ongoing. The user should be informed about it, the system also "
                  "might restart at any point while an update is running.";
    returns.insert("busy", enumValueName(Bool));
    returns.insert("updateRunning", enumValueName(Bool));
    registerMethod("GetUpdateStatus", description, params, returns);

    params.clear();
    returns.clear();
    description = "Instruct the system to poll the server for updates. Normally the system should automatically do this "
                  "in regular intervals, however, if the client wants to allow the user to manually check for new updates "
                  "now, this can be called. Returns true if the operation has been started successfully and the update "
                  "manager will become busy. In order to know whether there are updates available, clients should walk through "
                  "the list of packages retrieved from GetPackages and check whether there are packages with the updateAvailable "
                  "flag set to true.";
    returns.insert("success", enumValueName(Bool));
    registerMethod("CheckForUpdates", description, params, returns);

    params.clear();
    returns.clear();
    description = "Get the list of packages currently available to the system. This might include installed available but "
                  "not installed packages. Installed packages will have the installedVersion set to a non-empty value.";
    returns.insert("packages", objectRef("Packages"));
    registerMethod("GetPackages", description, params, returns);

    params.clear();
    returns.clear();
    description = "Starts updating/installing packages with the given ids. Returns true if the upgrade has been started "
                  "successfully. Note that it might still fail later. Before calling this method, clients should "
                  "check the packages whether they are in a state where they can either be installed (no installedVersion "
                  "set) or upgraded (updateAvailable set to true).";
    params.insert("o:packageIds", QVariantList() << enumValueName(String));
    returns.insert("success", enumValueName(Bool));
    registerMethod("UpdatePackages", description, params, returns);

    params.clear();
    returns.clear();
    description = "Starts a rollback. Returns true if the rollback has been started successfully. Before calling this "
                  "method, clients should check whether the package can be rolled back (canRollback set to true).";
    params.insert("packageIds", QVariantList() << enumValueName(String));
    returns.insert("success", enumValueName(Bool));
    registerMethod("RollbackPackages", description, params, returns);

    params.clear();
    returns.clear();
    description = "Starts removing a package. Returns true if the removal has been started successfully. Before calling "
                  "this method, clients should check whether the package can be removed (canRemove set to true).";
    params.insert("packageIds", QVariantList() << enumValueName(String));
    returns.insert("success", enumValueName(Bool));
    registerMethod("RemovePackages", description, params, returns);

    params.clear();
    returns.clear();
    description = "Get the list of repositories currently available to the system.";
    returns.insert("repositories", objectRef("Repositories"));
    registerMethod("GetRepositories", description, params, returns);

    params.clear();
    returns.clear();
    description = "Enable or disable a repository.";
    params.insert("repositoryId", enumValueName(String));
    params.insert("enabled", enumValueName(Bool));
    returns.insert("success", enumValueName(Bool));
    registerMethod("EnableRepository", description, params, returns);

    params.clear();
    returns.clear();
    description = "Get the system time and configuraton. The \"time\" and \"timeZone\" properties "
                  "give the current server time and time zone. \"automaticTimeAvailable\" indicates whether "
                  "this system supports automatically setting the clock (e.g. using NTP). \"automaticTime\" will "
                  "be true if the system is configured to automatically update the clock.";
    returns.insert("time", enumValueName(Uint));
    returns.insert("timeZone", enumValueName(String));
    returns.insert("automaticTimeAvailable", enumValueName(Bool));
    returns.insert("automaticTime", enumValueName(Bool));
    registerMethod("GetTime", description, params, returns);

    params.clear();
    returns.clear();
    description = "Set the system time configuraton. The system can be configured to update the time automatically "
                  "by setting \"automaticTime\" to true. This will only work if the \"timeManagement\" capability is "
                  "available on this system and \"GetTime\" indicates the availability of automatic time settings. If "
                  "any of those requirements are not met, this method will return \"false\" in the \"success\" property. "
                  "In order to manually configure the time, \"automaticTime\" should be set to false and \"time\" should "
                  "be set. Note that if \"automaticTime\" is set to true and a manual \"time\" is still passed, the system "
                  "will attempt to configure automatic time updates and only set the manual time if automatic mode fails. "
                  "A time zone can always be passed optionally to change the system time zone and should be a IANA time zone "
                  "id.";
    params.insert("o:automaticTime", enumValueName(Bool));
    params.insert("o:time", enumValueName(Uint));
    params.insert("o:timeZone", enumValueName(String));
    returns.insert("success", enumValueName(Bool));
    registerMethod("SetTime", description, params, returns);

    params.clear();
    returns.clear();
    description = "Returns the list of IANA specified time zone IDs which can be used to select a time zone. It is not "
                  "required to use this method if the client toolkit already provides means to obtain a list of IANA time "
                  "zone ids.";
    returns.insert("timeZones", enumValueName(StringList));
    registerMethod("GetTimeZones", description, params, returns);

    params.clear();
    returns.clear();
    description = "Returns information about the system nymea is running on.";
    returns.insert("deviceSerialNumber", enumValueName(String));
    registerMethod("GetSystemInfo", description, params, returns);

    // Notifications
    params.clear();
    description = "Emitted whenever the system capabilities change.";
    params.insert("powerManagement", enumValueName(Bool));
    params.insert("updateManagement", enumValueName(Bool));
    registerNotification("CapabilitiesChanged", description, params);

    params.clear();
    description = "Emitted whenever the update status changes.";
    params.insert("busy", enumValueName(Bool));
    params.insert("updateRunning", enumValueName(Bool));
    registerNotification("UpdateStatusChanged", description, params);

    params.clear();
    description = "Emitted whenever a package is added to the list of packages.";
    params.insert("package", objectRef("Package"));
    registerNotification("PackageAdded", description, params);

    params.clear();
    description = "Emitted whenever a package in the list of packages changes.";
    params.insert("package", objectRef("Package"));
    registerNotification("PackageChanged", description, params);

    params.clear();
    description = "Emitted whenever a package is removed from the list of packages.";
    params.insert("packageId", enumValueName(String));
    registerNotification("PackageRemoved", description, params);

    params.clear();
    description = "Emitted whenever a repository is added to the list of repositories.";
    params.insert("repository", objectRef("Repository"));
    registerNotification("RepositoryAdded", description, params);

    params.clear();
    description = "Emitted whenever a repository in the list of repositories changes.";
    params.insert("repository", objectRef("Repository"));
    registerNotification("RepositoryChanged", description, params);

    params.clear();
    description = "Emitted whenever a repository is removed from the list of repositories.";
    params.insert("repositoryId", enumValueName(String));
    registerNotification("RepositoryRemoved", description, params);

    params.clear();
    description = "Emitted whenever the time configuration is changed";
    params.insert("time", enumValueName(Uint));
    params.insert("timeZone", enumValueName(String));
    params.insert("automaticTimeAvailable", enumValueName(Bool));
    params.insert("automaticTime", enumValueName(Bool));
    registerNotification("TimeConfigurationChanged", description, params);

    connect(m_platform->systemController(), &PlatformSystemController::availableChanged, this, &SystemHandler::onCapabilitiesChanged);
    connect(m_platform->updateController(), &PlatformUpdateController::availableChanged, this, &SystemHandler::onCapabilitiesChanged);
    connect(m_platform->updateController(), &PlatformUpdateController::busyChanged, this, [this]() {
        QVariantMap params;
        params.insert("busy", m_platform->updateController()->busy());
        params.insert("updateRunning", m_platform->updateController()->updateRunning());
        emit UpdateStatusChanged(params);
    });
    connect(m_platform->updateController(), &PlatformUpdateController::updateRunningChanged, this, [this]() {
        QVariantMap params;
        params.insert("busy", m_platform->updateController()->busy());
        params.insert("updateRunning", m_platform->updateController()->updateRunning());
        emit UpdateStatusChanged(params);
    });
    connect(m_platform->updateController(), &PlatformUpdateController::packageAdded, this, [this](const Package &package) {
        QVariantMap params;
        params.insert("package", pack(package));
        emit PackageAdded(params);
    });
    connect(m_platform->updateController(), &PlatformUpdateController::packageChanged, this, [this](const Package &package) {
        QVariantMap params;
        params.insert("package", pack(package));
        emit PackageChanged(params);
    });
    connect(m_platform->updateController(), &PlatformUpdateController::packageRemoved, this, [this](const QString &packageId) {
        QVariantMap params;
        params.insert("packageId", packageId);
        emit PackageRemoved(params);
    });
    connect(m_platform->updateController(), &PlatformUpdateController::repositoryAdded, this, [this](const Repository &repository) {
        QVariantMap params;
        params.insert("repository", pack(repository));
        emit RepositoryAdded(params);
    });
    connect(m_platform->updateController(), &PlatformUpdateController::repositoryChanged, this, [this](const Repository &repository) {
        QVariantMap params;
        params.insert("repository", pack(repository));
        emit RepositoryChanged(params);
    });
    connect(m_platform->updateController(), &PlatformUpdateController::repositoryRemoved, this, [this](const QString &repositoryId) {
        QVariantMap params;
        params.insert("repositoryId", repositoryId);
        emit RepositoryRemoved(params);
    });
    connect(
        m_platform->systemController(),
        &PlatformSystemController::timeConfigurationChanged,
        this,
        [this]() {
            QVariantMap params;
            params.insert("time", QDateTime::currentMSecsSinceEpoch() / 1000);
            params.insert("timeZone", QTimeZone::systemTimeZoneId());
            params.insert("automaticTimeAvailable", m_platform->systemController()->automaticTimeAvailable());
            params.insert("automaticTime", m_platform->systemController()->automaticTime());
            emit TimeConfigurationChanged(params);
        },
        Qt::QueuedConnection); // Queued to give QDateTime a chance to sync itself to the system
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
    data.insert("timeManagement", m_platform->systemController()->timeManagementAvailable());
    return createReply(data);
}

JsonReply *SystemHandler::Restart(const QVariantMap &params) const
{
    Q_UNUSED(params)
    bool status = m_platform->systemController()->restart();
    QVariantMap returns;
    returns.insert("success", status);
    return createReply(returns);
}

JsonReply *SystemHandler::Reboot(const QVariantMap &params) const
{
    Q_UNUSED(params)
    bool status = m_platform->systemController()->reboot();
    QVariantMap returns;
    returns.insert("success", status);
    return createReply(returns);
}

JsonReply *SystemHandler::Shutdown(const QVariantMap &params) const
{
    Q_UNUSED(params)
    bool status = m_platform->systemController()->shutdown();
    QVariantMap returns;
    returns.insert("success", status);
    return createReply(returns);
}

JsonReply *SystemHandler::GetUpdateStatus(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap ret;
    ret.insert("busy", m_platform->updateController()->busy());
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
        packagelist.append(pack(package));
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
    Q_UNUSED(params)
    QVariantList repos;
    foreach (const Repository &repository, m_platform->updateController()->repositories()) {
        repos.append(pack(repository));
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

JsonReply *SystemHandler::GetTime(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap returns;
    returns.insert("automaticTimeAvailable", m_platform->systemController()->automaticTimeAvailable());
    returns.insert("automaticTime", m_platform->systemController()->automaticTime());
    returns.insert("time", QDateTime::currentMSecsSinceEpoch() / 1000);
    returns.insert("timeZone", QTimeZone::systemTimeZoneId());
    return createReply(returns);
}

JsonReply *SystemHandler::SetTime(const QVariantMap &params) const
{
    QVariantMap returns;
    bool handled = false;
    bool automaticTime = params.value("automaticTime", false).toBool();
    if (params.contains("automaticTime") && m_platform->systemController()->automaticTimeAvailable()) {
        if (!m_platform->systemController()->setAutomaticTime(automaticTime)) {
            returns.insert("success", false);
            return createReply(returns);
        }
        handled = true;
    }
    if (!automaticTime && params.contains("time")) {
        QDateTime time = QDateTime::fromMSecsSinceEpoch(params.value("time").toLongLong() * 1000);
        if (!m_platform->systemController()->setTime(time)) {
            returns.insert("success", false);
            return createReply(returns);
        }
        handled = true;
    }
    if (params.contains("timeZone")) {
        QTimeZone timeZone(params.value("timeZone").toByteArray());
        if (!m_platform->systemController()->setTimeZone(timeZone)) {
            returns.insert("success", false);
            return createReply(returns);
        }
        handled = true;
    }
    returns.insert("success", handled);
    return createReply(returns);
}

JsonReply *SystemHandler::GetTimeZones(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantList timeZones;
    foreach (const QByteArray &timeZoneId, QTimeZone::availableTimeZoneIds()) {
        timeZones.append(QString::fromUtf8(timeZoneId));
    }

    QVariantMap returns;
    returns.insert("timeZones", timeZones);
    return createReply(returns);
}

JsonReply *SystemHandler::GetSystemInfo(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap returns;
    QString deviceSerial = m_platform->systemController()->deviceSerialNumber();
    returns.insert("deviceSerialNumber", deviceSerial);
    return createReply(returns);
}

void SystemHandler::onCapabilitiesChanged()
{
    QVariantMap caps;
    caps.insert("powerManagement", m_platform->systemController()->powerManagementAvailable());
    caps.insert("updateManagement", m_platform->updateController()->updateManagementAvailable());
    emit CapabilitiesChanged(caps);
}

} // namespace nymeaserver
