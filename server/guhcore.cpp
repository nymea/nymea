/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class guhserver::GuhCore
    \brief The main entry point for the Guh Server and the place where all the messages are dispatched.

    \inmodule core

    GuhCore is a singleton instance and the main entry point of the Guh daemon. It is responsible to
    instantiate, set up and connect all the other components.
*/

/*! \enum guhserver::GuhCore::RunningMode
    \value RunningModeApplication
        Guh runns as application.
    \value RunningModeService
        Guh is started as service (daemon).
*/

/*! \fn void guhserver::GuhCore::eventTriggered(const Event &event);
    This signal is emitted when an \a event happend.
*/

/*! \fn void guhserver::GuhCore::deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value);
    This signal is emitted when the \l{State} of a \a device changed. The \a stateTypeId parameter describes the
    \l{StateType} and the \a value parameter holds the new value.
*/

/*! \fn void guhserver::GuhCore::deviceRemoved(const DeviceId &deviceId);
    This signal is emitted when a \l{Device} with the given \a deviceId was removed.
*/

/*! \fn void guhserver::GuhCore::deviceAdded(Device *device);
    This signal is emitted when a \a device was added to the system.
*/

/*! \fn void guhserver::GuhCore::deviceParamsChanged(Device *device);
    This signal is emitted when the \l{ParamList}{Params} of a \a device have been changed.
*/

/*! \fn void guhserver::GuhCore::actionExecuted(const ActionId &id, DeviceManager::DeviceError status);
    This signal is emitted when the \l{Action} with the given \a id is finished.
    The \a status of the \l{Action} execution will be described as \l{DeviceManager::DeviceError}{DeviceError}.
*/

/*! \fn void guhserver::GuhCore::devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> deviceDescriptors);
    This signal is emitted when the discovery of a \a deviceClassId is finished. The \a deviceDescriptors parameter describes the
    list of \l{DeviceDescriptor}{DeviceDescriptors} of all discovered \l{Device}{Devices}.
    \sa discoverDevices()
*/

/*! \fn void guhserver::GuhCore::deviceSetupFinished(Device *device, DeviceManager::DeviceError status);
    This signal is emitted when the setup of a \a device is finished. The \a status parameter describes the
    \l{DeviceManager::DeviceError}{DeviceError} that occurred.
*/

/*! \fn void guhserver::GuhCore::deviceEditFinished(Device *device, DeviceManager::DeviceError status);
    This signal is emitted when the edit request of a \a device is finished. The \a status of the edit request will be
    described as \l{DeviceManager::DeviceError}{DeviceError}.
*/

/*! \fn void guhserver::GuhCore::pairingFinished(const PairingTransactionId &pairingTransactionId, DeviceManager::DeviceError status, const DeviceId &deviceId);
    The DeviceManager will emit a this Signal when the pairing of a \l{Device} with the \a deviceId and \a pairingTransactionId is finished.
    The \a status of the pairing will be described as \l{DeviceManager::DeviceError}{DeviceError}.
*/

/*! \fn void guhserver::GuhCore::ruleRemoved(const RuleId &ruleId);
    This signal is emitted when a \l{Rule} with the given \a ruleId was removed.
*/

/*! \fn void guhserver::GuhCore::ruleAdded(const Rule &rule);
    This signal is emitted when a \a rule was added to the system.
*/

/*! \fn void guhserver::GuhCore::ruleConfigurationChanged(const Rule &rule);
    This signal is emitted when the configuration of \a rule changed.
*/

/*! \fn void ruleActiveChanged(const Rule &rule);
    This signal is emitted when a \a rule changed the active state.
    A \l{Rule} is active, when all \l{State}{States} match with the \l{StateDescriptor} conditions.

    \sa Rule::active()
*/

#include "guhcore.h"
#include "loggingcategories.h"
#include "jsonrpcserver.h"
#include "ruleengine.h"
#include "logging/logengine.h"

#include "devicemanager.h"
#include "plugin/device.h"


