/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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
    \class GuhCore
    \brief The main entry point for the Guh Server and the place where all the messages are dispatched.

    \ingroup core
    \inmodule server

    GuhCore is a singleton instance and the main entry point of the Guh daemon. It is responsible to
    instantiate, set up and connect all the other components.
*/

/*! \fn void GuhCore::eventTriggered(const Event &event);
    This signal is emitted when an \a event happend.
*/

/*! \fn void GuhCore::deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value);
    This signal is emitted when the \l{State} of a \a device changed. The \a stateTypeId parameter describes the
    \l{StateType} and the \a value parameter holds the new value.
*/

/*! \fn void GuhCore::actionExecuted(const ActionId &id, DeviceManager::DeviceError status);
    This signal is emitted when the \l{Action} with the given \a id is finished.
    The \a status of the \l{Action} execution will be described as \l{DeviceManager::DeviceError}{DeviceError}.
*/

/*! \fn void GuhCore::devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> deviceDescriptors);
    This signal is emitted when the discovery of a \a deviceClassId is finished. The \a deviceDescriptors parameter describes the
    list of \l{DeviceDescriptor}{DeviceDescriptors} of all discovered \l{Device}{Devices}.
    \sa discoverDevices()
*/

/*! \fn void GuhCore::deviceSetupFinished(Device *device, DeviceManager::DeviceError status);
    This signal is emitted when the setup of a \a device is finished. The \a status parameter describes the
    \l{DeviceManager::DeviceError}{DeviceError} that occurred.
*/

/*! \fn void GuhCore::pairingFinished(const PairingTransactionId &pairingTransactionId, DeviceManager::DeviceError status, const DeviceId &deviceId);
    The DeviceManager will emit a this Signal when the pairing of a \l{Device} with the \a deviceId and \a pairingTransactionId is finished.
    The \a status of the pairing will be described as \l{DeviceManager::DeviceError}{DeviceError}.
*/

#include "guhcore.h"
#include "jsonrpcserver.h"
#include "ruleengine.h"

#include "devicemanager.h"
#include "plugin/device.h"

#include <QDebug>

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
    qDebug() << "Shutting down. Bye.";
}

/*! Destroyes the \l{GuhCore} instance. */
void GuhCore::destroy()
{
    delete s_instance;
    s_instance = 0;
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
        qWarning() << "There are unhandled rules which depend on this device.";
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

    return m_deviceManager->removeConfiguredDevice(deviceId);
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
    return m_deviceManager->executeAction(action);
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

/*! Calls the metheod RuleEngine::rule().
 *  \sa RuleEngine, */
QList<Rule> GuhCore::rules() const
{
    return m_ruleEngine->rules();
}

/*! Calls the metheod RuleEngine::ruleIds().
 *  \sa RuleEngine, */
QList<RuleId> GuhCore::ruleIds() const
{
    return m_ruleEngine->ruleIds();
}

/*! Calls the metheod RuleEngine::findRule(\a ruleId).
 *  \sa RuleEngine, */
Rule GuhCore::findRule(const RuleId &ruleId)
{
    return m_ruleEngine->findRule(ruleId);
}

/*! Calls the metheod RuleEngine::addRule(\a id, \a eventDescriptorList, \a actionList, \a enabled).
 *  \sa RuleEngine, */
RuleEngine::RuleError GuhCore::addRule(const RuleId &id, const QList<EventDescriptor> &eventDescriptorList, const QList<Action> &actionList, bool enabled)
{
    return m_ruleEngine->addRule(id, eventDescriptorList, actionList, enabled);
}

/*! Calls the metheod RuleEngine::removeRule(\a id).
 *  \sa RuleEngine, */
RuleEngine::RuleError GuhCore::removeRule(const RuleId &id)
{
    return m_ruleEngine->removeRule(id);
}

/*! Calls the metheod RuleEngine::findRules(\a deviceId).
 *  \sa RuleEngine, */
QList<RuleId> GuhCore::findRules(const DeviceId &deviceId)
{
    return m_ruleEngine->findRules(deviceId);
}

/*! Calls the metheod RuleEngine::enableRule(\a ruleId).
 *  \sa RuleEngine, */
RuleEngine::RuleError GuhCore::enableRule(const RuleId &ruleId)
{
    return m_ruleEngine->enableRule(ruleId);
}

/*! Calls the metheod RuleEngine::disableRule(\a ruleId).
 *  \sa RuleEngine, */
RuleEngine::RuleError GuhCore::disableRule(const RuleId &ruleId)
{
    return m_ruleEngine->disableRule(ruleId);
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
    qDebug() << "*****************************************";
    qDebug() << "* GUH version:" << GUH_VERSION_STRING << "starting up.       *";
    qDebug() << "*****************************************";

    qDebug() << "*****************************************";
    qDebug() << "* Creating Device Manager               *";
    qDebug() << "*****************************************";
    m_deviceManager = new DeviceManager(this);

    qDebug() << "*****************************************";
    qDebug() << "* Creating Rule Engine                  *";
    qDebug() << "*****************************************";
    m_ruleEngine = new RuleEngine(this);

    qDebug() << "*****************************************";
    qDebug() << "* Starting JSON RPC Server              *";
    qDebug() << "*****************************************";
    m_jsonServer = new JsonRPCServer(this);

    connect(m_deviceManager, &DeviceManager::eventTriggered, this, &GuhCore::gotEvent);
    connect(m_deviceManager, &DeviceManager::deviceStateChanged, this, &GuhCore::deviceStateChanged);
    connect(m_deviceManager, &DeviceManager::actionExecutionFinished, this, &GuhCore::actionExecuted);

    connect(m_deviceManager, &DeviceManager::devicesDiscovered, this, &GuhCore::devicesDiscovered);
    connect(m_deviceManager, &DeviceManager::deviceSetupFinished, this, &GuhCore::deviceSetupFinished);
    connect(m_deviceManager, &DeviceManager::pairingFinished, this, &GuhCore::pairingFinished);
}

/*! Connected to the DeviceManager's emitEvent signal. Events received in
    here will be evaluated by the \l{RuleEngine} and the according \l{Action}{Actions} are executed.*/
void GuhCore::gotEvent(const Event &event)
{
    // first inform other things about it.
    emit eventTriggered(event);

    // Now execute all the associated rules
    foreach (const Action &action, m_ruleEngine->evaluateEvent(event)) {
        qDebug() << "executing action" << action.actionTypeId();
        DeviceManager::DeviceError status = m_deviceManager->executeAction(action);
        switch(status) {
        case DeviceManager::DeviceErrorNoError:
            break;
        case DeviceManager::DeviceErrorSetupFailed:
            qDebug() << "Error executing action. Device setup failed.";
            break;
        case DeviceManager::DeviceErrorInvalidParameter:
            qDebug() << "Error executing action. Invalid action parameter.";
            break;
        default:
            qDebug() << "Error executing action:" << status;
        }
    }
}
