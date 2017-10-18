/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2018 Simon Stürz <simon.stuerz@guh.io>              *
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

/*! \fn void guhserver::GuhCore::deviceChanged(Device *device);
    This signal is emitted when the \l{ParamList}{Params} of a \a device have been changed.
*/

/*! \fn void guhserver::GuhCore::actionExecuted(const ActionId &id, DeviceManager::DeviceError status);
    This signal is emitted when the \l{Action} with the given \a id is finished.
    The \a status of the \l{Action} execution will be described as \l{DeviceManager::DeviceError}{DeviceError}.
*/

/*! \fn void guhserver::GuhCore::devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> deviceDescriptors);
    This signal is emitted when the discovery of a \a deviceClassId is finished. The \a deviceDescriptors parameter describes the
    list of \l{DeviceDescriptor}{DeviceDescriptors} of all discovered \l{Device}{Devices}.
    \sa DeviceManager::discoverDevices()
*/

/*! \fn void guhserver::GuhCore::deviceSetupFinished(Device *device, DeviceManager::DeviceError status);
    This signal is emitted when the setup of a \a device is finished. The \a status parameter describes the
    \l{DeviceManager::DeviceError}{DeviceError} that occurred.
*/

/*! \fn void guhserver::GuhCore::deviceReconfigurationFinished(Device *device, DeviceManager::DeviceError status);
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
#include "networkmanager/networkmanager.h"

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
    m_logger->logSystemEvent(m_timeManager->currentDateTime(), false);

    // Make sure DeviceManager is teared down at first so plugins don't access any ressources any more.
    qCDebug(dcApplication) << "Shutting down \"Device Manager\"";
    delete m_deviceManager;

    qCDebug(dcApplication) << "Shutting down \"Log Engine\"";
    delete m_logger;

    qCDebug(dcApplication()) << "Shutting down \"Hardware Manager\"";
    delete m_hardwareManager;

    qCDebug(dcApplication) << "Shutting down \"Rule Engine\"";
    delete m_ruleEngine;

    qCDebug(dcApplication) << "Shutting down \"Server Manager\"";
    delete m_serverManager;

    qCDebug(dcApplication) << "Shutting down \"CloudManager\"";
    delete m_cloudManager;
}

/*! Destroyes the \l{GuhCore} instance. */
void GuhCore::destroy()
{
    if (s_instance) {
        delete s_instance;
    }

    s_instance = 0;
}