namespace guhserver {

GuhCore* GuhCore::s_instance = 0;

/*! Returns a pointer to the single \l{GuhCore} instance. */
GuhCore *GuhCore::instance()
{
    if (!s_instance) {
        s_instance = new GuhCore();
    }
    return s_instance;
}

/*! Destructor of the \l{GuhCore}. */
GuhCore::~GuhCore()
{
    m_logger->logSystemEvent(false);
    qCDebug(dcApplication) << "Shutting down. Bye.";
}

/*! Destroyes the \l{GuhCore} instance. */
void GuhCore::destroy()
{
    delete s_instance;
    s_instance = 0;
}

/*! Returns the runningMode of this instance. */
GuhCore::RunningMode GuhCore::runningMode() const
{
    return m_runningMode;
}

/*! Set the \a runningMode of this instance. */
void GuhCore::setRunningMode(const RunningMode &runningMode)
{
    m_runningMode = runningMode;
}

/*! Calls the metheod DeviceManager::plugins().
 *  \sa DeviceManager::plugins(), */
QList<DevicePlugin *> GuhCore::plugins() const
{
    return m_deviceManager->plugins();
}

/*! Calls the metheod DeviceManager::setPluginConfig(\a pluginId, \a params).
 *  \sa DeviceManager::setPluginConfig(), */
DeviceManager::DeviceError GuhCore::setPluginConfig(const PluginId &pluginId, const ParamList &params)
{
    return m_deviceManager->setPluginConfig(pluginId, params);
}

/*! Calls the metheod DeviceManager::supportedVendors().
 *  \sa DeviceManager::supportedVendors(), */
QList<Vendor> GuhCore::supportedVendors() const
{
    return m_deviceManager->supportedVendors();
}

/*! Calls the metheod DeviceManager::supportedDevices(\a vendorId).
 *  \sa DeviceManager::supportedDevices(), */
QList<DeviceClass> GuhCore::supportedDevices(const VendorId &vendorId) const
{
    return m_deviceManager->supportedDevices(vendorId);
}

/*! Removes a configured \l{Device} with the given \a deviceId and \a removePolicyList. */
DeviceManager::DeviceError GuhCore::removeConfiguredDevice(const DeviceId &deviceId, const QHash<RuleId, RuleEngine::RemovePolicy> &removePolicyList)
{
    QHash<RuleId, RuleEngine::RemovePolicy> toBeChanged;
    QList<RuleId> unhandledRules;
    foreach (const RuleId &ruleId, m_ruleEngine->findRules(deviceId)) {
        bool found = false;
        foreach (const RuleId &policyRuleId, removePolicyList.keys()) {
            if (ruleId == policyRuleId) {
                found = true;
                toBeChanged.insert(ruleId, removePolicyList.value(ruleId));
                break;
            }
        }
        if (!found) {
            unhandledRules.append(ruleId);
        }
    }

    if (!unhandledRules.isEmpty()) {
        qCWarning(dcDeviceManager) << "There are unhandled rules which depend on this device.";
        return DeviceManager::DeviceErrorDeviceInUse;
    }

    // Update the rules...
    foreach (const RuleId &ruleId, toBeChanged.keys()) {
        if (toBeChanged.value(ruleId) == RuleEngine::RemovePolicyCascade) {
            m_ruleEngine->removeRule(ruleId);
        } else if (toBeChanged.value(ruleId) == RuleEngine::RemovePolicyUpdate){
            m_ruleEngine->removeDeviceFromRule(ruleId, deviceId);
        }
    }

    DeviceManager::DeviceError removeError = m_deviceManager->removeConfiguredDevice(deviceId);
    if (removeError == DeviceManager::DeviceErrorNoError)
        m_logger->removeDeviceLogs(deviceId);

    return removeError;
}

/*! Calls the metheod DeviceManager::pairDevice(\a pairingTransactionId, \a deviceClassId, \a deviceDescriptorId).
 *  Returns \l{DeviceManager::DeviceError}{DeviceError} to inform about the result.
 *  \sa DeviceManager::pairDevice(), */
DeviceManager::DeviceError GuhCore::pairDevice(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const DeviceDescriptorId &deviceDescriptorId)
{
    return m_deviceManager->pairDevice(pairingTransactionId, deviceClassId, deviceDescriptorId);
}

/*! Calls the metheod DeviceManager::pairDevice(\a pairingTransactionId, \a deviceClassId, \a params).
 *  Returns \l{DeviceManager::DeviceError}{DeviceError} to inform about the result.
 *  \sa DeviceManager::pairDevice(), */
DeviceManager::DeviceError GuhCore::pairDevice(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params)
{
    return m_deviceManager->pairDevice(pairingTransactionId, deviceClassId, params);
}

/*! Calls the metheod DeviceManager::confirmPairing(\a pairingTransactionId, \a secret).
 *  Returns \l{DeviceManager::DeviceError}{DeviceError} to inform about the result.
 *  \sa DeviceManager::confirmPairing(), */
DeviceManager::DeviceError GuhCore::confirmPairing(const PairingTransactionId &pairingTransactionId, const QString &secret)
{
    return m_deviceManager->confirmPairing(pairingTransactionId, secret);
}

/*! Calls the metheod DeviceManager::executeAction(\a action).
 *  \sa DeviceManager::executeAction(), */
DeviceManager::DeviceError GuhCore::executeAction(const Action &action)
{
    DeviceManager::DeviceError ret = m_deviceManager->executeAction(action);
    if (ret == DeviceManager::DeviceErrorNoError) {
        m_logger->logAction(action);
    } else if (ret == DeviceManager::DeviceErrorAsync) {
        m_pendingActions.insert(action.id(), action);
    } else {
        m_logger->logAction(action, Logging::LoggingLevelAlert, ret);
    }
    return ret;
}

void GuhCore::executeRuleActions(const QList<RuleAction> ruleActions)
{
    foreach (const RuleAction &ruleAction, ruleActions) {
        Action action = ruleAction.toAction();
        qCDebug(dcRuleEngine) << "executing action" << ruleAction.actionTypeId() << action.params();
        DeviceManager::DeviceError status = executeAction(action);
        switch(status) {
        case DeviceManager::DeviceErrorNoError:
            break;
        case DeviceManager::DeviceErrorSetupFailed:
            qCWarning(dcRuleEngine) << "Error executing action. Device setup failed.";
            break;
        case DeviceManager::DeviceErrorAsync:
            qCDebug(dcRuleEngine) << "Executing asynchronous action.";
            break;
        case DeviceManager::DeviceErrorInvalidParameter:
            qCWarning(dcRuleEngine) << "Error executing action. Invalid action parameter.";
            break;
        default:
            qCWarning(dcRuleEngine) << "Error executing action:" << status;
        }

        if (status != DeviceManager::DeviceErrorAsync)
            m_logger->logAction(action, status == DeviceManager::DeviceErrorNoError ? Logging::LoggingLevelInfo : Logging::LoggingLevelAlert, status);
    }
}

/*! Calls the metheod DeviceManager::findDeviceClass(\a deviceClassId).
 *  \sa DeviceManager::findDeviceClass(), */
DeviceClass GuhCore::findDeviceClass(const DeviceClassId &deviceClassId) const
{
    return m_deviceManager->findDeviceClass(deviceClassId);
}

/*! Calls the metheod DeviceManager::discoverDevices(\a deviceClassId, \a params).
 *  \sa DeviceManager::discoverDevices(), */
DeviceManager::DeviceError GuhCore::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    return m_deviceManager->discoverDevices(deviceClassId, params);
}

