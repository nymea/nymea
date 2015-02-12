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

#ifndef GUHCORE_H
#define GUHCORE_H

#include "rule.h"
#include "types/event.h"
#include "plugin/deviceclass.h"
#include "plugin/devicedescriptor.h"

#include "devicemanager.h"
#include "ruleengine.h"

#include <QObject>

class JsonRPCServer;
class Device;
class LogEngine;

class GuhCore : public QObject
{
    Q_OBJECT
public:
    enum RunningMode {
        RunningModeApplication,
        RunningModeService
    };

    static GuhCore* instance();
    ~GuhCore();

    // Used for testing
    void destroy();

    RunningMode runningMode() const;
    void setRunningMode(const RunningMode &runningMode);

    QList<DevicePlugin *> plugins() const;
    DeviceManager::DeviceError setPluginConfig(const PluginId &pluginId, const ParamList &params);

    // Device handling
    QList<Vendor> supportedVendors() const;
    QList<DeviceClass> supportedDevices(const VendorId &vendorId = VendorId()) const;
    DeviceClass findDeviceClass(const DeviceClassId &deviceClassId) const;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params);
    DeviceManager::DeviceError addConfiguredDevice(const DeviceClassId &deviceClassId, const ParamList &params, const DeviceId &newId);
    DeviceManager::DeviceError addConfiguredDevice(const DeviceClassId &deviceClassId, const DeviceDescriptorId &deviceDescriptorId, const DeviceId &newId);
    QList<Device*> configuredDevices() const;
    Device *findConfiguredDevice(const DeviceId &deviceId) const;
    QList<Device*> findConfiguredDevices(const DeviceClassId &deviceClassId) const;
    DeviceManager::DeviceError removeConfiguredDevice(const DeviceId &deviceId, const QHash<RuleId, RuleEngine::RemovePolicy> &removePolicyList);

    DeviceManager::DeviceError pairDevice(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const DeviceDescriptorId &deviceDescriptorId);
    DeviceManager::DeviceError pairDevice(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params);
    DeviceManager::DeviceError confirmPairing(const PairingTransactionId &pairingTransactionId, const QString &secret = QString());

    DeviceManager::DeviceError executeAction(const Action &action);

    QList<Rule> rules() const;
    QList<RuleId> ruleIds() const;
    Rule findRule(const RuleId &ruleId);
    RuleEngine::RuleError addRule(const RuleId &id, const QList<EventDescriptor> &eventDescriptorList, const StateEvaluator &stateEvaluator, const QList<Action> &actionList, bool enabled = true);
    RuleEngine::RuleError removeRule(const RuleId &id);
    QList<RuleId> findRules(const DeviceId &deviceId);
    RuleEngine::RuleError enableRule(const RuleId &ruleId);
    RuleEngine::RuleError disableRule(const RuleId &ruleId);

    LogEngine* logEngine() const;

signals:
    void eventTriggered(const Event &event);
    void deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value);
    void actionExecuted(const ActionId &id, DeviceManager::DeviceError status);

    void devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> deviceDescriptors);
    void deviceSetupFinished(Device *device, DeviceManager::DeviceError status);
    void pairingFinished(const PairingTransactionId &pairingTransactionId, DeviceManager::DeviceError status, const DeviceId &deviceId);

private:
    RuleEngine *ruleEngine() const;
    DeviceManager* deviceManager() const;
    explicit GuhCore(QObject *parent = 0);
    static GuhCore *s_instance;
    RunningMode m_runningMode;

    JsonRPCServer *m_jsonServer;
    DeviceManager *m_deviceManager;
    RuleEngine *m_ruleEngine;

    LogEngine *m_logger;

    QHash<ActionId, Action> m_pendingActions;
private slots:
    void gotEvent(const Event &event);
    void actionExecutionFinished(const ActionId &id, DeviceManager::DeviceError status);

    friend class GuhTestBase;
};

#endif // GUHCORE_H