/*! Removes a configured \l{Device} with the given \a deviceId and \a removePolicyList. */
QPair<DeviceManager::DeviceError, QList<RuleId> > GuhCore::removeConfiguredDevice(const DeviceId &deviceId, const QHash<RuleId, RuleEngine::RemovePolicy> &removePolicyList)
{
    // Check if this is a child device
    Device *device = m_deviceManager->findConfiguredDevice(deviceId);

    if (!device)
        return QPair<DeviceManager::DeviceError, QList<RuleId> > (DeviceManager::DeviceErrorDeviceNotFound, QList<RuleId>());

    if (!device->parentId().isNull()) {
        qCWarning(dcDeviceManager) << "The device is a child of" << device->parentId().toString() << ". Please remove the parent device.";
        return QPair<DeviceManager::DeviceError, QList<RuleId> > (DeviceManager::DeviceErrorDeviceIsChild, QList<RuleId>());
    }

    // FIXME: Let's remove this for now. It will come back with more fine grained control, presumably introducing a RemoveMethod flag in the DeviceClass
//    if (device->autoCreated()) {
//        qCWarning(dcDeviceManager) << "This device has been auto-created and cannot be deleted manually.";
//        return QPair<DeviceManager::DeviceError, QList<RuleId> >(DeviceManager::DeviceErrorCreationMethodNotSupported, {});
//    }

    // Check if this device has child devices
    QList<Device *> devicesToRemove;
    devicesToRemove.append(device);
    QList<Device *> childDevices = m_deviceManager->findChildDevices(deviceId);
    if (!childDevices.isEmpty()) {
        foreach (Device *child, childDevices) {
            devicesToRemove.append(child);
        }
    }

    // check devices
    QList<RuleId> offendingRules;
    qCDebug(dcDeviceManager) << "Devices to remove:";
    foreach (Device *d, devicesToRemove) {
        qCDebug(dcDeviceManager) << " -> " << d->name() << d->id().toString();

        // Check if device is in a rule
        foreach (const RuleId &ruleId, m_ruleEngine->findRules(d->id())) {
            qCDebug(dcDeviceManager) << "      -> in rule:" << ruleId.toString();
            if (!offendingRules.contains(ruleId)) {
                offendingRules.append(ruleId);
            }
        }
    }

    // check each offending rule if there is a corresponding remove policy
    QHash<RuleId, RuleEngine::RemovePolicy> toBeChanged;
    QList<RuleId> unhandledRules;
    foreach (const RuleId &ruleId, offendingRules) {
        bool found = false;
        foreach (const RuleId &policyRuleId, removePolicyList.keys()) {
            if (ruleId == policyRuleId) {
                found = true;
                toBeChanged.insert(ruleId, removePolicyList.value(ruleId));
                break;
            }
        }
        if (!found)
            unhandledRules.append(ruleId);

    }

    if (!unhandledRules.isEmpty()) {
        qCWarning(dcDeviceManager) << "There are unhandled rules which depend on this device:\n" << unhandledRules;
        return QPair<DeviceManager::DeviceError, QList<RuleId> > (DeviceManager::DeviceErrorDeviceInRule, unhandledRules);
    }

    // Update the rules...
    foreach (const RuleId &ruleId, toBeChanged.keys()) {
        if (toBeChanged.value(ruleId) == RuleEngine::RemovePolicyCascade) {
            m_ruleEngine->removeRule(ruleId);
        } else if (toBeChanged.value(ruleId) == RuleEngine::RemovePolicyUpdate){
            foreach (Device *d, devicesToRemove) {
                m_ruleEngine->removeDeviceFromRule(ruleId, d->id());
            }
        }
    }

    // remove the child devices
    foreach (Device *d, childDevices) {
        DeviceManager::DeviceError removeError = m_deviceManager->removeConfiguredDevice(d->id());
        if (removeError == DeviceManager::DeviceErrorNoError) {
            m_logger->removeDeviceLogs(d->id());
        }
    }

    // delete the devices
    DeviceManager::DeviceError removeError = m_deviceManager->removeConfiguredDevice(deviceId);
    if (removeError == DeviceManager::DeviceErrorNoError) {
        m_logger->removeDeviceLogs(deviceId);
    }

    return QPair<DeviceManager::DeviceError, QList<RuleId> > (DeviceManager::DeviceErrorNoError, QList<RuleId>());
}


