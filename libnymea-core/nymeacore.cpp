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

#include "nymeacore.h"
#include "loggingcategories.h"
#include "platform/platform.h"
#include "jsonrpc/jsonrpcserverimplementation.h"
#include "ruleengine/ruleengine.h"
#include "nymeasettings.h"
#include "tagging/tagsstorage.h"
#include "platform/platform.h"
#include "experiences/experiencemanager.h"
#include "platform/platformsystemcontroller.h"
#include "logging/logengineinfluxdb.h"
#include "scriptengine/scriptengine.h"
#include "jsonrpc/scriptshandler.h"
#include "version.h"

#include "integrations/thingmanagerimplementation.h"
#include "integrations/thing.h"
#include "integrations/thingactioninfo.h"
#include "integrations/browseractioninfo.h"
#include "integrations/browseritemactioninfo.h"

#include "zigbee/zigbeemanager.h"

#include "zwave/zwavemanager.h"
#include "hardware/modbus/modbusrtumanager.h"
#include "hardware/serialport/serialportmonitor.h"

#include <networkmanager.h>

#include <QDir>
#include <QCoreApplication>

#ifdef WITH_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

NYMEA_LOGGING_CATEGORY(dcCore, "Core")

namespace nymeaserver {

NymeaCore* NymeaCore::s_instance = nullptr;
NymeaCore::ShutdownReason NymeaCore::s_shutdownReason = NymeaCore::ShutdownReasonTerm;

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

void NymeaCore::init(const QStringList &additionalInterfaces, bool disableLogEngine) {
    qCDebug(dcCore()) << "Initializing NymeaCore";

    qCDebug(dcPlatform()) << "Loading platform abstraction";
    m_platform = new Platform(this);

    qCDebug(dcCore()) << "Loading nymea configurations" << NymeaSettings(NymeaSettings::SettingsRoleGlobal).fileName();
    m_configuration = new NymeaConfiguration(this);

    qCDebug(dcCore()) << "Creating Time Manager";
    // Migration path: nymea < 0.18 doesn't use system time zone but stores its own time zone in the config
    // For migration, let's set the system's time zone to the config now to upgrade to the system time zone based nymea >= 0.18
    if (QTimeZone(m_configuration->timeZone()).isValid()) {
        if (m_platform->systemController()->setTimeZone(QTimeZone(m_configuration->timeZone()))) {
            m_configuration->setTimeZone("");
        }
    }
    m_timeManager = new TimeManager(this);

    qCDebug(dcCore()) << "Creating User Manager";
    m_userManager = new UserManager(NymeaSettings::settingsPath() + "/user-db.sqlite", this);

    qCDebug(dcCore) << "Creating Server Manager";
    m_serverManager = new ServerManager(m_platform, m_configuration, additionalInterfaces, this);

    qCDebug(dcCore()) << "Create Serial Port Monitor";
    m_serialPortMonitor = new SerialPortMonitor(this);

    qCDebug(dcCore()) << "Create Zigbee Manager";
    m_zigbeeManager = new ZigbeeManager(this);

    qCDebug(dcCore()) << "Creating ZWave Manager";
    m_zwaveManager = new ZWaveManager(m_serialPortMonitor, this);

    qCDebug(dcCore()) << "Create Modbus RTU Manager";
    m_modbusRtuManager = new ModbusRtuManager(m_serialPortMonitor, this);

    qCDebug(dcCore) << "Creating Hardware Manager";
    m_hardwareManager = new HardwareManagerImplementation(m_platform, m_serverManager->mqttBroker(), m_zigbeeManager, m_zwaveManager, m_modbusRtuManager, this);

    qCDebug(dcCore) << "Creating Log Engine";
    m_logEngine = new LogEngineInfluxDB(m_configuration->logDBHost(), m_configuration->logDBName(), m_configuration->logDBUser(), m_configuration->logDBPassword(), this);
    if (disableLogEngine) {
        m_logEngine->disable();
    } else {
        m_logEngine->enable();
    }

    m_logger = m_logEngine->registerLogSource("core", {"event"});

    qCDebug(dcCore) << "Creating Thing Manager (locale:" << m_configuration->locale() << ")";
    m_thingManager = new ThingManagerImplementation(m_hardwareManager, m_logEngine, m_configuration->locale(), this);

    qCDebug(dcCore) << "Creating Rule Engine";
    m_ruleEngine = new RuleEngine(m_thingManager, m_timeManager, m_logEngine, this);

    qCDebug(dcCore()) << "Creating Script Engine";
    m_scriptEngine = new scriptengine::ScriptEngine(m_thingManager, m_logEngine, this);
    m_serverManager->jsonServer()->registerHandler(new ScriptsHandler(m_scriptEngine, m_scriptEngine));

    qCDebug(dcCore()) << "Creating Tags Storage";
    m_tagsStorage = new TagsStorage(m_thingManager, m_ruleEngine, this);

    qCDebug(dcCore) << "Creating Network Manager";
    m_networkManager = new NetworkManager(this);
    m_networkManager->start();

    qCDebug(dcCore) << "Creating Debug Server Handler";
    m_debugServerHandler = new DebugServerHandler(this);

    qCDebug(dcCore()) << "Loading experiences";
    m_experienceManager = new ExperienceManager(m_thingManager, m_serverManager->jsonServer(), this);


    connect(m_configuration, &NymeaConfiguration::serverNameChanged, m_serverManager, &ServerManager::setServerName);

    connect(m_thingManager, &ThingManagerImplementation::loaded, this, &NymeaCore::thingManagerLoaded);

    m_logger->log({"started"}, {{"version", NYMEA_VERSION_STRING}});
#ifdef WITH_SYSTEMD
    sd_notify(0, "READY=1");
#endif
}

/*! Destructor of the \l{NymeaCore}. */
NymeaCore::~NymeaCore()
{
#ifdef WITH_SYSTEMD
    sd_notify(0, "STOPPING=1");
#endif

    qCDebug(dcCore()) << "Shutting down NymeaCore";
    m_logger->log({"stopped"}, {
                      {"version", NYMEA_VERSION_STRING},
                      {"shutdownReason", QMetaEnum::fromType<ShutdownReason>().valueToKey(s_shutdownReason)}
                  });

    // Disconnect all signals/slots, we're going down now
    m_timeManager->stopTimer();
    m_timeManager->disconnect(this);

    m_thingManager->disconnect(this);
    m_ruleEngine->disconnect(this);

    // Then stop magic from happening
    qCDebug(dcCore) << "Shutting down \"Rule Engine\"";
    delete m_ruleEngine;

    qCDebug(dcCore()) << "Shutting down \"Experiences\"";
    delete m_experienceManager;

    // Next, ThingManager, so plugins don't access any resources any more.
    qCDebug(dcCore) << "Shutting down \"Thing Manager\"";
    delete m_thingManager;

    // Destroy resources used by things
    qCDebug(dcCore) << "Shutting down \"Server Manager\"";
    delete m_serverManager;

    qCDebug(dcCore()) << "Shutting down \"Hardware Manager\"";
    delete m_hardwareManager;

    // Now go ahead and clean up stuff.
    qCDebug(dcCore) << "Shutting down \"Log Engine\"";
    delete m_logEngine;

    qCDebug(dcCore) << "Done shutting down NymeaCore";
}

void NymeaCore::destroy(ShutdownReason reason)
{
    if (s_instance) {
        s_shutdownReason = reason;
        delete s_instance;
    }

    s_instance = nullptr;
}

NymeaConfiguration *NymeaCore::configuration() const
{
    return m_configuration;
}

ThingManager *NymeaCore::thingManager() const
{
    return m_thingManager;
}

RuleEngine *NymeaCore::ruleEngine() const
{
    return m_ruleEngine;
}

ScriptEngine *NymeaCore::scriptEngine() const
{
    return m_scriptEngine;
}

TimeManager *NymeaCore::timeManager() const
{
    return m_timeManager;
}

ServerManager *NymeaCore::serverManager() const
{
    return m_serverManager;
}

QStringList NymeaCore::getAvailableLanguages()
{
    qCDebug(dcCore()) << "Loading translations from" << NymeaSettings::translationsPath();

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

QStringList NymeaCore::loggingFilters()
{
    return nymeaLoggingCategories();
}

QStringList NymeaCore::loggingFiltersPlugins()
{
    QStringList loggingFiltersPlugins;
    foreach (const QJsonObject &pluginMetadata, ThingManagerImplementation::pluginsMetadata()) {
        QString pluginName = pluginMetadata.value("name").toString();
        loggingFiltersPlugins << pluginName.left(1).toUpper() + pluginName.mid(1);
    }
    return loggingFiltersPlugins;
}

BluetoothServer *NymeaCore::bluetoothServer() const
{
    return m_serverManager->bluetoothServer();
}

NetworkManager *NymeaCore::networkManager() const
{
    return m_networkManager;
}

UserManager *NymeaCore::userManager() const
{
    return m_userManager;
}

CloudManager *NymeaCore::cloudManager() const
{
    return m_cloudManager;
}

DebugServerHandler *NymeaCore::debugServerHandler() const
{
    return m_debugServerHandler;
}

TagsStorage *NymeaCore::tagsStorage() const
{
    return m_tagsStorage;
}

Platform *NymeaCore::platform() const
{
    return m_platform;
}

ZigbeeManager *NymeaCore::zigbeeManager() const
{
    return m_zigbeeManager;
}

ZWaveManager *NymeaCore::zwaveManager() const
{
    return m_zwaveManager;
}

ModbusRtuManager *NymeaCore::modbusRtuManager() const
{
    return m_modbusRtuManager;
}

ExperienceManager *NymeaCore::experienceManager() const
{
    return m_experienceManager;
}

LogEngine *NymeaCore::logEngine() const
{
    return m_logEngine;
}

JsonRPCServerImplementation *NymeaCore::jsonRPCServer() const
{
    return m_serverManager->jsonServer();
}

void NymeaCore::thingManagerLoaded()
{

    // Tell hardare resources we're done with loading stuff...
    m_hardwareManager->thingsLoaded();

    emit initialized();

    // Do some houskeeping...
    qCDebug(dcCore()) << "Starting housekeeping...";
    QDateTime startTime = QDateTime::currentDateTime();
//    ThingsFetchJob *job = m_logger->fetchThings();
//    connect(job, &ThingsFetchJob::finished, m_thingManager, [this, job, startTime](){
//        foreach (const ThingId &thingId, job->results()) {
//            if (!m_thingManager->findConfiguredThing(thingId)) {
//                qCDebug(dcCore()) << "Cleaning stale thing entries from log DB for thing id" << thingId;
//                m_logger->removeThingLogs(thingId);
//            }
//        }
//        qCDebug(dcCore()) << "Housekeeping done in" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms.";
//    });

    foreach (const ThingId &thingId, m_ruleEngine->thingsInRules()) {
        if (!m_thingManager->findConfiguredThing(thingId)) {
            qCDebug(dcCore()) << "Cleaning stale rule entries for thing id" << thingId;
            foreach (const RuleId &ruleId, m_ruleEngine->findRules(thingId)) {
                m_ruleEngine->removeThingFromRule(ruleId, thingId);
            }
        }
    }

    foreach (const Tag &tag, m_tagsStorage->tags()) {
        if (!tag.ruleId().isNull() && !m_ruleEngine->findRule(tag.ruleId()).isValid()) {
            qCDebug(dcCore()) << "Cleaning up stale rule tag" << tag;
            m_tagsStorage->removeTag(tag);
        }
        if (!tag.thingId().isNull() && !m_thingManager->findConfiguredThing(tag.thingId())) {
            qCDebug(dcCore()) << "Cleaning up stale thing tag" << tag.tagId();
            m_tagsStorage->removeTag(tag);
        }
    }

}

}
