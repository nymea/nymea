/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2018 Simon Stürz <simon.stuerz@guh.io>              *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::NymeaCore
    \brief The main entry point for the nymea Server and the place where all the messages are dispatched.

    \inmodule core

    NymeaCore is a singleton instance and the main entry point of the nymea daemon. It is responsible to
    instantiate, set up and connect all the other components.
*/

/*! \fn void nymeaserver::NymeaCore::eventTriggered(const Event &event);
    This signal is emitted when an \a event happend.
*/

/*! \fn void nymeaserver::NymeaCore::deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value);
    This signal is emitted when the \l{State} of a \a device changed. The \a stateTypeId parameter describes the
    \l{StateType} and the \a value parameter holds the new value.
*/

/*! \fn void nymeaserver::NymeaCore::deviceRemoved(const DeviceId &deviceId);
    This signal is emitted when a \l{Device} with the given \a deviceId was removed.
*/

/*! \fn void nymeaserver::NymeaCore::deviceAdded(Device *device);
    This signal is emitted when a \a device was added to the system.
*/

/*! \fn void nymeaserver::NymeaCore::deviceChanged(Device *device);
    This signal is emitted when the \l{ParamList}{Params} of a \a device have been changed.
*/

/*! \fn void nymeaserver::NymeaCore::actionExecuted(const ActionId &id, DeviceManager::DeviceError status);
    This signal is emitted when the \l{Action} with the given \a id is finished.
    The \a status of the \l{Action} execution will be described as \l{DeviceManager::DeviceError}{DeviceError}.
*/

/*! \fn void nymeaserver::NymeaCore::devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> deviceDescriptors);
    This signal is emitted when the discovery of a \a deviceClassId is finished. The \a deviceDescriptors parameter describes the
    list of \l{DeviceDescriptor}{DeviceDescriptors} of all discovered \l{Device}{Devices}.
    \sa DeviceManager::discoverDevices()
*/

/*! \fn void nymeaserver::NymeaCore::deviceSetupFinished(Device *device, DeviceManager::DeviceError status);
    This signal is emitted when the setup of a \a device is finished. The \a status parameter describes the
    \l{DeviceManager::DeviceError}{DeviceError} that occurred.
*/

/*! \fn void nymeaserver::NymeaCore::deviceReconfigurationFinished(Device *device, DeviceManager::DeviceError status);
    This signal is emitted when the edit request of a \a device is finished. The \a status of the edit request will be
    described as \l{DeviceManager::DeviceError}{DeviceError}.
*/

/*! \fn void nymeaserver::NymeaCore::pairingFinished(const PairingTransactionId &pairingTransactionId, DeviceManager::DeviceError status, const DeviceId &deviceId);
    The DeviceManager will emit a this Signal when the pairing of a \l{Device} with the \a deviceId and \a pairingTransactionId is finished.
    The \a status of the pairing will be described as \l{DeviceManager::DeviceError}{DeviceError}.
*/

/*! \fn void nymeaserver::NymeaCore::ruleRemoved(const RuleId &ruleId);
    This signal is emitted when a \l{Rule} with the given \a ruleId was removed.
*/

/*! \fn void nymeaserver::NymeaCore::ruleAdded(const Rule &rule);
    This signal is emitted when a \a rule was added to the system.
*/

/*! \fn void nymeaserver::NymeaCore::ruleConfigurationChanged(const Rule &rule);
    This signal is emitted when the configuration of \a rule changed.
*/

/*! \fn void nymeaserver::NymeaCore::initialized();
    This signal is emitted when the core is initialized.
*/

/*! \fn void nymeaserver::NymeaCore::pluginConfigChanged(const PluginId &id, const ParamList &config);
    This signal is emitted when the plugin \a config of the plugin with the given \a id changed.
*/


/*! \fn void ruleActiveChanged(const Rule &rule);
    This signal is emitted when a \a rule changed the active state.
    A \l{Rule} is active, when all \l{State}{States} match with the \l{StateDescriptor} conditions.

    \sa Rule::active()
*/

