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

#ifndef NYMEACORE_H
#define NYMEACORE_H

#include "rule.h"
#include "types/event.h"
#include "types/deviceclass.h"
#include "plugin/deviceplugin.h"
#include "plugin/devicedescriptor.h"

#include "logging/logengine.h"
#include "devicemanager.h"
#include "ruleengine.h"
#include "servermanager.h"
#include "cloud/cloudmanager.h"

#include "time/timemanager.h"
#include "hardwaremanagerimplementation.h"

#include "debugserverhandler.h"

#include <QObject>

class Device;

namespace nymeaserver {

class JsonRPCServer;
class LogEngine;
class NetworkManager;
class NymeaConfiguration;
class TagsStorage;
class UserManager;
class Platform;
class System;

class NymeaCore : public QObject
{
    Q_OBJECT
    friend class NymeaTestBase;

public:
    static NymeaCore* instance();
    ~NymeaCore();

    void init();
    void destroy();

    // Device handling
    QPair<DeviceManager::DeviceError, QList<RuleId> >removeConfiguredDevice(const DeviceId &deviceId, const QHash<RuleId, RuleEngine::RemovePolicy> &removePolicyList);
    DeviceManager::DeviceError removeConfiguredDevice(const DeviceId &deviceId, const RuleEngine::RemovePolicy &removePolicy);

    DeviceManager::DeviceError executeAction(const Action &action);

    void executeRuleActions(const QList<RuleAction> ruleActions);

    RuleEngine::RuleError removeRule(const RuleId &id);

    NymeaConfiguration *configuration() const;
    LogEngine* logEngine() const;
    JsonRPCServer *jsonRPCServer() const;
    RestServer *restServer() const;
    DeviceManager *deviceManager() const;
    RuleEngine *ruleEngine() const;
    TimeManager *timeManager() const;
    ServerManager *serverManager() const;
    BluetoothServer *bluetoothServer() const;
    NetworkManager *networkManager() const;
    UserManager *userManager() const;
    CloudManager *cloudManager() const;
    DebugServerHandler *debugServerHandler() const;
    TagsStorage *tagsStorage() const;
    Platform *platform() const;

    static QStringList getAvailableLanguages();

signals:
    void initialized();

    void pluginConfigChanged(const PluginId &id, const ParamList &config);
    void eventTriggered(const Event &event);
    void deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value);
    void deviceRemoved(const DeviceId &deviceId);
    void deviceAdded(Device *device);
    void deviceChanged(Device *device);
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
    explicit NymeaCore(QObject *parent = nullptr);
    static NymeaCore *s_instance;

    Platform *m_platform = nullptr;

    NymeaConfiguration *m_configuration;
    ServerManager *m_serverManager;
    DeviceManager *m_deviceManager;
    RuleEngine *m_ruleEngine;
    LogEngine *m_logger;
    TimeManager *m_timeManager;
    CloudManager *m_cloudManager;
    HardwareManagerImplementation *m_hardwareManager;
    DebugServerHandler *m_debugServerHandler;
    TagsStorage *m_tagsStorage;

    NetworkManager *m_networkManager;
    UserManager *m_userManager;
    System *m_system;

    QHash<ActionId, Action> m_pendingActions;
    QList<RuleId> m_executingRules;

private slots:
    void gotEvent(const Event &event);
    void onDateTimeChanged(const QDateTime &dateTime);
    void actionExecutionFinished(const ActionId &id, DeviceManager::DeviceError status);
    void onDeviceDisappeared(const DeviceId &deviceId);
    void deviceManagerLoaded();

};

}

#endif // NYMEACORE_H
