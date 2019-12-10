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

#include "types/event.h"
#include "types/deviceclass.h"
#include "devices/deviceplugin.h"
#include "devices/devicedescriptor.h"
#include "devices/devicemanagerimplementation.h"

#include "ruleengine/rule.h"
#include "ruleengine/ruleengine.h"

#include "logging/logengine.h"
#include "servermanager.h"
#include "cloud/cloudmanager.h"

#include "time/timemanager.h"
#include "hardwaremanagerimplementation.h"

#include "debugserverhandler.h"

#include <QObject>

class Device;

namespace nymeaserver {

class JsonRPCServerImplementation;
class LogEngine;
class NetworkManager;
class NymeaConfiguration;
class TagsStorage;
class UserManager;
class Platform;
class System;
class ExperienceManager;
class ScriptEngine;

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
    QPair<Device::DeviceError, QList<RuleId> >removeConfiguredDevice(const DeviceId &deviceId, const QHash<RuleId, RuleEngine::RemovePolicy> &removePolicyList);
    Device::DeviceError removeConfiguredDevice(const DeviceId &deviceId, const RuleEngine::RemovePolicy &removePolicy);

    DeviceActionInfo *executeAction(const Action &action);
    BrowserActionInfo* executeBrowserItem(const BrowserAction &browserAction);
    BrowserItemActionInfo* executeBrowserItemAction(const BrowserItemAction &browserItemAction);

    void executeRuleActions(const QList<RuleAction> ruleActions);

    RuleEngine::RuleError removeRule(const RuleId &id);

    NymeaConfiguration *configuration() const;
    LogEngine* logEngine() const;
    JsonRPCServerImplementation *jsonRPCServer() const;
    DeviceManager *deviceManager() const;
    RuleEngine *ruleEngine() const;
    ScriptEngine *scriptEngine() const;
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
    static QStringList loggingFilters();
    static QStringList loggingFiltersPlugins();

signals:
    void initialized();

    void pluginConfigChanged(const PluginId &id, const ParamList &config);
    void eventTriggered(const Event &event);
    void deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value);
    void deviceRemoved(const DeviceId &deviceId);
    void deviceAdded(Device *device);
    void deviceChanged(Device *device);
    void deviceSettingChanged(const DeviceId deviceId, const ParamTypeId &settingParamTypeId, const QVariant &value);

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
    DeviceManagerImplementation *m_deviceManager;
    RuleEngine *m_ruleEngine;
    ScriptEngine *m_scriptEngine;
    LogEngine *m_logger;
    TimeManager *m_timeManager;
    CloudManager *m_cloudManager;
    HardwareManagerImplementation *m_hardwareManager;
    DebugServerHandler *m_debugServerHandler;
    TagsStorage *m_tagsStorage;

    NetworkManager *m_networkManager;
    UserManager *m_userManager;
    System *m_system;
    ExperienceManager *m_experienceManager;

    QList<RuleId> m_executingRules;

private slots:
    void gotEvent(const Event &event);
    void onDateTimeChanged(const QDateTime &dateTime);
    void onDeviceDisappeared(const DeviceId &deviceId);
    void deviceManagerLoaded();

};

}

#endif // NYMEACORE_H
