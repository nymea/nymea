// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2026, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "nymeacore.h"
#include "debugserverhandler.h"
#include "experiences/experiencemanager.h"
#include "jsonrpc/debughandler.h"
#include "jsonrpc/jsonrpcserverimplementation.h"
#include "jsonrpc/scriptshandler.h"
#include "logging/logengineinfluxdb.h"
#include "loggingcategories.h"
#include "nymeasettings.h"
#include "platform/platform.h"
#include "platform/platformsystemcontroller.h"
#include "ruleengine/ruleengine.h"
#include "scriptengine/scriptengine.h"
#include "tagging/tagsstorage.h"
#include "usermanager/usermanager.h"
#include "version.h"

#include "integrations/browseractioninfo.h"
#include "integrations/browseritemactioninfo.h"
#include "integrations/thing.h"
#include "integrations/thingactioninfo.h"
#include "integrations/thingmanagerimplementation.h"

#include "zigbee/zigbeemanager.h"

#include "hardware/modbus/modbusrtumanager.h"
#include "hardware/serialport/serialportmonitor.h"
#include "zwave/zwavemanager.h"

#include <networkmanager.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMetaObject>

#ifdef WITH_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

NYMEA_LOGGING_CATEGORY(dcCore, "Core")

namespace nymeaserver {

NymeaCore *NymeaCore::s_instance = nullptr;
NymeaCore::ShutdownReason NymeaCore::s_shutdownReason = NymeaCore::ShutdownReasonTerm;
QStringList NymeaCore::s_additionalInterfaces;
bool NymeaCore::s_disableLogEngine = false;
NymeaCore::PendingRestartAction NymeaCore::s_pendingRestartAction = NymeaCore::PendingRestartActionNone;
QString NymeaCore::s_pendingRestoreBackupPath;

namespace {

bool removeDirectoryContents(const QString &directoryPath)
{
    QDir directory(directoryPath);
    if (!directory.exists()) {
        return true;
    }

    const QFileInfoList entries = directory.entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
    foreach (const QFileInfo &entry, entries) {
        bool success = false;
        if (entry.isDir()) {
            success = QDir(entry.absoluteFilePath()).removeRecursively();
        } else {
            success = QFile::remove(entry.absoluteFilePath());
        }

        if (!success) {
            qCWarning(dcCore()) << "Failed to remove persistent entry:" << entry.absoluteFilePath();
            return false;
        }
    }

    return true;
}

} // namespace

/*! Constructs NymeaCore with the given \a parent. This is private.
    Use \l{NymeaCore::instance()} to access the single instance.*/
NymeaCore::NymeaCore(QObject *parent)
    : QObject(parent)
{}

/*! Returns a pointer to the single \l{NymeaCore} instance. */
NymeaCore *NymeaCore::instance()
{
    if (!s_instance) {
        s_instance = new NymeaCore();
    }
    return s_instance;
}

void NymeaCore::restart(ShutdownReason reason)
{
    instance()->destroy(reason);
    instance()->init(s_additionalInterfaces, s_disableLogEngine);
}

void NymeaCore::init(const QStringList &additionalInterfaces, bool disableLogEngine)
{
    if (m_initialized) {
        qCWarning(dcCore()) << "NymeaCore is already initialized.";
        return;
    }
    m_initialized = true;

    s_additionalInterfaces = additionalInterfaces;
    s_disableLogEngine = disableLogEngine;

    qCDebug(dcCore()) << "Initializing NymeaCore";

    qCDebug(dcPlatform()) << "Loading platform abstraction";
    m_platform = new Platform(this);

    qCDebug(dcCore()) << "Loading nymea configurations from" << QFileInfo(NymeaSettings(NymeaSettings::SettingsRoleGlobal).fileName()).absoluteFilePath();
    m_configuration = new NymeaConfiguration(this);

    m_backupManager = new BackupManager(this);
    m_backupManager->setSourceDirectory(m_configuration->path());
    m_backupManager->setDestinationDirectory(m_configuration->backupDestinationDirectory());
    m_backupManager->setMaxBackups(m_configuration->backupMaxCount());
    m_backupManager->setAutomaticBackupInterval(m_configuration->autoBackupInterval());
    m_backupManager->setAutomaticBackupEnabled(m_configuration->autoBackupEnabled());

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
    m_userManager = new UserManager(NymeaSettings::privodeFromDefaultFilePath("user-db.sqlite"), this);

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
    m_hardwareManager = new HardwareManagerImplementation(m_platform, m_configuration, m_serverManager->mqttBroker(), m_zigbeeManager, m_zwaveManager, m_modbusRtuManager, this);

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
    m_serverManager->registerWebServerResource(m_debugServerHandler);

    qCDebug(dcCore) << "Register Debug Handler";
    m_serverManager->jsonServer()->registerHandler(new DebugHandler(m_serverManager->jsonServer()));

    qCDebug(dcCore()) << "Loading experiences";
    m_experienceManager = new ExperienceManager(m_thingManager, m_serverManager->jsonServer(), m_serverManager, m_userManager, m_logEngine, this);

    connect(m_configuration, &NymeaConfiguration::serverNameChanged, m_serverManager, &ServerManager::setServerName);
    connect(m_configuration, &NymeaConfiguration::backupDestinationDirectoryChanged, m_backupManager, &BackupManager::setDestinationDirectory);
    connect(m_configuration, &NymeaConfiguration::backupMaxCountChanged, m_backupManager, &BackupManager::setMaxBackups);
    connect(m_configuration, &NymeaConfiguration::autoBackupIntervalChanged, m_backupManager, &BackupManager::setAutomaticBackupInterval);
    connect(m_configuration, &NymeaConfiguration::autoBackupEnabledChanged, m_backupManager, &BackupManager::setAutomaticBackupEnabled);
    connect(m_thingManager, &ThingManagerImplementation::loaded, this, &NymeaCore::thingManagerLoaded);
    connect(m_thingManager, &ThingManagerImplementation::thingRemoved, m_userManager, &UserManager::onThingRemoved);

    QMetaObject::invokeMethod(m_serverManager->jsonServer(), "setup", Qt::QueuedConnection);

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
    if (m_logger)
        m_logger->log({"stopped"}, {{"version", NYMEA_VERSION_STRING}, {"shutdownReason", QMetaEnum::fromType<ShutdownReason>().valueToKey(s_shutdownReason)}});

    // Disconnect all signals/slots, we're going down now
    if (m_timeManager) {
        m_timeManager->stopTimer();
        m_timeManager->disconnect(this);
    }

    if (m_thingManager) {
        m_thingManager->disconnect(this);
    }
    if (m_ruleEngine) {
        m_ruleEngine->disconnect(this);
    }

    // Then stop magic from happening
    if (m_ruleEngine) {
        qCDebug(dcCore) << "Shutting down \"Rule Engine\"";
        delete m_ruleEngine;
        m_ruleEngine = nullptr;
    }

    if (m_experienceManager) {
        qCDebug(dcCore()) << "Shutting down \"Experiences\"";
        delete m_experienceManager;
        m_experienceManager = nullptr;
    }

    // Next, ThingManager, so plugins don't access any resources any more.
    if (m_thingManager) {
        qCDebug(dcCore) << "Shutting down \"Thing Manager\"";
        delete m_thingManager;
        m_thingManager = nullptr;
    }

    // Destroy resources used by things
    if (m_hardwareManager) {
        qCDebug(dcCore()) << "Shutting down \"Hardware Manager\"";
        delete m_hardwareManager;
        m_hardwareManager = nullptr;
    }
    if (m_serverManager) {
        qCDebug(dcCore) << "Shutting down \"Server Manager\"";
        delete m_serverManager;
        m_serverManager = nullptr;
    }

    // Now go ahead and clean up stuff.
    if (m_logEngine) {
        qCDebug(dcCore) << "Shutting down \"Log Engine\"";
        delete m_logEngine;
        m_logEngine = nullptr;
    }

    qCDebug(dcCore) << "Done shutting down NymeaCore";
}

void NymeaCore::destroy(ShutdownReason reason)
{
    if (s_instance) {
        s_shutdownReason = reason;
        delete s_instance;
    }

    s_instance = nullptr;
    performPendingRestartAction();
}

void NymeaCore::scheduleFactoryReset()
{
    s_pendingRestartAction = PendingRestartActionFactoryReset;
    s_pendingRestoreBackupPath.clear();
}

void NymeaCore::scheduleBackupRestore(const QString &backupFilePath)
{
    s_pendingRestartAction = PendingRestartActionRestoreBackup;
    s_pendingRestoreBackupPath = backupFilePath;
}

NymeaConfiguration *NymeaCore::configuration() const
{
    return m_configuration;
}

BackupManager *NymeaCore::backupManager() const
{
    return m_backupManager;
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
        const QStringList filesInPath = translationDirectory.entryList();
        qCDebug(dcTranslations()) << filesInPath.count() << "translations in" << path;
        foreach (const QString &translationFile, filesInPath) {
            if (!translationFiles.contains(translationFile)) {
                translationFiles.append(translationFile);
            }
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
        loggingFiltersPlugins << pluginName.at(0).toUpper() + pluginName.mid(1);
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

bool NymeaCore::wipePersistentData()
{
    return removeDirectoryContents(NymeaSettings::settingsPath()) && removeDirectoryContents(NymeaSettings::cachePath());
}

bool NymeaCore::performPendingRestartAction()
{
    const PendingRestartAction pendingRestartAction = s_pendingRestartAction;
    const QString pendingRestoreBackupPath = s_pendingRestoreBackupPath;

    s_pendingRestartAction = PendingRestartActionNone;
    s_pendingRestoreBackupPath.clear();

    if (pendingRestartAction == PendingRestartActionNone) {
        return true;
    }

    if (pendingRestartAction == PendingRestartActionRestoreBackup) {
        BackupManager backupManager;
        bool success = backupManager.restoreBackup(pendingRestoreBackupPath, NymeaSettings::settingsPath());
        if (success) {
            success = removeDirectoryContents(NymeaSettings::cachePath());
        }
        if (!success) {
            qCWarning(dcCore()) << "Failed to restore backup during restart:" << pendingRestoreBackupPath;
            qCWarning(dcCore()) << "Pending restart action did not complete successfully.";
        }
        return success;
    }

    const bool success = wipePersistentData();
    if (!success) {
        qCWarning(dcCore()) << "Pending restart action did not complete successfully.";
    }

    return success;
}

void NymeaCore::thingManagerLoaded()
{
    // Tell hardare resources we're done with loading stuff...
    m_hardwareManager->thingsLoaded();

    emit initialized();

    qCDebug(dcCore()) << "Starting housekeeping...";
    QDateTime startTime = QDateTime::currentDateTime();

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

} // namespace nymeaserver