/*! Calls the metheod DeviceManager::addConfiguredDevice(\a deviceClassId, \a params, \a newId).
 *  \sa DeviceManager::addConfiguredDevice(), */
DeviceManager::DeviceError GuhCore::addConfiguredDevice(const DeviceClassId &deviceClassId, const ParamList &params, const DeviceId &newId)
{
    return m_deviceManager->addConfiguredDevice(deviceClassId, params, newId);
}

/*! Calls the metheod DeviceManager::addConfiguredDevice(\a deviceClassId, \a deviceDescriptorId, \a newId).
 *  \sa DeviceManager::addConfiguredDevice(), */
DeviceManager::DeviceError GuhCore::addConfiguredDevice(const DeviceClassId &deviceClassId, const DeviceDescriptorId &deviceDescriptorId, const DeviceId &newId)
{
    return m_deviceManager->addConfiguredDevice(deviceClassId, deviceDescriptorId, newId);
}

/*! Calls the metheod DeviceManager::configuredDevices().
 *  \sa DeviceManager::configuredDevices(), */
QList<Device *> GuhCore::configuredDevices() const
{
    return m_deviceManager->configuredDevices();
}

/*! Calls the metheod DeviceManager::findConfiguredDevice(\a deviceId).
 *  \sa DeviceManager::findConfiguredDevice(), */
Device *GuhCore::findConfiguredDevice(const DeviceId &deviceId) const
{
    return m_deviceManager->findConfiguredDevice(deviceId);
}

/*! Calls the metheod DeviceManager::findConfiguredDevice(\a deviceClassId).
 *  \sa DeviceManager::findConfiguredDevice(), */
QList<Device *> GuhCore::findConfiguredDevices(const DeviceClassId &deviceClassId) const
{
    return m_deviceManager->findConfiguredDevices(deviceClassId);
}

/*! Calls the metheod DeviceManager::editDevice(\a deviceId, \a params).
 *  \sa DeviceManager::editDevice(), */
DeviceManager::DeviceError GuhCore::editDevice(const DeviceId &deviceId, const ParamList &params)
{
    return m_deviceManager->editDevice(deviceId, params);
}

/*! Calls the metheod DeviceManager::editDevice(\a deviceId, \a deviceDescriptorId).
 *  \sa DeviceManager::editDevice(), */
