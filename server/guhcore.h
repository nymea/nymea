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

class GuhCore : public QObject
{
    Q_OBJECT
public:
    static GuhCore* instance();
    ~GuhCore();

    // Used for testing
    void destroy();

    QList<DevicePlugin *> plugins() const;
    QPair<DeviceManager::DeviceError, QString> setPluginConfig(const PluginId &pluginId, const QList<Param> params);

    // Device handling
    QList<Vendor> supportedVendors() const;
    QList<DeviceClass> supportedDevices(const VendorId &vendorId = VendorId()) const;
    DeviceClass findDeviceClass(const DeviceClassId &deviceClassId) const;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const QList<Param> &params);
    QPair<DeviceManager::DeviceError, QString> addConfiguredDevice(const DeviceClassId &deviceClassId, const QList<Param> &params, const DeviceId &newId);
    QPair<DeviceManager::DeviceError, QString> addConfiguredDevice(const DeviceClassId &deviceClassId, const DeviceDescriptorId &deviceDescriptorId, const DeviceId &newId);
    QList<Device*> configuredDevices() const;
    Device *findConfiguredDevice(const DeviceId &deviceId) const;
    QList<Device*> findConfiguredDevices(const DeviceClassId &deviceClassId) const;
    QPair<DeviceManager::DeviceError, QString> removeConfiguredDevice(const DeviceId &deviceId, const QHash<RuleId, RuleEngine::RemovePolicy> &removePolicyList);

    QPair<DeviceManager::DeviceError, QString> pairDevice(const DeviceClassId &deviceClassId, const DeviceDescriptorId &deviceDescriptorId);
    QPair<DeviceManager::DeviceError, QString> pairDevice(const DeviceClassId &deviceClassId, const QList<Param> &params);
    QPair<DeviceManager::DeviceError, QString> confirmPairing(const QUuid &pairingTransactionId, const QString &secret = QString());

    QPair<DeviceManager::DeviceError, QString> executeAction(const Action &action);

    QList<Rule> rules() const;
    QList<RuleId> ruleIds() const;
    Rule findRule(const RuleId &ruleId);
    RuleEngine::RuleError addRule(const RuleId &id, const QList<EventDescriptor> &eventDescriptorList, const QList<Action> &actionList);
    RuleEngine::RuleError removeRule(const RuleId &id);
    QList<RuleId> findRules(const DeviceId &deviceId);

signals:
    void eventTriggered(const Event &event);
    void deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value);
    void actionExecuted(const ActionId &id, DeviceManager::DeviceError status, const QString &errorMessage);

    void devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> deviceDescriptors);
    void deviceSetupFinished(Device *device, DeviceManager::DeviceError status);
    void pairingFinished(const QUuid &pairingTransactionId, DeviceManager::DeviceError status, const QString &errorMessage, const DeviceId &deviceId);

private:
    RuleEngine *ruleEngine() const;
    DeviceManager* deviceManager() const;
    explicit GuhCore(QObject *parent = 0);
    static GuhCore *s_instance;

    JsonRPCServer *m_jsonServer;
    DeviceManager *m_deviceManager;
    RuleEngine *m_ruleEngine;

private slots:
    void gotEvent(const Event &event);

    friend class GuhTestBase;
};

#endif // GUHCORE_H
