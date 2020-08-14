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

#include "scriptengine/scriptengine.h"
#include "jsonrpc/scriptshandler.h"

#include "integrations/thingmanagerimplementation.h"
#include "integrations/thing.h"
#include "integrations/thingactioninfo.h"
#include "integrations/browseractioninfo.h"
#include "integrations/browseritemactioninfo.h"
#include "cloud/cloudmanager.h"
#include "cloud/cloudnotifications.h"
#include "cloud/cloudtransport.h"

#include <networkmanager.h>

#include <QDir>
#include <QCoreApplication>

NYMEA_LOGGING_CATEGORY(dcCore, "Core")

namespace nymeaserver {

NymeaCore* NymeaCore::s_instance = nullptr;

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

void NymeaCore::init() {
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

    qCDebug(dcCore) << "Creating Log Engine";
    m_logger = new LogEngine(m_configuration->logDBDriver(), m_configuration->logDBName(), m_configuration->logDBHost(), m_configuration->logDBUser(), m_configuration->logDBPassword(), m_configuration->logDBMaxEntries(), this);

    qCDebug(dcCore()) << "Creating User Manager";
    m_userManager = new UserManager(NymeaSettings::settingsPath() + "/user-db.sqlite", this);

    qCDebug(dcCore) << "Creating Server Manager";
    m_serverManager = new ServerManager(m_platform, m_configuration, this);

    qCDebug(dcCore) << "Creating Hardware Manager";
    m_hardwareManager = new HardwareManagerImplementation(m_platform, m_serverManager->mqttBroker(), this);

    qCDebug(dcCore) << "Creating Thing Manager (locale:" << m_configuration->locale() << ")";
    m_thingManager = new ThingManagerImplementation(m_hardwareManager, m_configuration->locale(), this);

    qCDebug(dcCore) << "Creating Rule Engine";
    m_ruleEngine = new RuleEngine(this);

    qCDebug(dcCore()) << "Creating Script Engine";
    m_scriptEngine = new ScriptEngine(m_thingManager, this);
    m_serverManager->jsonServer()->registerHandler(new ScriptsHandler(m_scriptEngine, m_scriptEngine));

    qCDebug(dcCore()) << "Creating Tags Storage";
    m_tagsStorage = new TagsStorage(m_thingManager, m_ruleEngine, this);

    qCDebug(dcCore) << "Creating Network Manager";
    m_networkManager = new NetworkManager(this);
    m_networkManager->start();

    qCDebug(dcCore) << "Creating Debug Server Handler";
    m_debugServerHandler = new DebugServerHandler(this);

    qCDebug(dcCore) << "Creating Cloud Manager";
    m_cloudManager = new CloudManager(m_configuration, m_networkManager, this);

    qCDebug(dcCore()) << "Loading experiences";
    m_experienceManager = new ExperienceManager(m_thingManager, m_serverManager->jsonServer(), this);


    CloudNotifications *cloudNotifications = m_cloudManager->createNotificationsPlugin();
    m_thingManager->registerStaticPlugin(cloudNotifications, cloudNotifications->metaData());

    CloudTransport *cloudTransport = m_cloudManager->createTransportInterface();
    m_serverManager->jsonServer()->registerTransportInterface(cloudTransport, false);

    connect(m_configuration, &NymeaConfiguration::serverNameChanged, m_serverManager, &ServerManager::setServerName);

    connect(m_thingManager, &ThingManagerImplementation::pluginConfigChanged, this, &NymeaCore::pluginConfigChanged);
    connect(m_thingManager, &ThingManagerImplementation::eventTriggered, this, &NymeaCore::gotEvent);
    connect(m_thingManager, &ThingManagerImplementation::thingStateChanged, this, &NymeaCore::thingStateChanged);
    connect(m_thingManager, &ThingManagerImplementation::thingAdded, this, &NymeaCore::thingAdded);
    connect(m_thingManager, &ThingManagerImplementation::thingChanged, this, &NymeaCore::thingChanged);
    connect(m_thingManager, &ThingManagerImplementation::thingSettingChanged, this, &NymeaCore::thingSettingChanged);
    connect(m_thingManager, &ThingManagerImplementation::thingRemoved, this, &NymeaCore::thingRemoved);
    connect(m_thingManager, &ThingManagerImplementation::thingDisappeared, this, &NymeaCore::onThingDisappeared);
    connect(m_thingManager, &ThingManagerImplementation::loaded, this, &NymeaCore::thingManagerLoaded);

    connect(m_ruleEngine, &RuleEngine::ruleAdded, this, &NymeaCore::ruleAdded);
    connect(m_ruleEngine, &RuleEngine::ruleRemoved, this, &NymeaCore::ruleRemoved);
    connect(m_ruleEngine, &RuleEngine::ruleConfigurationChanged, this, &NymeaCore::ruleConfigurationChanged);

    connect(m_timeManager, &TimeManager::dateTimeChanged, this, &NymeaCore::onDateTimeChanged);

    m_logger->logSystemEvent(m_timeManager->currentDateTime(), true);
}

/*! Destructor of the \l{NymeaCore}. */
NymeaCore::~NymeaCore()
{
    qCDebug(dcCore()) << "Shutting down NymeaCore";
    m_logger->logSystemEvent(m_timeManager->currentDateTime(), false);

    // Disconnect all signals/slots, we're going down now
    m_timeManager->disconnect(this);
    m_thingManager->disconnect(this);
    m_ruleEngine->disconnect(this);

    // At very first, cut off the outside world
    qCDebug(dcCore) << "Shutting down \"Server Manager\"";
    delete m_serverManager;
    qCDebug(dcCore) << "Shutting down \"CloudManager\"";
    delete m_cloudManager;

    // Then stop magic from happening
    qCDebug(dcCore) << "Shutting down \"Rule Engine\"";
    delete m_ruleEngine;

    // Next, ThingManager, so plugins don't access any resources any more.
    qCDebug(dcCore) << "Shutting down \"Thing Manager\"";
    delete m_thingManager;

    // Now go ahead and clean up stuff.
    qCDebug(dcCore) << "Shutting down \"Log Engine\"";
    delete m_logger;

    qCDebug(dcCore()) << "Shutting down \"Hardware Manager\"";
    delete m_hardwareManager;

    qCDebug(dcCore) << "Done shutting down NymeaCore";
}

void NymeaCore::destroy()
{
    if (s_instance) {
        delete s_instance;
    }

    s_instance = nullptr;
}

QPair<Thing::ThingError, QList<RuleId> > NymeaCore::removeConfiguredThing(const ThingId &thingId, const QHash<RuleId, RuleEngine::RemovePolicy> &removePolicyList)
{
    Thing *thing = m_thingManager->findConfiguredThing(thingId);

    if (!thing) {
        return QPair<Thing::ThingError, QList<RuleId> > (Thing::ThingErrorThingNotFound, QList<RuleId>());
    }

    // Check if this is a child
    if (!thing->parentId().isNull() && thing->autoCreated()) {
        qCWarning(dcThingManager) << "Thing is an autocreated child of" << thing->parentId().toString() << ". Please remove the parent.";
        return QPair<Thing::ThingError, QList<RuleId> > (Thing::ThingErrorThingIsChild, QList<RuleId>());
    }

    // FIXME: Let's remove this for now. It will come back with more fine grained control, presumably introducing a RemoveMethod flag in the DeviceClass
//    if (thing->autoCreated()) {
//        qCWarning(dcThingManager) << "This thing has been auto-created and cannot be deleted manually.";
//        return QPair<Thing::ThingError, QList<RuleId> >(Thing::ThingErrorCreationMethodNotSupported, {});
//    }

    // Check if this thing has childs
    QList<Thing *> thingsToRemove;
    thingsToRemove.append(thing);
    QList<Thing *> childs = m_thingManager->findChilds(thingId);
    if (!childs.isEmpty()) {
        foreach (Thing *child, childs) {
            thingsToRemove.append(child);
        }
    }

    // check things
    QList<RuleId> offendingRules;
    qCDebug(dcThingManager) << "Things to remove:";
    foreach (Thing *d, thingsToRemove) {
        qCDebug(dcThingManager) << " -> " << d->name() << d->id().toString();

        // Check if thing is in a rule
        foreach (const RuleId &ruleId, m_ruleEngine->findRules(d->id())) {
            qCDebug(dcThingManager) << "      -> in rule:" << ruleId.toString();
            if (!offendingRules.contains(ruleId)) {
                offendingRules.append(ruleId);
            }
        }
    }

    // check each offending rule if there is a corresponding remove policy
    QHash<RuleId, RuleEngine::RemovePolicy> toBeChanged;
    QList<RuleId> unhandledRules;
    foreach (const RuleId &ruleId, offendingRules) {
        bool found = false;
        foreach (const RuleId &policyRuleId, removePolicyList.keys()) {
            if (ruleId == policyRuleId) {
                found = true;
                toBeChanged.insert(ruleId, removePolicyList.value(ruleId));
                break;
            }
        }
        if (!found)
            unhandledRules.append(ruleId);

    }

    if (!unhandledRules.isEmpty()) {
        qCWarning(dcThingManager) << "There are unhandled rules which depend on this thing:\n" << unhandledRules;
        return QPair<Thing::ThingError, QList<RuleId> > (Thing::ThingErrorThingInRule, unhandledRules);
    }

    // Update the rules...
    foreach (const RuleId &ruleId, toBeChanged.keys()) {
        if (toBeChanged.value(ruleId) == RuleEngine::RemovePolicyCascade) {
            m_ruleEngine->removeRule(ruleId);
        } else if (toBeChanged.value(ruleId) == RuleEngine::RemovePolicyUpdate){
            foreach (Thing *thing, thingsToRemove) {
                m_ruleEngine->removeThingFromRule(ruleId, thing->id());
            }
        }
    }

    // remove the childs
    foreach (Thing *d, childs) {
        Thing::ThingError removeError = m_thingManager->removeConfiguredThing(d->id());
        if (removeError == Thing::ThingErrorNoError) {
            m_logger->removeThingLogs(d->id());
        }
    }

    // delete the things
    Thing::ThingError removeError = m_thingManager->removeConfiguredThing(thingId);
    if (removeError == Thing::ThingErrorNoError) {
        m_logger->removeThingLogs(thingId);
    }

    return QPair<Thing::ThingError, QList<RuleId> > (Thing::ThingErrorNoError, QList<RuleId>());
}


Thing::ThingError NymeaCore::removeConfiguredThing(const ThingId &thingId, const RuleEngine::RemovePolicy &removePolicy)
{
    Thing *thing = m_thingManager->findConfiguredThing(thingId);

    if (!thing) {
        return Thing::ThingErrorThingNotFound;
    }

    // Check if this is a child
    if (!thing->parentId().isNull() && thing->autoCreated()) {
        qCWarning(dcThingManager) << "Thing is an autocreated child of" << thing->parentId().toString() << ". Please remove the parent.";
        return Thing::ThingErrorThingIsChild;
    }

    // FIXME: Let's remove this for now. It will come back with more fine grained control, presumably introducing a RemoveMethod flag in the DeviceClass
//    if (thing->autoCreated()) {
//        qCWarning(dcThingManager) << "This thing has been auto-created and cannot be deleted manually.";
//        return Thing::ThingErrorCreationMethodNotSupported;
//    }

    // Check if this thing has childs
    QList<Thing *> thingsToRemove;
    thingsToRemove.append(thing);
    QList<Thing *> childs = m_thingManager->findChilds(thingId);
    if (!childs.isEmpty()) {
        foreach (Thing *child, childs) {
            thingsToRemove.append(child);
        }
    }

    // check things
    QList<RuleId> offendingRules;
    qCDebug(dcThingManager) << "Things to remove:";
    foreach (Thing *d, thingsToRemove) {
        qCDebug(dcThingManager) << " -> " << d->name() << d->id().toString();

        // Check if thing is in a rule
        foreach (const RuleId &ruleId, m_ruleEngine->findRules(d->id())) {
            qCDebug(dcThingManager) << "      -> in rule:" << ruleId.toString();
            if (!offendingRules.contains(ruleId)) {
                offendingRules.append(ruleId);
            }
        }
    }

    // apply removepolicy for foreach rule
    foreach (const RuleId &ruleId, offendingRules) {
        if (removePolicy == RuleEngine::RemovePolicyCascade) {
            m_ruleEngine->removeRule(ruleId);
        } else if (removePolicy == RuleEngine::RemovePolicyUpdate){
            foreach (Thing *thing, thingsToRemove) {
                m_ruleEngine->removeThingFromRule(ruleId, thing->id());
            }
        }
    }

    // remove the childs
    foreach (Thing *d, childs) {
        Thing::ThingError removeError = m_thingManager->removeConfiguredThing(d->id());
        if (removeError == Thing::ThingErrorNoError) {
            m_logger->removeThingLogs(d->id());
        }
    }

    // delete the things
    Thing::ThingError removeError = m_thingManager->removeConfiguredThing(thingId);
    if (removeError == Thing::ThingErrorNoError) {
        m_logger->removeThingLogs(thingId);
    }

    return removeError;
}

ThingActionInfo* NymeaCore::executeAction(const Action &action)
{
    ThingActionInfo *info = m_thingManager->executeAction(action);
    connect(info, &ThingActionInfo::finished, this, [this, info](){
        if (info->status() == Thing::ThingErrorNoError) {
            m_logger->logAction(info->action());
        } else {
            m_logger->logAction(info->action(), Logging::LoggingLevelAlert, info->status());
        }
    });

    return info;
}

BrowserActionInfo* NymeaCore::executeBrowserItem(const BrowserAction &browserAction)
{
    BrowserActionInfo *info = m_thingManager->executeBrowserItem(browserAction);
    connect(info, &BrowserActionInfo::finished, info->thing(), [this, info](){
        m_logger->logBrowserAction(info->browserAction(), info->status() == Thing::ThingErrorNoError ? Logging::LoggingLevelInfo : Logging::LoggingLevelAlert, info->status());
    });
    return info;
}

BrowserItemActionInfo *NymeaCore::executeBrowserItemAction(const BrowserItemAction &browserItemAction)
{
    BrowserItemActionInfo *info = m_thingManager->executeBrowserItemAction(browserItemAction);
    connect(info, &BrowserItemActionInfo::finished, info->thing(), [this, info](){
        m_logger->logBrowserItemAction(info->browserItemAction(), info->status() == Thing::ThingErrorNoError ? Logging::LoggingLevelInfo : Logging::LoggingLevelAlert, info->status());
    });
    return info;
}

/*! Execute the given \a ruleActions. */
void NymeaCore::executeRuleActions(const QList<RuleAction> ruleActions)
{
    QList<Action> actions;
    QList<BrowserAction> browserActions;
    foreach (const RuleAction &ruleAction, ruleActions) {
        if (ruleAction.type() == RuleAction::TypeThing) {
            Thing *thing = m_thingManager->findConfiguredThing(ruleAction.thingId());
            if (!thing) {
                qCWarning(dcRuleEngine()) << "Unable to find thing" << ruleAction.thingId() << "for rule action" << ruleAction;
                continue;
            }
            ActionTypeId actionTypeId = ruleAction.actionTypeId();
            ParamList params;
            bool ok = true;
            foreach (const RuleActionParam &ruleActionParam, ruleAction.ruleActionParams()) {
                if (ruleActionParam.isValueBased()) {
                    params.append(Param(ruleActionParam.paramTypeId(), ruleActionParam.value()));
                } else if (ruleActionParam.isStateBased()) {
                    Thing *stateThing = m_thingManager->findConfiguredThing(ruleActionParam.stateThingId());
                    if (!stateThing) {
                        qCWarning(dcRuleEngine()) << "Cannot find thing" << ruleActionParam.stateThingId() << "required by rule action";
                        ok = false;
                        break;
                    }
                    ThingClass stateThingClass = m_thingManager->findThingClass(stateThing->thingClassId());
                    if (!stateThingClass.hasStateType(ruleActionParam.stateTypeId())) {
                        qCWarning(dcRuleEngine()) << "Device" << thing->name() << thing->id() << "does not have a state type" << ruleActionParam.stateTypeId();
                        ok = false;
                        break;
                    }
                    params.append(Param(ruleActionParam.paramTypeId(), stateThing->stateValue(ruleActionParam.stateTypeId())));
                }
            }
            if (!ok) {
                qCWarning(dcRuleEngine()) << "Not executing rule action";
                continue;
            }
            Action action(actionTypeId, thing->id(), Action::TriggeredByRule);
            action.setParams(params);
            actions.append(action);
        } else if (ruleAction.type() == RuleAction::TypeBrowser) {
            Thing *thing = m_thingManager->findConfiguredThing(ruleAction.thingId());
            if (!thing) {
                qCWarning(dcRuleEngine()) << "Unable to find thing" << ruleAction.thingId() << "for rule action" << ruleAction;
                continue;
            }
            BrowserAction browserAction(ruleAction.thingId(), ruleAction.browserItemId());
            browserActions.append(browserAction);
        } else {
            Things things = m_thingManager->findConfiguredThings(ruleAction.interface());
            foreach (Thing* thing, things) {
                ThingClass thingClass = m_thingManager->findThingClass(thing->thingClassId());
                ActionType actionType = thingClass.actionTypes().findByName(ruleAction.interfaceAction());
                if (actionType.id().isNull()) {
                    qCWarning(dcRuleEngine()) << "Error creating Action. The given ThingClass does not implement action:" << ruleAction.interfaceAction();
                    continue;
                }

                ParamList params;
                bool ok = true;
                foreach (const RuleActionParam &ruleActionParam, ruleAction.ruleActionParams()) {
                    ParamType paramType = actionType.paramTypes().findByName(ruleActionParam.paramName());
                    if (paramType.id().isNull()) {
                        qCWarning(dcRuleEngine()) << "Error creating Action. The given ActionType does not have a parameter:" << ruleActionParam.paramName();
                        ok = false;
                        continue;
                    }
                    if (ruleActionParam.isValueBased()) {
                        params.append(Param(paramType.id(), ruleActionParam.value()));
                    } else if (ruleActionParam.isStateBased()) {
                        Thing *stateThing = m_thingManager->findConfiguredThing(ruleActionParam.stateThingId());
                        if (!stateThing) {
                            qCWarning(dcRuleEngine()) << "Cannot find thing" << ruleActionParam.stateThingId() << "required by rule action";
                            ok = false;
                            break;
                        }
                        ThingClass stateThingClass = m_thingManager->findThingClass(stateThing->thingClassId());
                        if (!stateThingClass.hasStateType(ruleActionParam.stateTypeId())) {
                            qCWarning(dcRuleEngine()) << "Thing" << thing->name() << thing->id() << "does not have a state type" << ruleActionParam.stateTypeId();
                            ok = false;
                            break;
                        }
                        params.append(Param(paramType.id(), stateThing->stateValue(ruleActionParam.stateTypeId())));
                    }
                }
                if (!ok) {
                    qCWarning(dcRuleEngine()) << "Not executing rule action";
                    continue;
                }

                Action action = Action(actionType.id(), thing->id(), Action::TriggeredByRule);
                action.setParams(params);
                actions.append(action);
            }
        }
    }

    foreach (const Action &action, actions) {
        qCDebug(dcRuleEngine) << "Executing action" << action.actionTypeId() << action.params();
        ThingActionInfo *info = executeAction(action);
        connect(info, &ThingActionInfo::finished, this, [info](){
            if (info->status() != Thing::ThingErrorNoError) {
                qCWarning(dcRuleEngine) << "Error executing action:" << info->status() << info->displayMessage();
            }
        });
    }

    foreach (const BrowserAction &browserAction, browserActions) {
        BrowserActionInfo *info = executeBrowserItem(browserAction);
        connect(info, &BrowserActionInfo::finished, this, [info](){
            if (info->status() != Thing::ThingErrorNoError) {
                qCWarning(dcRuleEngine) << "Error executing browser action:" << info->status();
            }
        });
    }
}

/*! Calls the metheod RuleEngine::removeRule(\a id).
 *  \sa RuleEngine, */
RuleEngine::RuleError NymeaCore::removeRule(const RuleId &id)
{
    RuleEngine::RuleError removeError = m_ruleEngine->removeRule(id);
    if (removeError == RuleEngine::RuleErrorNoError)
        m_logger->removeRuleLogs(id);

    return removeError;
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
    return s_nymeaLoggingCategories;
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

void NymeaCore::gotEvent(const Event &event)
{
    m_logger->logEvent(event);
    emit eventTriggered(event);

    QList<RuleAction> actions;
    QList<RuleAction> eventBasedActions;
    foreach (const Rule &rule, m_ruleEngine->evaluateEvent(event)) {
        if (m_executingRules.contains(rule.id())) {
            qCWarning(dcRuleEngine()) << "WARNING: Loop detected in rule execution for rule" << rule.id() << rule.name();
            break;
        }
        m_executingRules.append(rule.id());

        // Event based
        if (!rule.eventDescriptors().isEmpty()) {
            m_logger->logRuleTriggered(rule);
            QList<RuleAction> tmp;
            if (rule.statesActive() && rule.timeActive()) {
                qCDebug(dcRuleEngineDebug()) << "Executing actions";
                tmp = rule.actions();
            } else {
                qCDebug(dcRuleEngineDebug()) << "Executing exitActions";
                tmp = rule.exitActions();
            }
            // check if we have an event based action or a normal action
            foreach (const RuleAction &action, tmp) {
                if (action.isEventBased()) {
                    eventBasedActions.append(action);
                } else {
                    actions.append(action);
                }
            }
        } else {
            // State based rule
            m_logger->logRuleActiveChanged(rule);
            emit ruleActiveChanged(rule);
            if (rule.active()) {
                actions.append(rule.actions());
            } else {
                actions.append(rule.exitActions());
            }
        }
    }

    // Set action params, depending on the event value
    foreach (RuleAction ruleAction, eventBasedActions) {
        RuleActionParams newParams;
        foreach (RuleActionParam ruleActionParam, ruleAction.ruleActionParams()) {
            // if this event param should be taken over in this action
            if (event.eventTypeId() == ruleActionParam.eventTypeId()) {
                QVariant eventValue = event.params().paramValue(ruleActionParam.eventParamTypeId());

                // TODO: limits / scale calculation -> actionValue = eventValue * x
                //       something like a EventParamDescriptor

                ruleActionParam.setValue(eventValue);
                qCDebug(dcRuleEngine) << "Using param value from event:" << ruleActionParam.value();
            }
            newParams.append(ruleActionParam);
        }
        ruleAction.setRuleActionParams(newParams);
        actions.append(ruleAction);
    }

    executeRuleActions(actions);
    m_executingRules.clear();
}

void NymeaCore::onDateTimeChanged(const QDateTime &dateTime)
{
    QList<RuleAction> actions;
    foreach (const Rule &rule, m_ruleEngine->evaluateTime(dateTime)) {
        // TimeEvent based
        if (!rule.timeDescriptor().timeEventItems().isEmpty()) {
            m_logger->logRuleTriggered(rule);
            if (rule.statesActive() && rule.timeActive()) {
                actions.append(rule.actions());
            } else {
                actions.append(rule.exitActions());
            }
        } else {
            // Calendar based rule
            m_logger->logRuleActiveChanged(rule);
            emit ruleActiveChanged(rule);
            if (rule.active()) {
                actions.append(rule.actions());
            } else {
                actions.append(rule.exitActions());
            }
        }
    }
    executeRuleActions(actions);
}

LogEngine* NymeaCore::logEngine() const
{
    return m_logger;
}

JsonRPCServerImplementation *NymeaCore::jsonRPCServer() const
{
    return m_serverManager->jsonServer();
}

void NymeaCore::onThingDisappeared(const ThingId &thingId)
{
    Thing *thing = m_thingManager->findConfiguredThing(thingId);
    if (!thing) {
        return;
    }

    // Check if this thing has childs
    Things thingsToRemove;
    thingsToRemove.append(thing);
    QList<Thing *> childs = m_thingManager->findChilds(thingId);
    if (!childs.isEmpty()) {
        foreach (Thing *child, childs) {
            thingsToRemove.append(child);
        }
    }

    // check things
    QList<RuleId> offendingRules;
    qCDebug(dcThingManager) << "Thing to remove:";
    foreach (Thing *d, thingsToRemove) {
        qCDebug(dcThingManager) << " -> " << d->name() << d->id().toString();

        // Check if thing is in a rule
        foreach (const RuleId &ruleId, m_ruleEngine->findRules(d->id())) {
            qCDebug(dcThingManager) << "      -> in rule:" << ruleId.toString();
            if (!offendingRules.contains(ruleId)) {
                offendingRules.append(ruleId);
            }
        }
    }

    // update involved rules
    foreach (const RuleId &ruleId, offendingRules) {
        foreach (Thing *thing, thingsToRemove) {
            m_ruleEngine->removeThingFromRule(ruleId, thing->id());
        }
    }

    // remove the child devices
    foreach (Thing *d, childs) {
        Thing::ThingError removeError = m_thingManager->removeConfiguredThing(d->id());
        if (removeError == Thing::ThingErrorNoError) {
            m_logger->removeThingLogs(d->id());
        }
    }

    // delete the thing
    Thing::ThingError removeError = m_thingManager->removeConfiguredThing(thingId);
    if (removeError == Thing::ThingErrorNoError) {
        m_logger->removeThingLogs(thingId);
    }
}

void NymeaCore::thingManagerLoaded()
{
    m_ruleEngine->init();
    // Evaluate rules on current time
    onDateTimeChanged(m_timeManager->currentDateTime());

    emit initialized();

    // Do some houskeeping...
    qCDebug(dcCore()) << "Starting housekeeping...";
    QDateTime startTime = QDateTime::currentDateTime();
    ThingsFetchJob *job = m_logger->fetchThings();
    connect(job, &ThingsFetchJob::finished, m_thingManager, [this, job, startTime](){
        foreach (const ThingId &thingId, job->results()) {
            if (!m_thingManager->findConfiguredThing(thingId)) {
                qCDebug(dcCore()) << "Cleaning stale thing entries from log DB for thing id" << thingId;
                m_logger->removeThingLogs(thingId);
            }
        }
        qCDebug(dcCore()) << "Housekeeping done in" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms.";
    });

    foreach (const ThingId &thingId, m_ruleEngine->thingsInRules()) {
        if (!m_thingManager->findConfiguredThing(thingId)) {
            qCDebug(dcCore()) << "Cleaning stale rule entries for thing id" << thingId;
            foreach (const RuleId &ruleId, m_ruleEngine->findRules(thingId)) {
                m_ruleEngine->removeThingFromRule(ruleId, thingId);
            }
        }
    }
}

}
