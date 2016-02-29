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

#ifndef GUHCORE_H
#define GUHCORE_H

#include "rule.h"
#include "types/event.h"
#include "plugin/deviceplugin.h"
#include "plugin/deviceclass.h"
#include "plugin/devicedescriptor.h"

#include "devicemanager.h"
#include "ruleengine.h"
#include "servermanager.h"

#include <QObject>
#include <QDebug>

class Device;

namespace guhserver {

class JsonRPCServer;
class LogEngine;

class GuhCore : public QObject
{
    Q_OBJECT
    friend class GuhTestBase;

public:
    static GuhCore* instance();
    ~GuhCore();

    void destroy();

    // Device handling
    QPair<DeviceManager::DeviceError, QList<RuleId> >removeConfiguredDevice(const DeviceId &deviceId, const QHash<RuleId, RuleEngine::RemovePolicy> &removePolicyList);
    DeviceManager::DeviceError removeConfiguredDevice(const DeviceId &deviceId, const RuleEngine::RemovePolicy &removePolicy);

    DeviceManager::DeviceError executeAction(const Action &action);

    void executeRuleActions(const QList<RuleAction> ruleActions);

    RuleEngine::RuleError removeRule(const RuleId &id);

    LogEngine* logEngine() const;
    JsonRPCServer *jsonRPCServer() const;
    RestServer *restServer() const;
    DeviceManager *deviceManager() const;
    RuleEngine *ruleEngine() const;

signals:
    void eventTriggered(const Event &event);
    void deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value);
    void deviceRemoved(const DeviceId &deviceId);
    void deviceAdded(Device *device);
    void deviceParamsChanged(Device *device);
    void actionExecuted(const ActionId &id, DeviceManager::DeviceError status);

    void devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> deviceDescriptors);
    void deviceSetupFinished(Device *device, DeviceManager::DeviceError status);
    void deviceReconfigurationFinished(Device *device, DeviceManager::DeviceError status);
    void pairingFinished(const PairingTransactionId &pairingTransactionId, DeviceManager::DeviceError status, const DeviceId &deviceId);

    void ruleRemoved(const RuleId &ruleId);
    void ruleAdded(const Rule &rule);
    void ruleActiveChanged(const Rule &rule);
    void ruleConfigurationChanged(const Rule &rule);

private:
    explicit GuhCore(QObject *parent = 0);
    static GuhCore *s_instance;

    ServerManager *m_serverManager;
    DeviceManager *m_deviceManager;
    RuleEngine *m_ruleEngine;

    LogEngine *m_logger;

    QHash<ActionId, Action> m_pendingActions;

private slots:
    void gotEvent(const Event &event);
    void actionExecutionFinished(const ActionId &id, DeviceManager::DeviceError status);

};

}

#endif // GUHCORE_H
