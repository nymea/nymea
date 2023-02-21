/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef NYMEACORE_H
#define NYMEACORE_H

#include "types/event.h"
#include "types/thingclass.h"
#include "integrations/integrationplugin.h"
#include "integrations/thingdescriptor.h"
#include "integrations/thingmanagerimplementation.h"

#include "ruleengine/rule.h"
#include "ruleengine/ruleengine.h"

#include "logging/logengine.h"
#include "servermanager.h"

#include "time/timemanager.h"
#include "hardwaremanagerimplementation.h"

#include "debugserverhandler.h"

#include <QObject>

class Thing;

class NetworkManager;

namespace nymeaserver {

class JsonRPCServerImplementation;
class LogEngine;
class NymeaConfiguration;
class TagsStorage;
class UserManager;
class Platform;
class System;
class ExperienceManager;
class CloudManager;
class ZigbeeManager;
class ZWaveManager;
class ModbusRtuManager;
class SerialPortMonitor;

namespace scriptengine {
class ScriptEngine;
}
using namespace scriptengine;

class NymeaCore : public QObject
{
    Q_OBJECT
    friend class NymeaTestBase;

public:
    static NymeaCore* instance();
    ~NymeaCore();

    void init(const QStringList &additionalInterfaces = QStringList());
    void destroy();

    RuleEngine::RuleError removeRule(const RuleId &id);

    NymeaConfiguration *configuration() const;
    LogEngine* logEngine() const;
    JsonRPCServerImplementation *jsonRPCServer() const;
    ThingManager *thingManager() const;
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
    ZigbeeManager *zigbeeManager() const;
    ZWaveManager *zwaveManager() const;
    ModbusRtuManager *modbusRtuManager() const;
    ExperienceManager *experienceManager() const;

    static QStringList getAvailableLanguages();
    static QStringList loggingFilters();
    static QStringList loggingFiltersPlugins();

signals:
    void initialized();

private:
    explicit NymeaCore(QObject *parent = nullptr);
    static NymeaCore *s_instance;

    Platform *m_platform = nullptr;

    NymeaConfiguration *m_configuration;
    ServerManager *m_serverManager;
    ThingManagerImplementation *m_thingManager;
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
    ZigbeeManager *m_zigbeeManager;
    ZWaveManager *m_zwaveManager;
    SerialPortMonitor *m_serialPortMonitor;
    ModbusRtuManager *m_modbusRtuManager;


private slots:
    void thingManagerLoaded();

};

}

#endif // NYMEACORE_H