DeviceManager::DeviceError GuhCore::editDevice(const DeviceId &deviceId, const DeviceDescriptorId &deviceDescriptorId)
{
    return m_deviceManager->editDevice(deviceId, deviceDescriptorId);
}

/*! Calls the metheod RuleEngine::rules().
 *  \sa RuleEngine::rules(), */
QList<Rule> GuhCore::rules() const
{
    return m_ruleEngine->rules();
}

/*! Calls the metheod RuleEngine::ruleIds().
 *  \sa RuleEngine::ruleIds(), */
QList<RuleId> GuhCore::ruleIds() const
{
    return m_ruleEngine->ruleIds();
}

/*! Calls the metheod RuleEngine::findRule(\a ruleId).
 *  \sa RuleEngine::findRule(), */
Rule GuhCore::findRule(const RuleId &ruleId)
{
    return m_ruleEngine->findRule(ruleId);
}

/*! Calls the metheod RuleEngine::addRule(\a id, \a name, \a eventDescriptorList, \a stateEvaluator \a actionList, \a exitActionList, \a enabled).
 *  \sa RuleEngine::addRule(), */
RuleEngine::RuleError GuhCore::addRule(const RuleId &id, const QString &name, const QList<EventDescriptor> &eventDescriptorList, const StateEvaluator &stateEvaluator, const QList<RuleAction> &actionList, const QList<RuleAction> &exitActionList, bool enabled, bool executable)
{
    return m_ruleEngine->addRule(id, name, eventDescriptorList, stateEvaluator, actionList, exitActionList, enabled, executable);
}

/*! Calls the metheod RuleEngine::editRule(\a id, \a name, \a eventDescriptorList, \a stateEvaluator \a actionList, \a exitActionList, \a enabled).
 *  \sa RuleEngine::editRule(), */
RuleEngine::RuleError GuhCore::editRule(const RuleId &id, const QString &name, const QList<EventDescriptor> &eventDescriptorList, const StateEvaluator &stateEvaluator, const QList<RuleAction> &actionList, const QList<RuleAction> &exitActionList, bool enabled, bool executable)
{
    return m_ruleEngine->editRule(id, name, eventDescriptorList, stateEvaluator, actionList, exitActionList, enabled, executable);
}

/*! Calls the metheod RuleEngine::removeRule(\a id).
 *  \sa RuleEngine, */
RuleEngine::RuleError GuhCore::removeRule(const RuleId &id)
{
    RuleEngine::RuleError removeError = m_ruleEngine->removeRule(id);
    if (removeError != RuleEngine::RuleErrorNoError)
        m_logger->removeRuleLogs(id);

    return removeError;
}

/*! Calls the metheod RuleEngine::findRules(\a deviceId).
 *  \sa RuleEngine, */
QList<RuleId> GuhCore::findRules(const DeviceId &deviceId)
{
    return m_ruleEngine->findRules(deviceId);
}

/*! Calls the metheod RuleEngine::enableRule(\a ruleId).
 *  \sa RuleEngine::enableRule(), */
RuleEngine::RuleError GuhCore::enableRule(const RuleId &ruleId)
{
    return m_ruleEngine->enableRule(ruleId);
}

/*! Calls the metheod RuleEngine::disableRule(\a ruleId).
 *  \sa RuleEngine::disableRule(), */
RuleEngine::RuleError GuhCore::disableRule(const RuleId &ruleId)
{
    return m_ruleEngine->disableRule(ruleId);
}

RuleEngine::RuleError GuhCore::executeRuleActions(const RuleId &ruleId)
{
    return m_ruleEngine->executeActions(ruleId);
}

RuleEngine::RuleError GuhCore::executeRuleExitActions(const RuleId &ruleId)
{
    return m_ruleEngine->executeExitActions(ruleId);
}

/*! Returns a pointer to the \l{DeviceManager} instance owned by GuhCore.*/
DeviceManager *GuhCore::deviceManager() const
{
    return m_deviceManager;
}

/*! Returns a pointer to the \l{RuleEngine} instance owned by GuhCore.*/
RuleEngine *GuhCore::ruleEngine() const
{
    return m_ruleEngine;
}

/*! Constructs GuhCore with the given \a parent. This is private.
    Use \l{GuhCore::instance()} to access the single instance.*/