#include "nymeacore.h"
#include "loggingcategories.h"
#include "platform/platform.h"
#include "jsonrpc/jsonrpcserver.h"
#include "ruleengine.h"
#include "networkmanager/networkmanager.h"
#include "nymeasettings.h"
#include "tagging/tagsstorage.h"
#include "platform/platform.h"

#include "devicemanager.h"
#include "plugin/device.h"
#include "cloud/cloudnotifications.h"
#include "cloud/cloudtransport.h"

#include <QDir>
#include <QCoreApplication>

namespace nymeaserver {

NymeaCore* NymeaCore::s_instance = nullptr;

/*! Returns a pointer to the single \l{NymeaCore} instance. */
NymeaCore *NymeaCore::instance()
{
    if (!s_instance) {
        s_instance = new NymeaCore();
    }
    return s_instance;
}

/*! Constructs NymeaCore with the given \a parent. This is private.
    Use \l{NymeaCore::instance()} to access the single instance.*/
NymeaCore::NymeaCore(QObject *parent) :
    QObject(parent)
{
}

void NymeaCore::init() {
    qCDebug(dcApplication()) << "Initializing NymeaCore";

    qCDebug(dcPlatform()) << "Loading platform abstraction";
    m_platform = new Platform(this);

    qCDebug(dcApplication()) << "Loading nymea configurations" << NymeaSettings(NymeaSettings::SettingsRoleGlobal).fileName();
    m_configuration = new NymeaConfiguration(this);

    qCDebug(dcApplication()) << "Creating Time Manager";
    m_timeManager = new TimeManager(m_configuration->timeZone(), this);

    qCDebug(dcApplication) << "Creating Log Engine";
    m_logger = new LogEngine(m_configuration->logDBDriver(), m_configuration->logDBName(), m_configuration->logDBHost(), m_configuration->logDBUser(), m_configuration->logDBPassword(), m_configuration->logDBMaxEntries(), this);

    qCDebug(dcApplication()) << "Creating User Manager";
    m_userManager = new UserManager(NymeaSettings::settingsPath() + "/user-db.sqlite", this);

    qCDebug(dcApplication) << "Creating Server Manager";
    m_serverManager = new ServerManager(m_configuration, this);

    qCDebug(dcApplication) << "Creating Hardware Manager";
    m_hardwareManager = new HardwareManagerImplementation(m_serverManager->mqttBroker(), this);

    qCDebug(dcApplication) << "Creating Device Manager (locale:" << m_configuration->locale() << ")";
    m_deviceManager = new DeviceManager(m_hardwareManager, m_configuration->locale(), this);

    qCDebug(dcApplication) << "Creating Rule Engine";
    m_ruleEngine = new RuleEngine(this);

    qCDebug(dcApplication()) << "Creating Tags Storage";
    m_tagsStorage = new TagsStorage(m_deviceManager, m_ruleEngine, this);

    qCDebug(dcApplication) << "Creating Network Manager";
    m_networkManager = new NetworkManager(this);

    qCDebug(dcApplication) << "Creating Debug Server Handler";
    m_debugServerHandler = new DebugServerHandler(this);

    qCDebug(dcApplication) << "Creating Cloud Manager";
    m_cloudManager = new CloudManager(m_configuration, m_networkManager, this);

    CloudNotifications *cloudNotifications = m_cloudManager->createNotificationsPlugin();
    m_deviceManager->registerStaticPlugin(cloudNotifications, cloudNotifications->metaData());

    CloudTransport *cloudTransport = m_cloudManager->createTransportInterface();
    m_serverManager->jsonServer()->registerTransportInterface(cloudTransport, false);

    connect(m_configuration, &NymeaConfiguration::serverNameChanged, m_serverManager, &ServerManager::setServerName);

    connect(m_deviceManager, &DeviceManager::pluginConfigChanged, this, &NymeaCore::pluginConfigChanged);
    connect(m_deviceManager, &DeviceManager::eventTriggered, this, &NymeaCore::gotEvent);
    connect(m_deviceManager, &DeviceManager::deviceStateChanged, this, &NymeaCore::deviceStateChanged);
    connect(m_deviceManager, &DeviceManager::deviceAdded, this, &NymeaCore::deviceAdded);
    connect(m_deviceManager, &DeviceManager::deviceChanged, this, &NymeaCore::deviceChanged);
    connect(m_deviceManager, &DeviceManager::deviceRemoved, this, &NymeaCore::deviceRemoved);
    connect(m_deviceManager, &DeviceManager::deviceDisappeared, this, &NymeaCore::onDeviceDisappeared);
    connect(m_deviceManager, &DeviceManager::actionExecutionFinished, this, &NymeaCore::actionExecutionFinished);
    connect(m_deviceManager, &DeviceManager::devicesDiscovered, this, &NymeaCore::devicesDiscovered);
    connect(m_deviceManager, &DeviceManager::deviceSetupFinished, this, &NymeaCore::deviceSetupFinished);
    connect(m_deviceManager, &DeviceManager::deviceReconfigurationFinished, this, &NymeaCore::deviceReconfigurationFinished);
    connect(m_deviceManager, &DeviceManager::pairingFinished, this, &NymeaCore::pairingFinished);
    connect(m_deviceManager, &DeviceManager::loaded, this, &NymeaCore::deviceManagerLoaded);

    connect(m_ruleEngine, &RuleEngine::ruleAdded, this, &NymeaCore::ruleAdded);
    connect(m_ruleEngine, &RuleEngine::ruleRemoved, this, &NymeaCore::ruleRemoved);
    connect(m_ruleEngine, &RuleEngine::ruleConfigurationChanged, this, &NymeaCore::ruleConfigurationChanged);

    connect(m_timeManager, &TimeManager::dateTimeChanged, this, &NymeaCore::onDateTimeChanged);
    connect(m_timeManager, &TimeManager::tick, m_deviceManager, &DeviceManager::timeTick);

    m_logger->logSystemEvent(m_timeManager->currentDateTime(), true);
}

/*! Destructor of the \l{NymeaCore}. */
NymeaCore::~NymeaCore()
{
    m_logger->logSystemEvent(m_timeManager->currentDateTime(), false);

    // Disconnect everything that could still spawn events
    disconnect(m_deviceManager);
    disconnect(m_ruleEngine);
    disconnect(m_timeManager);

    // At very first, cut off the outside world
    qCDebug(dcApplication) << "Shutting down \"Server Manager\"";
    delete m_serverManager;
    qCDebug(dcApplication) << "Shutting down \"CloudManager\"";
    delete m_cloudManager;

    // Then stop magic from happening
    qCDebug(dcApplication) << "Shutting down \"Rule Engine\"";
    delete m_ruleEngine;

    // Next, DeviceManager, so plugins don't access any resources any more.
    qCDebug(dcApplication) << "Shutting down \"Device Manager\"";
    delete m_deviceManager;

    // Now go ahead and clean up stuff.
    qCDebug(dcApplication) << "Shutting down \"Log Engine\"";
    delete m_logger;

    qCDebug(dcApplication()) << "Shutting down \"Hardware Manager\"";
    delete m_hardwareManager;

    qCDebug(dcApplication) << "Done shutting down NymeaCore";
}

/*! Destroyes the \l{NymeaCore} instance. */
void NymeaCore::destroy()
{
    if (s_instance) {
        delete s_instance;
    }

    s_instance = nullptr;
}

/*! Removes a configured \l{Device} with the given \a deviceId and \a removePolicyList. */
QPair<DeviceManager::DeviceError, QList<RuleId> > NymeaCore::removeConfiguredDevice(const DeviceId &deviceId, const QHash<RuleId, RuleEngine::RemovePolicy> &removePolicyList)
{
    Device *device = m_deviceManager->findConfiguredDevice(deviceId);

    if (!device) {
        return QPair<DeviceManager::DeviceError, QList<RuleId> > (DeviceManager::DeviceErrorDeviceNotFound, QList<RuleId>());
    }

    // Check if this is a child device
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
DeviceManager::DeviceError NymeaCore::removeConfiguredDevice(const DeviceId &deviceId, const RuleEngine::RemovePolicy &removePolicy)
{
    Device *device = m_deviceManager->findConfiguredDevice(deviceId);

    if (!device) {
        return DeviceManager::DeviceErrorDeviceNotFound;
    }

    // Check if this is a child device
    if (!device->parentId().isNull()) {
        qCWarning(dcDeviceManager) << "The device is a child of" << device->parentId().toString() << ". Please remove the parent device.";
        return DeviceManager::DeviceErrorDeviceIsChild;
    }

    // FIXME: Let's remove this for now. It will come back with more fine grained control, presumably introducing a RemoveMethod flag in the DeviceClass
//    if (device->autoCreated()) {
//        qCWarning(dcDeviceManager) << "This device has been auto-created and cannot be deleted manually.";
//        return DeviceManager::DeviceErrorCreationMethodNotSupported;
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
DeviceManager::DeviceError NymeaCore::executeAction(const Action &action)
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
void NymeaCore::executeRuleActions(const QList<RuleAction> ruleActions)
{
    QList<Action> actions;
    foreach (const RuleAction &ruleAction, ruleActions) {
        if (ruleAction.type() == RuleAction::TypeDevice) {
            Device *device = m_deviceManager->findConfiguredDevice(ruleAction.deviceId());
            ActionTypeId actionTypeId = ruleAction.actionTypeId();
            ParamList params;
            bool ok = true;
            foreach (const RuleActionParam &ruleActionParam, ruleAction.ruleActionParams()) {
                if (ruleActionParam.isValueBased()) {
                    params.append(Param(ruleActionParam.paramTypeId(), ruleActionParam.value()));
                } else if (ruleActionParam.isStateBased()) {
                    Device *stateDevice = m_deviceManager->findConfiguredDevice(ruleActionParam.stateDeviceId());
                    if (!stateDevice) {
                        qCWarning(dcRuleEngine()) << "Cannot find device" << ruleActionParam.stateDeviceId() << "required by rule action" << ruleAction.id();
                        ok = false;
                        break;
                    }
                    DeviceClass stateDeviceClass = m_deviceManager->findDeviceClass(stateDevice->deviceClassId());
                    if (!stateDeviceClass.hasStateType(ruleActionParam.stateTypeId())) {
                        qCWarning(dcRuleEngine()) << "Device" << device->name() << device->id() << "does not have a state type" << ruleActionParam.stateTypeId();
                        ok = false;
                        break;
                    }
                    params.append(Param(ruleActionParam.paramTypeId(), stateDevice->stateValue(ruleActionParam.stateTypeId())));
                }
            }
            if (!ok) {
                qCWarning(dcRuleEngine()) << "Not executing rule action" << ruleAction.id();
                continue;
            }
            Action action(actionTypeId, device->id());
            action.setParams(params);
            actions.append(action);
        } else {
            QList<Device*> devices = m_deviceManager->findConfiguredDevices(ruleAction.interface());
            foreach (Device* device, devices) {
                DeviceClass deviceClass = m_deviceManager->findDeviceClass(device->deviceClassId());
                ActionType actionType = deviceClass.actionTypes().findByName(ruleAction.interfaceAction());
                if (actionType.id().isNull()) {
                    qCWarning(dcRuleEngine()) << "Error creating Action. The given DeviceClass does not implement action:" << ruleAction.interfaceAction();
                    continue;
                }

                ParamList params;
                bool ok = true;
                foreach (const RuleActionParam &ruleActionParam, ruleAction.ruleActionParams()) {
                    ParamType paramType = actionType.paramTypes().findByName(ruleActionParam.paramName());
                    if (paramType.id().isNull()) {
                        qCWarning(dcRuleEngine()) << "Error creating Action. The given ActionType does not have a parameter:" << ruleActionParam.paramName();
                        ok = false;
                        continue;
                    }
                    if (ruleActionParam.isValueBased()) {
                        params.append(Param(paramType.id(), ruleActionParam.value()));
                    } else if (ruleActionParam.isStateBased()) {
                        Device *stateDevice = m_deviceManager->findConfiguredDevice(ruleActionParam.stateDeviceId());
                        if (!stateDevice) {
                            qCWarning(dcRuleEngine()) << "Cannot find device" << ruleActionParam.stateDeviceId() << "required by rule action" << ruleAction.id();
                            ok = false;
                            break;
                        }
                        DeviceClass stateDeviceClass = m_deviceManager->findDeviceClass(stateDevice->deviceClassId());
                        if (!stateDeviceClass.hasStateType(ruleActionParam.stateTypeId())) {
                            qCWarning(dcRuleEngine()) << "Device" << device->name() << device->id() << "does not have a state type" << ruleActionParam.stateTypeId();
                            ok = false;
                            break;
                        }
                        params.append(Param(paramType.id(), stateDevice->stateValue(ruleActionParam.stateTypeId())));
                    }
                }
                if (!ok) {
                    qCWarning(dcRuleEngine()) << "Not executing rule action" << ruleAction.id();
                    continue;
                }

                Action action = Action(actionType.id(), device->id());
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
RuleEngine::RuleError NymeaCore::removeRule(const RuleId &id)
{
    RuleEngine::RuleError removeError = m_ruleEngine->removeRule(id);
    if (removeError == RuleEngine::RuleErrorNoError)
        m_logger->removeRuleLogs(id);

    return removeError;
}

/*! Returns a pointer to the \l{NymeaConfiguration} instance owned by NymeaCore.*/
NymeaConfiguration *NymeaCore::configuration() const
{
    return m_configuration;
}

/*! Returns a pointer to the \l{DeviceManager} instance owned by NymeaCore.*/
DeviceManager *NymeaCore::deviceManager() const
{
    return m_deviceManager;
}

/*! Returns a pointer to the \l{RuleEngine} instance owned by NymeaCore.*/
RuleEngine *NymeaCore::ruleEngine() const
{
    return m_ruleEngine;
}

/*! Returns a pointer to the \l{TimeManager} instance owned by NymeaCore.*/
TimeManager *NymeaCore::timeManager() const
{
    return m_timeManager;
}

/*! Returns a pointer to the \l{ServerManager} instance owned by NymeaCore. */
ServerManager *NymeaCore::serverManager() const
{
    return m_serverManager;
}

/*! Returns the list of available system languages. */
QStringList NymeaCore::getAvailableLanguages()
{
    qCDebug(dcApplication()) << "Loading translations from" << NymeaSettings::translationsPath();

    QStringList searchPaths;
    searchPaths << QCoreApplication::applicationDirPath() + "/../translations";
    searchPaths << NymeaSettings::translationsPath();

    QStringList translationFiles;
    foreach (const QString &path, searchPaths) {
        QDir translationDirectory(path);
        translationDirectory.setNameFilters(QStringList() << "*.qm");
        translationFiles = translationDirectory.entryList();
        qCDebug(dcTranslations()) << translationFiles.count() << "translations in" << path;
        if (translationFiles.count() > 0) {
            break;
        }
    }

    QStringList availableLanguages;
    foreach (QString translationFile, translationFiles) {
        if (!translationFile.startsWith("nymead-"))
            continue;

        QString language = translationFile.remove("nymead-").remove(".qm");
        QLocale languageLocale(language);
        availableLanguages.append(languageLocale.name());
    }

    return availableLanguages;
}

/*! Returns a pointer to the \l{BluetoothServer} instance owned by NymeaCore. */
BluetoothServer *NymeaCore::bluetoothServer() const
{
    return m_serverManager->bluetoothServer();
}

/*! Returns a pointer to the \l{NetworkManager} instance owned by NymeaCore. */
NetworkManager *NymeaCore::networkManager() const
{
    return m_networkManager;
}

/*! Returns a pointer to the \l{UserManager} instance owned by NymeaCore. */
UserManager *NymeaCore::userManager() const
{
    return m_userManager;
}

/*! Returns a pointer to the CloudManager instance owned by NymeaCore. */
CloudManager *NymeaCore::cloudManager() const
{
    return m_cloudManager;
}

/*! Returns a pointer to the \l{DebugServerHandler} instance owned by NymeaCore. */
DebugServerHandler *NymeaCore::debugServerHandler() const
{
    return m_debugServerHandler;
}

/*! Returns a pointer to the \l{TagsStorage} instance owned by NymeaCore. */
TagsStorage *NymeaCore::tagsStorage() const
{
    return m_tagsStorage;
}

/*! Returns a pointer to the \l{Platform} instance owned by NymeaCore.
    The Platform represents the host system this nymea instance is running on.
*/
Platform *NymeaCore::platform() const
{
    return m_platform;
}


/*! Connected to the DeviceManager's emitEvent signal. Events received in
    here will be evaluated by the \l{RuleEngine} and the according \l{RuleAction}{RuleActions} are executed.*/
void NymeaCore::gotEvent(const Event &event)
{
    m_logger->logEvent(event);
    emit eventTriggered(event);

    QList<RuleAction> actions;
    QList<RuleAction> eventBasedActions;
    foreach (const Rule &rule, m_ruleEngine->evaluateEvent(event)) {
        if (m_executingRules.contains(rule.id())) {
            qCWarning(dcRuleEngine()) << "WARNING: Loop detected in rule execution for rule" << rule.id() << rule.name();
            break;
        }
        m_executingRules.append(rule.id());

        // Event based
        if (!rule.eventDescriptors().isEmpty()) {
            m_logger->logRuleTriggered(rule);
            QList<RuleAction> tmp;
            if (rule.statesActive() && rule.timeActive()) {
                qCDebug(dcRuleEngineDebug()) << "Executing actions";
                tmp = rule.actions();
            } else {
                qCDebug(dcRuleEngineDebug()) << "Executing exitActions";
                tmp = rule.exitActions();
            }
            // check if we have an event based action or a normal action
            foreach (const RuleAction &action, tmp) {
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
                QVariant eventValue = event.params().paramValue(ruleActionParam.eventParamTypeId());

                // TODO: limits / scale calculation -> actionValue = eventValue * x
                //       something like a EventParamDescriptor

                ruleActionParam.setValue(eventValue);
                qCDebug(dcRuleEngine) << "Using param value from event:" << ruleActionParam.value();
            }
            newParams.append(ruleActionParam);
        }
        ruleAction.setRuleActionParams(newParams);
        actions.append(ruleAction);
    }

    executeRuleActions(actions);
    m_executingRules.clear();
}

void NymeaCore::onDateTimeChanged(const QDateTime &dateTime)
{
    QList<RuleAction> actions;
    foreach (const Rule &rule, m_ruleEngine->evaluateTime(dateTime)) {
        // TimeEvent based
        if (!rule.timeDescriptor().timeEventItems().isEmpty()) {
            m_logger->logRuleTriggered(rule);
            if (rule.statesActive() && rule.timeActive()) {
                actions.append(rule.actions());
            } else {
                actions.append(rule.exitActions());
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

/*! Return the instance of the log engine */
LogEngine* NymeaCore::logEngine() const
{
    return m_logger;
}

/*! Returns the pointer to the \l{JsonRPCServer} of this instance. */
JsonRPCServer *NymeaCore::jsonRPCServer() const
{
    return m_serverManager->jsonServer();
}

/*! Returns the pointer to the \l{RestServer} of this instance. */
RestServer *NymeaCore::restServer() const
{
    return m_serverManager->restServer();
}

void NymeaCore::actionExecutionFinished(const ActionId &id, DeviceManager::DeviceError status)
{
    emit actionExecuted(id, status);
    Action action = m_pendingActions.take(id);
    m_logger->logAction(action, status == DeviceManager::DeviceErrorNoError ? Logging::LoggingLevelInfo : Logging::LoggingLevelAlert, status);
}

void NymeaCore::onDeviceDisappeared(const DeviceId &deviceId)
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

void NymeaCore::deviceManagerLoaded()
{
    m_ruleEngine->init();
    // Evaluate rules on current time
    onDateTimeChanged(m_timeManager->currentDateTime());

    emit initialized();

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
