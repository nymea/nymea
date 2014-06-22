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

#include "guhcore.h"
#include "jsonrpcserver.h"
#include "ruleengine.h"

#include "devicemanager.h"
#include "plugin/device.h"

#include <QDebug>

GuhCore* GuhCore::s_instance = 0;

/*! Returns a pointer to the single \l{GuhCore} instance.*/
GuhCore *GuhCore::instance()
{
    if (!s_instance) {
        s_instance = new GuhCore();
    }
    return s_instance;
}

GuhCore::~GuhCore()
{
    qDebug() << "Shutting down. Bye.";
}

void GuhCore::destroy()
{
    delete s_instance;
    s_instance = 0;
}

QList<DevicePlugin *> GuhCore::plugins() const
{
    return m_deviceManager->plugins();
}

QPair<DeviceManager::DeviceError, QString> GuhCore::setPluginConfig(const PluginId &pluginId, const QList<Param> params)
{
    return m_deviceManager->setPluginConfig(pluginId, params);
}

QList<Vendor> GuhCore::supportedVendors() const
{
    return m_deviceManager->supportedVendors();
}

QList<DeviceClass> GuhCore::supportedDevices(const VendorId &vendorId) const
{
    return m_deviceManager->supportedDevices(vendorId);
}

QPair<DeviceManager::DeviceError, QString> GuhCore::removeConfiguredDevice(const DeviceId &deviceId, const QHash<RuleId, RuleEngine::RemovePolicy> &removePolicyList)
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
        return qMakePair<DeviceManager::DeviceError, QString>(DeviceManager::DeviceErrorMissingParameter, "There are unhandled rules which depend on this device.");
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

QPair<DeviceManager::DeviceError, QString> GuhCore::pairDevice(const DeviceClassId &deviceClassId, const DeviceDescriptorId &deviceDescriptorId)
{
    return m_deviceManager->pairDevice(deviceClassId, deviceDescriptorId);
}

QPair<DeviceManager::DeviceError, QString> GuhCore::pairDevice(const DeviceClassId &deviceClassId, const QList<Param> &params)
{
    return m_deviceManager->pairDevice(deviceClassId, params);
}

QPair<DeviceManager::DeviceError, QString> GuhCore::confirmPairing(const QUuid &pairingTransactionId, const QString &secret)
{
    return m_deviceManager->confirmPairing(pairingTransactionId, secret);
}

QPair<DeviceManager::DeviceError, QString> GuhCore::executeAction(const Action &action)
{
    return m_deviceManager->executeAction(action);
}

DeviceClass GuhCore::findDeviceClass(const DeviceClassId &deviceClassId) const
{
    return m_deviceManager->findDeviceClass(deviceClassId);
}

DeviceManager::DeviceError GuhCore::discoverDevices(const DeviceClassId &deviceClassId, const QList<Param> &params)
{
    return m_deviceManager->discoverDevices(deviceClassId, params);
}

QPair<DeviceManager::DeviceError, QString> GuhCore::addConfiguredDevice(const DeviceClassId &deviceClassId, const QList<Param> &params, const DeviceId &newId)
{
    return m_deviceManager->addConfiguredDevice(deviceClassId, params, newId);
}

QPair<DeviceManager::DeviceError, QString> GuhCore::addConfiguredDevice(const DeviceClassId &deviceClassId, const DeviceDescriptorId &deviceDescriptorId, const DeviceId &newId)
{
    return m_deviceManager->addConfiguredDevice(deviceClassId, deviceDescriptorId, newId);
}

QList<Device *> GuhCore::configuredDevices() const
{
    return m_deviceManager->configuredDevices();
}

Device *GuhCore::findConfiguredDevice(const DeviceId &deviceId) const
{
    return m_deviceManager->findConfiguredDevice(deviceId);
}

QList<Device *> GuhCore::findConfiguredDevices(const DeviceClassId &deviceClassId) const
{
    return m_deviceManager->findConfiguredDevices(deviceClassId);
}

QList<Rule> GuhCore::rules() const
{
    return m_ruleEngine->rules();
}

QList<RuleId> GuhCore::ruleIds() const
{
    return m_ruleEngine->ruleIds();
}

Rule GuhCore::findRule(const RuleId &ruleId)
{
    return m_ruleEngine->findRule(ruleId);
}

RuleEngine::RuleError GuhCore::addRule(const RuleId &id, const QList<EventDescriptor> &eventDescriptorList, const QList<Action> &actionList)
{
    return m_ruleEngine->addRule(id, eventDescriptorList, actionList);
}

RuleEngine::RuleError GuhCore::removeRule(const RuleId &id)
{
    return m_ruleEngine->removeRule(id);
}

QList<RuleId> GuhCore::findRules(const DeviceId &deviceId)
{
    return m_ruleEngine->findRules(deviceId);
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
        QPair<DeviceManager::DeviceError, QString> status = m_deviceManager->executeAction(action);
        switch(status.first) {
        case DeviceManager::DeviceErrorNoError:
            break;
        case DeviceManager::DeviceErrorSetupFailed:
            qDebug() << "Error executing action. Device setup failed:" << status.second;
            break;
        case DeviceManager::DeviceErrorActionParameterError:
            qDebug() << "Error executing action. Invalid action parameter:" << status.second;
            break;
        default:
            qDebug() << "Error executing action:" << status.first << status.second;
        }
    }
}