GuhCore::GuhCore(QObject *parent) :
    QObject(parent)
{
    qCDebug(dcApplication) << "guh version:" << GUH_VERSION_STRING << "starting up.";

    m_logger = new LogEngine(this);

    qCDebug(dcApplication) << "Creating Device Manager";
    m_deviceManager = new DeviceManager(this);

    qCDebug(dcApplication) << "Creating Rule Engine";
    m_ruleEngine = new RuleEngine(this);


    m_serverManager = new ServerManager(this);

    connect(m_deviceManager, &DeviceManager::eventTriggered, this, &GuhCore::gotEvent);
    connect(m_deviceManager, &DeviceManager::deviceStateChanged, this, &GuhCore::deviceStateChanged);
    connect(m_deviceManager, &DeviceManager::deviceAdded, this, &GuhCore::deviceAdded);
    connect(m_deviceManager, &DeviceManager::deviceParamsChanged, this, &GuhCore::deviceParamsChanged);
    connect(m_deviceManager, &DeviceManager::deviceRemoved, this, &GuhCore::deviceRemoved);
    connect(m_deviceManager, &DeviceManager::actionExecutionFinished, this, &GuhCore::actionExecutionFinished);
    connect(m_deviceManager, &DeviceManager::devicesDiscovered, this, &GuhCore::devicesDiscovered);
    connect(m_deviceManager, &DeviceManager::deviceSetupFinished, this, &GuhCore::deviceSetupFinished);
    connect(m_deviceManager, &DeviceManager::deviceEditFinished, this, &GuhCore::deviceEditFinished);
    connect(m_deviceManager, &DeviceManager::pairingFinished, this, &GuhCore::pairingFinished);

    connect(m_ruleEngine, &RuleEngine::ruleAdded, this, &GuhCore::ruleAdded);
    connect(m_ruleEngine, &RuleEngine::ruleRemoved, this, &GuhCore::ruleRemoved);
    connect(m_ruleEngine, &RuleEngine::ruleConfigurationChanged, this, &GuhCore::ruleConfigurationChanged);

    m_logger->logSystemEvent(true);
}

/*! Connected to the DeviceManager's emitEvent signal. Events received in
    here will be evaluated by the \l{RuleEngine} and the according \l{RuleAction}{RuleActions} are executed.*/
void GuhCore::gotEvent(const Event &event)
{
    m_logger->logEvent(event);
    emit eventTriggered(event);

    QList<RuleAction> actions;
    QList<RuleAction> eventBasedActions;
    foreach (const Rule &rule, m_ruleEngine->evaluateEvent(event)) {
        // Event based
        if (rule.eventDescriptors().count() > 0) {
            m_logger->logRuleTriggered(rule);
            // check if we have an event based action or a normal action
            foreach (const RuleAction &action, rule.actions()) {
                if (action.isEventBased()) {
                    eventBasedActions.append(action);
                } else {
                    actions.append(action);
                }
            }
        } else {
            // State based rule
            m_logger->logRuleActiveChanged(rule);
            emit ruleActiveChanged(rule);
            if (rule.active()) {
                actions.append(rule.actions());
            } else {
                actions.append(rule.exitActions());
            }
        }
    }

    // Set action params, depending on the event value
    foreach (RuleAction ruleAction, eventBasedActions) {
        RuleActionParamList newParams;
        foreach (RuleActionParam ruleActionParam, ruleAction.ruleActionParams()) {
            // if this event param should be taken over in this action
            if (event.eventTypeId() == ruleActionParam.eventTypeId()) {
                QVariant eventValue = event.params().first().value();

                // TODO: get param names...when an event has more than one parameter

                // TODO: limits / scale calculation -> actionValue = eventValue * x
                //       something like a EventParamDescriptor

                ruleActionParam.setValue(eventValue);
                qCDebug(dcRuleEngine) << "take over event param value" << ruleActionParam.value();
            }
            newParams.append(ruleActionParam);
        }
        ruleAction.setRuleActionParams(newParams);
        actions.append(ruleAction);
    }

    executeRuleActions(actions);
}

/*! Return the instance of the log engine */
LogEngine* GuhCore::logEngine() const
{
    return m_logger;
}

/*! Returns the pointer to the \l{JsonRPCServer} of this instance. */
JsonRPCServer *GuhCore::jsonRPCServer() const
{
    return m_serverManager->jsonServer();
}

/*! Returns the pointer to the \l{RestServer} of this instance. */
RestServer *GuhCore::restServer() const
{
    return m_serverManager->restServer();
}

void GuhCore::actionExecutionFinished(const ActionId &id, DeviceManager::DeviceError status)
{
    emit actionExecuted(id, status);
    Action action = m_pendingActions.take(id);
    m_logger->logAction(action, status == DeviceManager::DeviceErrorNoError ? Logging::LoggingLevelInfo : Logging::LoggingLevelAlert, status);
}

}
