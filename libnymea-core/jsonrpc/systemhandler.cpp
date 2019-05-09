#include "systemhandler.h"

#include "platform/platform.h"
#include "platform/platformupdatecontroller.h"
#include "platform/platformsystemcontroller.h"

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
    setDescription("GetUpdateStatus", "Get the current system status in regard to updates. That is, the currently installed version, any candidate version available etc.");
    setParams("GetUpdateStatus", params);
    returns.insert("updateAvailable", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("currentVersion", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("candidateVersion", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("availableChannels", JsonTypes::basicTypeToString(JsonTypes::StringList));
    returns.insert("currentChannel", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("updateInProgress", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("GetUpdateStatus", returns);

    params.clear(); returns.clear();
    setDescription("StartUpdate", "Starts a system update. Returns true if the upgrade has been started successfully.");
    setParams("StartUpdate", params);
    returns.insert("success", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("StartUpdate", returns);

    params.clear(); returns.clear();
    setDescription("SelectChannel", "Select an update channel.");
    params.insert("channel", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("SelectChannel", params);
    returns.insert("success", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("SelectChannel", returns);


    // Notifications
    params.clear();
    setDescription("UpdateStatusChanged", "Emitted whenever there is a change in the information from GetUpdateStatus");
    params.insert("updateAvailable", JsonTypes::basicTypeToString(JsonTypes::Bool));
    params.insert("currentVersion", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("candidateVersion", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("availableChannels", JsonTypes::basicTypeToString(JsonTypes::StringList));
    params.insert("currentChannel", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("updateInProgress", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setParams("UpdateStatusChanged", params);

    connect(m_platform->updateController(), &PlatformUpdateController::updateStatusChanged, this, &SystemHandler::onUpdateStatusChanged);
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
    Q_UNUSED(params);
    QVariantMap returns;
    returns.insert("updateAvailable", m_platform->updateController()->updateAvailable());
    returns.insert("currentVersion", m_platform->updateController()->currentVersion());
    returns.insert("candidateVersion", m_platform->updateController()->candidateVersion());
    returns.insert("availableChannels", m_platform->updateController()->availableChannels());
    returns.insert("currentChannel", m_platform->updateController()->currentChannel());
    returns.insert("updateInProgress", m_platform->updateController()->updateInProgress());
    return createReply(returns);
}

JsonReply *SystemHandler::StartUpdate(const QVariantMap &params)
{
    Q_UNUSED(params)
    QVariantMap returns;
    bool success = m_platform->updateController()->startUpdate();
    returns.insert("success", success);
    return createReply(returns);
}

JsonReply *SystemHandler::SelectChannel(const QVariantMap &params)
{
    QString channel = params.value("channel").toString();

    QVariantMap returns;
    if (m_platform->updateController()->availableChannels().contains(channel)) {
        bool success = m_platform->updateController()->selectChannel(channel);
        returns.insert("success", success);
    } else {
        returns.insert("success", false);
    }
    return createReply(returns);
}

void SystemHandler::onUpdateStatusChanged()
{
    QVariantMap params;
    params.insert("updateAvailable", m_platform->updateController()->updateAvailable());
    params.insert("currentVersion", m_platform->updateController()->currentVersion());
    params.insert("candidateVersion", m_platform->updateController()->candidateVersion());
    params.insert("availableChannels", m_platform->updateController()->availableChannels());
    params.insert("currentChannel", m_platform->updateController()->currentChannel());
    params.insert("updateInProgress", m_platform->updateController()->updateInProgress());
    emit UpdateStatusChanged(params);
}