/*! Removes a configured \l{Device} with the given \a deviceId and \a removePolicy. */
DeviceManager::DeviceError GuhCore::removeConfiguredDevice(const DeviceId &deviceId, const RuleEngine::RemovePolicy &removePolicy)
{
    // Check if this is a child device
    Device *device = m_deviceManager->findConfiguredDevice(deviceId);

    if (!device)
        return DeviceManager::DeviceErrorDeviceNotFound;

    if (!device->parentId().isNull()) {
        qCWarning(dcDeviceManager) << "The device is a child of" << device->parentId().toString() << ". Please remove the parent device.";
        return DeviceManager::DeviceErrorDeviceIsChild;
    }

    if (device->autoCreated()) {
        qCWarning(dcDeviceManager) << "This device has been auto-created and cannot be deleted manually.";
        return DeviceManager::DeviceErrorCreationMethodNotSupported;
    }

    // Check if this device has child devices
    QList<Device *> devicesToRemove;
    devicesToRemove.append(device);
    QList<Device *> childDevices = m_deviceManager->findChildDevices(deviceId);
    if (!childDevices.isEmpty()) {
        foreach (Device *child, childDevices) {
            devicesToRemove.append(child);
        }
    }

    // check devices
    QList<RuleId> offendingRules;
    qCDebug(dcDeviceManager) << "Devices to remove:";
    foreach (Device *d, devicesToRemove) {
        qCDebug(dcDeviceManager) << " -> " << d->name() << d->id().toString();

        // Check if device is in a rule
        foreach (const RuleId &ruleId, m_ruleEngine->findRules(d->id())) {
            qCDebug(dcDeviceManager) << "      -> in rule:" << ruleId.toString();
            if (!offendingRules.contains(ruleId)) {
                offendingRules.append(ruleId);
            }
        }
    }

    // apply removepolicy for foreach rule
    foreach (const RuleId &ruleId, offendingRules) {
        if (removePolicy == RuleEngine::RemovePolicyCascade) {
            m_ruleEngine->removeRule(ruleId);
        } else if (removePolicy == RuleEngine::RemovePolicyUpdate){
            foreach (Device *d, devicesToRemove) {
                m_ruleEngine->removeDeviceFromRule(ruleId, d->id());
            }
        }
    }

    // remove the child devices
    foreach (Device *d, childDevices) {
        DeviceManager::DeviceError removeError = m_deviceManager->removeConfiguredDevice(d->id());
        if (removeError == DeviceManager::DeviceErrorNoError) {
            m_logger->removeDeviceLogs(d->id());
        }
    }

    // delete the devices
    DeviceManager::DeviceError removeError = m_deviceManager->removeConfiguredDevice(deviceId);
    if (removeError == DeviceManager::DeviceErrorNoError) {
        m_logger->removeDeviceLogs(deviceId);
    }

    return removeError;
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

/*! Execute the given \a ruleActions. */
void GuhCore::executeRuleActions(const QList<RuleAction> ruleActions)
{
    QList<Action> actions;
    foreach (const RuleAction &ruleAction, ruleActions) {
        if (ruleAction.type() == RuleAction::TypeDevice) {
            actions.append(ruleAction.toAction());
        } else {
            QList<Device*> devices = m_deviceManager->findConfiguredDevices(ruleAction.interface());
            foreach (Device* device, devices) {
                DeviceClass dc = m_deviceManager->findDeviceClass(device->deviceClassId());
                ActionType at = dc.actionTypes().findByName(ruleAction.interfaceAction());
                if (at.id().isNull()) {
                    qCWarning(dcRuleEngine()) << "Error creating Action. The given DeviceClass does not implement action:" << ruleAction.interfaceAction();
                    continue;
                }
                Action action = Action(at.id(), device->id());
                ParamList params;
                foreach (const RuleActionParam &rap, ruleAction.ruleActionParams()) {
                    ParamType pt = at.paramTypes().findByName(rap.paramName());
                    if (pt.id().isNull()) {
                        qCWarning(dcRuleEngine()) << "Error creating Action. Failed to match interface param type to DeviceClass paramtype.";
                        continue;
                    }
                    params.append(Param(pt.id(), rap.value()));
                }
                action.setParams(params);
                actions.append(action);
            }
        }
    }
    foreach (const Action &action, actions) {
        qCDebug(dcRuleEngine) << "Executing action" << action.actionTypeId() << action.params();
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

        //        if (status != DeviceManager::DeviceErrorAsync)
        //            m_logger->logAction(action, status == DeviceManager::DeviceErrorNoError ? Logging::LoggingLevelInfo : Logging::LoggingLevelAlert, status);
    }
}

/*! Calls the metheod RuleEngine::removeRule(\a id).
 *  \sa RuleEngine, */
RuleEngine::RuleError GuhCore::removeRule(const RuleId &id)
{
    RuleEngine::RuleError removeError = m_ruleEngine->removeRule(id);
    if (removeError == RuleEngine::RuleErrorNoError)
        m_logger->removeRuleLogs(id);

    return removeError;
}

/*! Returns a pointer to the \l{GuhConfiguration} instance owned by GuhCore.*/
GuhConfiguration *GuhCore::configuration() const
{
    return m_configuration;
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

/*! Returns a pointer to the \l{TimeManager} instance owned by GuhCore.*/
TimeManager *GuhCore::timeManager() const
{
    return m_timeManager;
}

/*! Returns a pointer to the \l{ServerManager} instance owned by GuhCore. */
ServerManager *GuhCore::serverManager() const
{
    return m_serverManager;
}

/*! Returns the list of available system languages. */
QStringList GuhCore::getAvailableLanguages()
{
    // TODO: parse available translation files
    return QStringList() << "en_US" << "de_DE";
}

/*! Returns a pointer to the \l{BluetoothServer} instance owned by GuhCore. */
BluetoothServer *GuhCore::bluetoothServer() const
{
    return m_serverManager->bluetoothServer();
}

/*! Returns a pointer to the \l{NetworkManager} instance owned by GuhCore. */
NetworkManager *GuhCore::networkManager() const
{
    return m_networkManager;
}

UserManager *GuhCore::userManager() const
{
    return m_userManager;
}

CloudManager *GuhCore::cloudManager() const
{
    return m_cloudManager;
}


/*! Constructs GuhCore with the given \a parent. This is private.
    Use \l{GuhCore::instance()} to access the single instance.*/
GuhCore::GuhCore(QObject *parent) :
    QObject(parent)
{
    staticMetaObject.invokeMethod(this, "init", Qt::QueuedConnection);
}

void GuhCore::init() {
    qCDebug(dcApplication()) << "Loading guh configurations" << GuhSettings(GuhSettings::SettingsRoleGlobal).fileName();
    m_configuration = new GuhConfiguration(this);

    qCDebug(dcApplication()) << "Creating Time Manager";
    m_timeManager = new TimeManager(QTimeZone::systemTimeZoneId(), this);

    qCDebug(dcApplication) << "Creating Log Engine";
    m_logger = new LogEngine(GuhSettings::logPath(), this);

    qCDebug(dcApplication) << "Creating Hardware Manager";
    m_hardwareManager = new HardwareManagerImplementation(this);

    qCDebug(dcApplication) << "Creating Device Manager (locale:" << m_configuration->locale() << ")";
    m_deviceManager = new DeviceManager(m_hardwareManager, m_configuration->locale(), this);

    qCDebug(dcApplication) << "Creating Rule Engine";
    m_ruleEngine = new RuleEngine(this);

    qCDebug(dcApplication()) << "Creating User Manager";
    m_userManager = new UserManager(this);

    qCDebug(dcApplication) << "Creating Server Manager";
    m_serverManager = new ServerManager(m_configuration, this);

    // Create the NetworkManager
    qCDebug(dcApplication) << "Creating Network Manager";
    m_networkManager = new NetworkManager(this);

    qCDebug(dcApplication) << "Creating Cloud Manager";
    m_cloudManager = new CloudManager(m_networkManager, this);
    m_cloudManager->setDeviceId(m_configuration->serverUuid());
    m_cloudManager->setDeviceName(m_configuration->serverName());
    m_cloudManager->setServerUrl(m_configuration->cloudServerUrl());
    m_cloudManager->setClientCertificates(m_configuration->cloudCertificateCA(), m_configuration->cloudCertificate(), m_configuration->cloudCertificateKey());
    m_cloudManager->setEnabled(m_configuration->cloudEnabled());

    connect(m_configuration, &GuhConfiguration::localeChanged, this, &GuhCore::onLocaleChanged);
    connect(m_configuration, &GuhConfiguration::cloudEnabledChanged, m_cloudManager, &CloudManager::setEnabled);
    connect(m_configuration, &GuhConfiguration::serverNameChanged, m_cloudManager, &CloudManager::setDeviceName);
    connect(m_configuration, &GuhConfiguration::serverNameChanged, m_serverManager, &ServerManager::setServerName);

    connect(m_deviceManager, &DeviceManager::pluginConfigChanged, this, &GuhCore::pluginConfigChanged);
    connect(m_deviceManager, &DeviceManager::eventTriggered, this, &GuhCore::gotEvent);
    connect(m_deviceManager, &DeviceManager::deviceStateChanged, this, &GuhCore::deviceStateChanged);
    connect(m_deviceManager, &DeviceManager::deviceAdded, this, &GuhCore::deviceAdded);
    connect(m_deviceManager, &DeviceManager::deviceChanged, this, &GuhCore::deviceChanged);
    connect(m_deviceManager, &DeviceManager::deviceRemoved, this, &GuhCore::deviceRemoved);
    connect(m_deviceManager, &DeviceManager::deviceDisappeared, this, &GuhCore::onDeviceDisappeared);
    connect(m_deviceManager, &DeviceManager::actionExecutionFinished, this, &GuhCore::actionExecutionFinished);
    connect(m_deviceManager, &DeviceManager::devicesDiscovered, this, &GuhCore::devicesDiscovered);
    connect(m_deviceManager, &DeviceManager::deviceSetupFinished, this, &GuhCore::deviceSetupFinished);
    connect(m_deviceManager, &DeviceManager::deviceReconfigurationFinished, this, &GuhCore::deviceReconfigurationFinished);
    connect(m_deviceManager, &DeviceManager::pairingFinished, this, &GuhCore::pairingFinished);
    connect(m_deviceManager, &DeviceManager::loaded, this, &GuhCore::deviceManagerLoaded);

    connect(m_ruleEngine, &RuleEngine::ruleAdded, this, &GuhCore::ruleAdded);
    connect(m_ruleEngine, &RuleEngine::ruleRemoved, this, &GuhCore::ruleRemoved);
    connect(m_ruleEngine, &RuleEngine::ruleConfigurationChanged, this, &GuhCore::ruleConfigurationChanged);

    connect(m_timeManager, &TimeManager::dateTimeChanged, this, &GuhCore::onDateTimeChanged);
    connect(m_timeManager, &TimeManager::tick, m_deviceManager, &DeviceManager::timeTick);

    m_logger->logSystemEvent(m_timeManager->currentDateTime(), true);

    emit initialized();

    // Evaluate rules on current time
    onDateTimeChanged(m_timeManager->currentDateTime());
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
        if (!rule.eventDescriptors().isEmpty()) {
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

void GuhCore::onDateTimeChanged(const QDateTime &dateTime)
{
    QList<RuleAction> actions;
    foreach (const Rule &rule, m_ruleEngine->evaluateTime(dateTime)) {
        // TimeEvent based
        if (!rule.timeDescriptor().timeEventItems().isEmpty()) {
            m_logger->logRuleTriggered(rule);
            foreach (const RuleAction &action, rule.actions()) {
                actions.append(action);
            }
        } else {
            // Calendar based rule
            m_logger->logRuleActiveChanged(rule);
            emit ruleActiveChanged(rule);
            if (rule.active()) {
                actions.append(rule.actions());
            } else {
                actions.append(rule.exitActions());
            }
        }
    }
    executeRuleActions(actions);
}

void GuhCore::onLocaleChanged()
{
    m_deviceManager->setLocale(m_configuration->locale());
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

void GuhCore::onDeviceDisappeared(const DeviceId &deviceId)
{
    Device *device = m_deviceManager->findConfiguredDevice(deviceId);
    if (!device) {
        return;
    }

    // Check if this device has child devices
    QList<Device *> devicesToRemove;
    devicesToRemove.append(device);
    QList<Device *> childDevices = m_deviceManager->findChildDevices(deviceId);
    if (!childDevices.isEmpty()) {
        foreach (Device *child, childDevices) {
            devicesToRemove.append(child);
        }
    }

    // check devices
    QList<RuleId> offendingRules;
    qCDebug(dcDeviceManager) << "Devices to remove:";
    foreach (Device *d, devicesToRemove) {
        qCDebug(dcDeviceManager) << " -> " << d->name() << d->id().toString();

        // Check if device is in a rule
        foreach (const RuleId &ruleId, m_ruleEngine->findRules(d->id())) {
            qCDebug(dcDeviceManager) << "      -> in rule:" << ruleId.toString();
            if (!offendingRules.contains(ruleId)) {
                offendingRules.append(ruleId);
            }
        }
    }

    // update involved rules
    foreach (const RuleId &ruleId, offendingRules) {
        foreach (Device *d, devicesToRemove) {
            m_ruleEngine->removeDeviceFromRule(ruleId, d->id());
        }
    }

    // remove the child devices
    foreach (Device *d, childDevices) {
        DeviceManager::DeviceError removeError = m_deviceManager->removeConfiguredDevice(d->id());
        if (removeError == DeviceManager::DeviceErrorNoError) {
            m_logger->removeDeviceLogs(d->id());
        }
    }

    // delete the device
    DeviceManager::DeviceError removeError = m_deviceManager->removeConfiguredDevice(deviceId);
    if (removeError == DeviceManager::DeviceErrorNoError) {
        m_logger->removeDeviceLogs(deviceId);
    }
}

void GuhCore::deviceManagerLoaded()
{
    // Do some houskeeping...
    qCDebug(dcApplication()) << "Starting housekeeping...";
    QDateTime startTime = QDateTime::currentDateTime();
    foreach (const DeviceId &deviceId, m_logger->devicesInLogs()) {
        if (!m_deviceManager->findConfiguredDevice(deviceId)) {
            qCDebug(dcApplication()) << "Cleaning stale device entries from log DB for device id" << deviceId;
            m_logger->removeDeviceLogs(deviceId);
        }
    }

    foreach (const DeviceId &deviceId, m_ruleEngine->devicesInRules()) {
        if (!m_deviceManager->findConfiguredDevice(deviceId)) {
            qCDebug(dcApplication()) << "Cleaning stale rule entries for device id" << deviceId;
            foreach (const RuleId &ruleId, m_ruleEngine->findRules(deviceId)) {
                m_ruleEngine->removeDeviceFromRule(ruleId, deviceId);
            }
        }
    }

    qCDebug(dcApplication()) << "Housekeeping done in" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms.";
}

}
