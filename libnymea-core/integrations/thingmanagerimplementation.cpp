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

#include "thingmanagerimplementation.h"
#include "translator.h"
#if QT_VERSION >= QT_VERSION_CHECK(5,12,0)
#include "scriptintegrationplugin.h"
#endif
#ifdef WITH_PYTHON
#include "pythonintegrationplugin.h"
#endif

#include "loggingcategories.h"
#include "typeutils.h"
#include "nymeasettings.h"
#include "version.h"
#include "plugininfocache.h"

#include "integrations/thingdiscoveryinfo.h"
#include "integrations/thingpairinginfo.h"
#include "integrations/thingsetupinfo.h"
#include "integrations/thingactioninfo.h"
#include "integrations/integrationplugin.h"
#include "integrations/thingutils.h"
#include "integrations/browseresult.h"
#include "integrations/browseritemresult.h"
#include "integrations/browseractioninfo.h"
#include "integrations/browseritemactioninfo.h"

#include "apikeysprovidersloader.h"

//#include "unistd.h"

#include "plugintimer.h"

#include <QPluginLoader>
#include <QStaticPlugin>
#include <QtPlugin>
#include <QDebug>
#include <QStringList>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>

ThingManagerImplementation::ThingManagerImplementation(HardwareManager *hardwareManager, const QLocale &locale, QObject *parent) :
    ThingManager(parent),
    m_hardwareManager(hardwareManager),
    m_locale(locale),
    m_translator(new Translator(this))
{
    foreach (const Interface &interface, ThingUtils::allInterfaces()) {
        m_supportedInterfaces.insert(interface.name(), interface);
    }

    // Migrate config from devices.conf (<0.20) to things.conf
    QString settingsPath = NymeaSettings::settingsPath();
    if (QFile::exists(settingsPath + "/devices.conf") && !QFile::exists(settingsPath + "/things.conf")) {
        qCDebug(dcThingManager()) << "Migrating config from devices.conf to things.conf";
        QFile oldFile(settingsPath + "/devices.conf");
        oldFile.copy(settingsPath + "/things.conf");
        QFile oldStateFile(settingsPath + "/devicestates.conf");
        oldStateFile.copy(settingsPath + "/thingstates.conf");
    }

    m_apiKeysProvidersLoader = new ApiKeysProvidersLoader(this);

    // Give hardware a chance to start up before loading plugins etc.
    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "loadConfiguredThings", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "startMonitoringAutoThings", Qt::QueuedConnection);

    // Make sure this is always emitted after plugins and things are loaded
    QMetaObject::invokeMethod(this, "onLoaded", Qt::QueuedConnection);

#ifdef WITH_PYTHON
    PythonIntegrationPlugin::initPython();
#endif
}

ThingManagerImplementation::~ThingManagerImplementation()
{

    delete m_translator;

    foreach (Thing *thing, m_configuredThings) {
        storeThingStates(thing);
        delete thing;
    }

    foreach (IntegrationPlugin *plugin, m_integrationPlugins) {
        if (plugin->parent() == this) {
            qCDebug(dcThingManager()) << "Deleting plugin" << plugin->pluginName();
            delete plugin;
        } else {
            qCDebug(dcThingManager()) << "Not deleting plugin" << plugin->pluginName();
        }
    }

#ifdef WITH_PYTHON
    PythonIntegrationPlugin::deinitPython();
#endif
}

QStringList ThingManagerImplementation::pluginSearchDirs()
{
    QStringList searchDirs;
    QByteArray envPath = qgetenv("NYMEA_PLUGINS_PATH");
    if (!envPath.isEmpty()) {
        searchDirs << QString(envPath).split(':');
    }

    foreach (QString libraryPath, QCoreApplication::libraryPaths()) {
        searchDirs << libraryPath.replace("qt5", "nymea");
    }
    foreach (QString libraryPath, QCoreApplication::libraryPaths()) {
        searchDirs << libraryPath.replace("plugins", "nymea/plugins");
    }
    searchDirs << QDir(QCoreApplication::applicationDirPath() + "/../lib/nymea/plugins/").absolutePath();
    searchDirs << QDir(QCoreApplication::applicationDirPath() + "/../plugins/").absolutePath();
    searchDirs << QDir(QCoreApplication::applicationDirPath() + "/../../../plugins/").absolutePath();
    searchDirs.removeDuplicates();
    return searchDirs;
}

QList<QJsonObject> ThingManagerImplementation::pluginsMetadata()
{
    QList<QJsonObject> pluginList;
    QStringList searchDirs;
    // Add first level of subdirectories to the plugin search dirs so we can point to a collection of plugins
    foreach (const QString &path, pluginSearchDirs()) {
        searchDirs.append(path);
        QDir dir(path);
        foreach (const QString &subdir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            searchDirs.append(path + '/' + subdir);
        }
    }

    foreach (const QString &path, searchDirs) {
        QDir dir(path);
        foreach (const QString &entry, dir.entryList({"*.so", "*.js", "*.py"}, QDir::Files)) {

            QFileInfo fi(path + '/' + entry);
            if (entry.startsWith("libnymea_integrationplugin") && entry.endsWith(".so")) {
                QPluginLoader loader(fi.absoluteFilePath());
                pluginList.append(loader.metaData().value("MetaData").toObject());
#if QT_VERSION >= QT_VERSION_CHECK(5,12,0)
            } else if (entry.startsWith("integrationplugin") && entry.endsWith(".js")) {
                QFile jsonFile(fi.absolutePath() + "/" + fi.baseName() + ".json");
                if (!jsonFile.open(QFile::ReadOnly)) {
                    qCDebug(dcThingManager()) << "Failed to open json file for:" << entry;
                    continue;
                }
                QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll());
                pluginList.append(jsonDoc.object());
#endif
            } else if (entry.startsWith("integrationplugin") && entry.endsWith(".py")) {
                QFile jsonFile(fi.absolutePath() + "/" + fi.baseName() + ".json");
                if (!jsonFile.open(QFile::ReadOnly)) {
                    qCDebug(dcThingManager()) << "Failed to open json file for:" << entry;
                    continue;
                }
                QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll());
                pluginList.append(jsonDoc.object());
            }
        }
    }
    return pluginList;
}

void ThingManagerImplementation::registerStaticPlugin(IntegrationPlugin *plugin)
{
    if (!plugin->metadata().isValid()) {
        qCWarning(dcThingManager()) << "Plugin metadata not valid. Not loading static plugin:" << plugin->pluginName();
        return;
    }
    loadPlugin(plugin);
}

IntegrationPlugins ThingManagerImplementation::plugins() const
{
    return m_integrationPlugins.values();
}

IntegrationPlugin *ThingManagerImplementation::plugin(const PluginId &pluginId) const
{
    return m_integrationPlugins.value(pluginId);
}

Thing::ThingError ThingManagerImplementation::setPluginConfig(const PluginId &pluginId, const ParamList &pluginConfig)
{
    IntegrationPlugin *plugin = m_integrationPlugins.value(pluginId);
    if (!plugin) {
        qCWarning(dcThingManager()) << "Could not set plugin configuration. There is no plugin with id" << pluginId.toString();
        return Thing::ThingErrorPluginNotFound;
    }

    Thing::ThingError verify = ThingUtils::verifyParams(plugin->configurationDescription(), pluginConfig);
    if (verify != Thing::ThingErrorNoError)
        return verify;
    ParamList params = buildParams(plugin->configurationDescription(), pluginConfig);

    Thing::ThingError result = plugin->setConfiguration(params);
    if (result != Thing::ThingErrorNoError)
        return result;

    NymeaSettings settings(NymeaSettings::SettingsRolePlugins);
    settings.beginGroup("PluginConfig");
    settings.beginGroup(plugin->pluginId().toString());

    foreach (const Param &param, pluginConfig) {
        settings.setValue(param.paramTypeId().toString(), param.value());
    }

    settings.endGroup();
    settings.endGroup();
    emit pluginConfigChanged(plugin->pluginId(), pluginConfig);
    return result;
}

Vendors ThingManagerImplementation::supportedVendors() const
{
    return m_supportedVendors.values();
}

Interfaces ThingManagerImplementation::supportedInterfaces() const
{
    return m_supportedInterfaces.values();
}

ThingClasses ThingManagerImplementation::supportedThings(const VendorId &vendorId) const
{
    if (vendorId.isNull()) {
        return m_supportedThings.values();
    }
    QList<ThingClass> ret;
    foreach (const ThingClass &thingClass, m_supportedThings) {
        if (!vendorId.isNull() && thingClass.vendorId() != vendorId) {
            continue;
        }
        ret.append(thingClass);
    }
    return ret;
}

ThingDiscoveryInfo* ThingManagerImplementation::discoverThings(const ThingClassId &thingClassId, const ParamList &params)
{
    ThingClass thingClass = findThingClass(thingClassId);
    if (!thingClass.isValid()) {
        qCWarning(dcThingManager) << "Thing discovery failed. Invalid thing class id:" << thingClassId.toString();
        ThingDiscoveryInfo *discoveryInfo = new ThingDiscoveryInfo(thingClassId, params, this);
        discoveryInfo->finish(Thing::ThingErrorThingClassNotFound);
        return discoveryInfo;
    }
    if (!thingClass.createMethods().testFlag(ThingClass::CreateMethodDiscovery)) {
        qCWarning(dcThingManager) << "Thing discovery failed. Thing class" << thingClass.name() << "cannot be discovered.";
        ThingDiscoveryInfo *discoveryInfo = new ThingDiscoveryInfo(thingClassId, params, this);
        discoveryInfo->finish(Thing::ThingErrorCreationMethodNotSupported);
        return discoveryInfo;
    }
    IntegrationPlugin *plugin = m_integrationPlugins.value(thingClass.pluginId());
    if (!plugin) {
        qCWarning(dcThingManager) << "Thing discovery failed. Plugin not found for thing class" << thingClass.name();
        ThingDiscoveryInfo *discoveryInfo = new ThingDiscoveryInfo(thingClassId, params, this);
        discoveryInfo->finish(Thing::ThingErrorPluginNotFound, tr("The plugin for this thing is not loaded."));
        return discoveryInfo;
    }

    Thing::ThingError result = ThingUtils::verifyParams(thingClass.discoveryParamTypes(), params);
    if (result != Thing::ThingErrorNoError) {
        qCWarning(dcThingManager) << "Thing discovery failed. Parameter verification failed.";
        ThingDiscoveryInfo *discoveryInfo = new ThingDiscoveryInfo(thingClassId, params, this);
        discoveryInfo->finish(result);
        return discoveryInfo;
    }
    ParamList effectiveParams = buildParams(thingClass.discoveryParamTypes(), params);

    ThingDiscoveryInfo *discoveryInfo = new ThingDiscoveryInfo(thingClassId, effectiveParams, this, 30000);
    connect(discoveryInfo, &ThingDiscoveryInfo::finished, this, [this, discoveryInfo](){
        if (discoveryInfo->status() != Thing::ThingErrorNoError) {
            qCWarning(dcThingManager()) << "Discovery failed:" << discoveryInfo->status() << discoveryInfo->displayMessage();
            return;
        }
        qCDebug(dcThingManager()) << "Discovery finished. Found things:" << discoveryInfo->thingDescriptors().count();
        foreach (const ThingDescriptor &descriptor, discoveryInfo->thingDescriptors()) {
            if (!descriptor.isValid()) {
                qCWarning(dcThingManager()) << "Descriptor is invalid. Not adding to results";
                continue;
            }
            m_discoveredThings.insert(descriptor.id(), descriptor);
        }
    });

    qCDebug(dcThingManager) << "Thing discovery for" << thingClass.name() << "started...";
    plugin->discoverThings(discoveryInfo);
    return discoveryInfo;
}

ThingSetupInfo* ThingManagerImplementation::addConfiguredThing(const ThingClassId &thingClassId, const ParamList &params, const QString &name)
{
    return addConfiguredThingInternal(thingClassId, name, params);
}

ThingSetupInfo *ThingManagerImplementation::addConfiguredThing(const ThingDescriptorId &thingDescriptorId, const ParamList &params, const QString &name)
{
    ThingDescriptor descriptor = m_discoveredThings.value(thingDescriptorId);
    if (!descriptor.isValid()) {
        qCWarning(dcThingManager()) << "Cannot add thing. ThingDescriptor" << thingDescriptorId << "not found.";
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        info->finish(Thing::ThingErrorThingDescriptorNotFound);
        return info;
    }

    ThingClass thingClass = findThingClass(descriptor.thingClassId());
    if (!thingClass.isValid()) {
        qCWarning(dcThingManager()) << "Cannot add thing. ThingClass" << descriptor.thingClassId() << "not found.";
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        info->finish(Thing::ThingErrorThingClassNotFound);
        return info;
    }
    if (!thingClass.createMethods().testFlag(ThingClass::CreateMethodDiscovery)) {
        qCWarning(dcThingManager()) << "Cannot add thing. This thing cannot be added via discovery.";
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        info->finish(Thing::ThingErrorCreationMethodNotSupported);
        return info;
    }

    // Merging params from descriptor and user provided ones
    ParamList finalParams = buildParams(thingClass.paramTypes(), params, descriptor.params());

    return addConfiguredThingInternal(descriptor.thingClassId(), name, finalParams, descriptor.parentId());
}

ThingSetupInfo* ThingManagerImplementation::reconfigureThing(const ThingId &thingId, const ParamList &params, const QString &name)
{
    Thing *thing = findConfiguredThing(thingId);
    if (!thing) {
        qCWarning(dcThingManager()) << "Cannot reconfigure thing. Thing with id" << thingId.toString() << "not found.";
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        info->finish(Thing::ThingErrorThingNotFound);
        return info;
    }

    ThingClass thingClass = findThingClass(thing->thingClassId());
    if (thingClass.id().isNull()) {
        qCWarning(dcThingManager()) << "Cannot reconfigure thing. ThingClass for thing" << thing->name() << thingId.toString() << "not found.";
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        info->finish(Thing::ThingErrorThingClassNotFound);
        return info;
    }

    foreach (const ParamType &paramType, thingClass.paramTypes()) {
        if (paramType.readOnly() && params.hasParam(paramType.id())) {
            ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
            qCWarning(dcThingManager()) << "Parameter" << paramType.name() << paramType.id() << "is not writable";
            info->finish(Thing::ThingErrorParameterNotWritable);
            return info;
        }
    }

    ParamList finalParams = buildParams(thingClass.paramTypes(), params);

    return reconfigureThingInternal(thing, finalParams, name);
}

ThingSetupInfo *ThingManagerImplementation::reconfigureThing(const ThingDescriptorId &thingDescriptorId, const ParamList &params, const QString &name)
{
    ThingDescriptor descriptor = m_discoveredThings.value(thingDescriptorId);
    if (!descriptor.isValid()) {
        qCWarning(dcThingManager()) << "Cannot reconfigure thing. No thing descriptor with ID" << thingDescriptorId << "found.";
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        info->finish(Thing::ThingErrorThingDescriptorNotFound);
        return info;
    }

    Thing *thing = findConfiguredThing(descriptor.thingId());
    if (!thing) {
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        qCWarning(dcThingManager()) << "Cannot reconfigure thing. No thing with ID" << descriptor.thingId() << "found.";
        info->finish(Thing::ThingErrorThingNotFound);
        return info;
    }

    ThingClass thingClass = findThingClass(thing->thingClassId());
    if (!thingClass.isValid()) {
        qCWarning(dcThingManager()) << "Cannot reconfigure tning. No ThingClass with ID" << thing->thingClassId() << "found.";
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        info->finish(Thing::ThingErrorThingClassNotFound);
        return info;
    }

    ParamList finalParams = buildParams(thing->thingClass().paramTypes(), params, descriptor.params());
    return reconfigureThingInternal(thing, finalParams, name);
}

ThingSetupInfo *ThingManagerImplementation::reconfigureThingInternal(Thing *thing, const ParamList &params, const QString &name)
{
    IntegrationPlugin *plugin = m_integrationPlugins.value(thing->thingClass().pluginId());
    if (!plugin) {
        qCWarning(dcThingManager()) << "Cannot reconfigure thing. Plugin for ThingClass" << thing->thingClassId().toString() << "not found.";
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        info->finish(Thing::ThingErrorPluginNotFound);
        return info;
    }

    Thing::ThingError result = ThingUtils::verifyParams(thing->thingClass().paramTypes(), params);
    if (result != Thing::ThingErrorNoError) {
        qCWarning(dcThingManager()) << "Cannot reconfigure thing. Params failed validation.";
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        info->finish(result);
        return info;
    }
    ParamList finalParams = buildParams(thing->thingClass().paramTypes(), params);

    // mark setup as incomplete
    thing->setSetupStatus(Thing::ThingSetupStatusInProgress, Thing::ThingErrorNoError);

    // set new params
    foreach (const Param &param, params) {
        thing->setParamValue(param.paramTypeId(), param.value());
    }

    if (!name.isEmpty()) {
        thing->setName(name);
    }

    // try to setup the thing with the new params
    ThingSetupInfo *info = new ThingSetupInfo(thing, this, 30000);
    plugin->setupThing(info);
    connect(info, &ThingSetupInfo::destroyed, thing, [=](){
        m_pendingSetups.remove(thing->id());
    });

    connect(info, &ThingSetupInfo::finished, this, [this, info](){

        if (info->status() != Thing::ThingErrorNoError) {
            qCWarning(dcThingManager()) << "Thing reconfiguration failed for" << info->thing()->name() << info->thing()->id().toString() << info->status() << info->displayMessage();
            info->thing()->setSetupStatus(Thing::ThingSetupStatusFailed, info->status(), info->displayMessage());
            // TODO: recover old params.??
            return;
        }

        storeConfiguredThings();

        postSetupThing(info->thing());
        info->thing()->setSetupStatus(Thing::ThingSetupStatusComplete, Thing::ThingErrorNoError);

        emit thingChanged(info->thing());

    });

    return info;

}

Thing::ThingError ThingManagerImplementation::editThing(const ThingId &thingId, const QString &name)
{
    Thing *thing = findConfiguredThing(thingId);
    if (!thing)
        return Thing::ThingErrorThingNotFound;

    thing->setName(name);

    return Thing::ThingErrorNoError;
}

Thing::ThingError ThingManagerImplementation::setThingSettings(const ThingId &thingId, const ParamList &settings)
{
    Thing *thing = findConfiguredThing(thingId);
    if (!thing) {
        qCWarning(dcThingManager()) << "Cannot set thing settings. Thing" << thingId.toString() << "not found";
        return Thing::ThingErrorThingNotFound;
    }
    Thing::ThingError status = ThingUtils::verifyParams(thing->thingClass().settingsTypes(), settings);
    if (status != Thing::ThingErrorNoError) {
        qCWarning(dcThingManager()) << "Error setting thing settings for" << thing->name() << thing->id().toString();
        return status;
    }
    // build a list of settings using: a) provided new settings b) previous settings and c) default values
    ParamList effectiveSettings = buildParams(thing->thingClass().settingsTypes(), settings, thing->settings());
    thing->setSettings(effectiveSettings);
    return Thing::ThingErrorNoError;
}

Thing::ThingError ThingManagerImplementation::setStateLogging(const ThingId &thingId, const StateTypeId &stateTypeId, bool enabled)
{
    Thing *thing = m_configuredThings.value(thingId);
    if (!thing) {
        qCWarning(dcThingManager()) << "Cannot configure event logging. Thing" << thingId.toString() << "not found";
        return Thing::ThingErrorThingNotFound;
    }
    if (!thing->thingClass().stateTypes().findById(stateTypeId).isValid()) {
        qCWarning(dcThingManager()) << "Cannot configure state logging. Thing" << thingId.toString() << "has no state type with id" << stateTypeId;
        return Thing::ThingErrorStateTypeNotFound;
    }
    QList<StateTypeId> loggedStateTypes = thing->loggedStateTypeIds();
    if (enabled && !loggedStateTypes.contains(stateTypeId)) {
        loggedStateTypes.append(stateTypeId);
        thing->setLoggedStateTypeIds(loggedStateTypes);
        emit thingChanged(thing);
    } else if (!enabled && loggedStateTypes.contains(stateTypeId)) {
        loggedStateTypes.removeAll(stateTypeId);
        thing->setLoggedStateTypeIds(loggedStateTypes);
        emit thingChanged(thing);
    }
    return Thing::ThingErrorNoError;
}

Thing::ThingError ThingManagerImplementation::setEventLogging(const ThingId &thingId, const EventTypeId &eventTypeId, bool enabled)
{
    Thing *thing = m_configuredThings.value(thingId);
    if (!thing) {
        qCWarning(dcThingManager()) << "Cannot configure event logging. Thing" << thingId.toString() << "not found";
        return Thing::ThingErrorThingNotFound;
    }
    if (!thing->thingClass().eventTypes().findById(eventTypeId).isValid()) {
        qCWarning(dcThingManager()) << "Cannot configure event logging. Thing" << thingId.toString() << "has no event type with id" << eventTypeId;
        return Thing::ThingErrorEventTypeNotFound;
    }
    QList<EventTypeId> loggedEventTypes = thing->loggedEventTypeIds();
    if (enabled && !loggedEventTypes.contains(eventTypeId)) {
        loggedEventTypes.append(eventTypeId);
        thing->setLoggedEventTypeIds(loggedEventTypes);
        emit thingChanged(thing);
    } else if (!enabled && loggedEventTypes.contains(eventTypeId)) {
        loggedEventTypes.removeAll(eventTypeId);
        thing->setLoggedEventTypeIds(loggedEventTypes);
        emit thingChanged(thing);
    }
    return Thing::ThingErrorNoError;
}

Thing::ThingError ThingManagerImplementation::setStateFilter(const ThingId &thingId, const StateTypeId &stateTypeId, Types::StateValueFilter filter)
{
    Thing *thing = m_configuredThings.value(thingId);
    if (!thing) {
        qCWarning(dcThingManager()) << "Cannot configure state filter. Thing" << thingId.toString() << "not found";
        return Thing::ThingErrorThingNotFound;
    }
    if (!thing->thingClass().stateTypes().findById(stateTypeId).isValid()) {
        qCWarning(dcThingManager()) << "Cannot configure state filter. Thing" << thingId.toString() << "has no state type with id" << stateTypeId;
        return Thing::ThingErrorEventTypeNotFound;
    }

    thing->setStateValueFilter(stateTypeId, filter);
    emit thingChanged(thing);
    return Thing::ThingErrorNoError;
}

ThingPairingInfo* ThingManagerImplementation::pairThing(const ThingClassId &thingClassId, const ParamList &params, const QString &name)
{
    PairingTransactionId transactionId = PairingTransactionId::createPairingTransactionId();

    ThingClass thingClass = m_supportedThings.value(thingClassId);
    if (!thingClass.isValid()) {
        qCWarning(dcThingManager) << "Cannot find a ThingClass with ID" << thingClassId.toString();
        ThingPairingInfo *info = new ThingPairingInfo(transactionId, thingClassId, ThingId(), name, ParamList(), ThingId(), this);
        info->finish(Thing::ThingErrorThingClassNotFound);
        return info;
    }

    // Create new thing id
    ThingId newThingId = ThingId::createThingId();

    // Use given params, if there are missing some, use the defaults ones.
    ParamList finalParams = buildParams(thingClass.paramTypes(), params);

    ThingPairingInfo *info = new ThingPairingInfo(transactionId, thingClassId, newThingId, name, finalParams, ThingId(), this, 30000);
    pairThingInternal(info);
    return info;
}

ThingPairingInfo* ThingManagerImplementation::pairThing(const ThingDescriptorId &thingDescriptorId, const ParamList &params, const QString &name)
{
    PairingTransactionId pairingTransactionId = PairingTransactionId::createPairingTransactionId();
    ThingDescriptor descriptor = m_discoveredThings.value(thingDescriptorId);
    if (!descriptor.isValid()) {
        qCWarning(dcThingManager) << "Cannot find a ThingDescriptor with ID" << thingDescriptorId.toString();
        ThingPairingInfo *info = new ThingPairingInfo(pairingTransactionId, ThingClassId(), ThingId(), name, ParamList(), ThingId(), this);
        info->finish(Thing::ThingErrorThingDescriptorNotFound);
        return info;
    }

    ThingClass thingClass = m_supportedThings.value(descriptor.thingClassId());
    if (!thingClass.isValid()) {
        qCWarning(dcThingManager) << "Cannot find a ThingClass with ID" << descriptor.thingClassId().toString();
        ThingPairingInfo *info = new ThingPairingInfo(pairingTransactionId, descriptor.thingClassId(), ThingId(), name, ParamList(), ThingId(), this);
        info->finish(Thing::ThingErrorThingClassNotFound);
        return info;
    }

    ThingId thingId = descriptor.thingId();
    // If it's a new thing (not a reconfiguration), create a new ThingId now.
    if (thingId.isNull()) {
        thingId = ThingId::createThingId();
    }

    // Use given params, if there are missing some, use the discovered ones.
    ParamList finalParams = buildParams(thingClass.paramTypes(), params, descriptor.params());

    ThingPairingInfo *info = new ThingPairingInfo(pairingTransactionId, descriptor.thingClassId(), thingId, name, finalParams, descriptor.parentId(), this, 30000);
    pairThingInternal(info);
    return info;
}

ThingPairingInfo *ThingManagerImplementation::pairThing(const ThingId &thingId, const ParamList &params, const QString &name)
{
    PairingTransactionId pairingTransactionId = PairingTransactionId::createPairingTransactionId();

    Thing *thing = findConfiguredThing(thingId);
    if (!thing) {
        qCWarning(dcThingManager) << "Cannot find a thing with ID" << thingId.toString();
        ThingPairingInfo *info = new ThingPairingInfo(pairingTransactionId, ThingClassId(), thingId, name, ParamList(), ThingId(), this);
        info->finish(Thing::ThingErrorThingDescriptorNotFound);
        return info;
    }

    // Use new params, if there are missing some, use the existing ones.
    ParamList finalParams = buildParams(thing->thingClass().paramTypes(), params, thing->params());

    ThingPairingInfo *info = new ThingPairingInfo(pairingTransactionId, thing->thingClassId(), thingId, name, finalParams, ThingId(), this, 30000);
    pairThingInternal(info);
    return info;
}

ThingPairingInfo *ThingManagerImplementation::confirmPairing(const PairingTransactionId &pairingTransactionId, const QString &username, const QString &secret)
{
    if (!m_pendingPairings.contains(pairingTransactionId)) {
        qCWarning(dcThingManager()) << "No pairing transaction with id" << pairingTransactionId.toString();
        ThingPairingInfo *info = new ThingPairingInfo(pairingTransactionId, ThingClassId(), ThingId(), QString(), ParamList(), ThingId(), this);
        info->finish(Thing::ThingErrorPairingTransactionIdNotFound);
        return info;
    }

    PairingContext context = m_pendingPairings.take(pairingTransactionId);
    ThingClassId thingClassId = context.thingClassId;

    ThingClass thingClass = m_supportedThings.value(thingClassId);
    IntegrationPlugin *plugin = m_integrationPlugins.value(thingClass.pluginId());
    if (!plugin) {
        qCWarning(dcThingManager) << "Can't find a plugin for this thing class:" << thingClass;
        ThingPairingInfo *info = new ThingPairingInfo(pairingTransactionId, thingClassId, context.thingId, context.thingName, context.params, context.parentId, this);
        info->finish(Thing::ThingErrorPluginNotFound);
        return info;
    }

    ThingId thingId = context.thingId;
    // If we already have a thing for this ID, we're reconfiguring an existing thing, else we're adding a new one.
    bool addNewThing = !m_configuredThings.contains(context.thingId);

    // We're using two different info objects here, one to hand over to the plugin for the pairing, the other we give out
    // to the user. After the internal one has finished, we'll start a setupThing job and finish the external pairingInfo only after
    // both, the internal pairing and the setup have completed.
    ThingPairingInfo *internalInfo = new ThingPairingInfo(pairingTransactionId, thingClassId, thingId, context.thingName, context.params, context.parentId, this);
    ThingPairingInfo *externalInfo = new ThingPairingInfo(pairingTransactionId, thingClassId, thingId, context.thingName, context.params, context.parentId, this);
    plugin->confirmPairing(internalInfo, username, secret);

    connect(internalInfo, &ThingPairingInfo::finished, this, [this, internalInfo, externalInfo, plugin, addNewThing](){

        // Internal pairing failed, so fail the exernal one too.
        if (internalInfo->status() != Thing::ThingErrorNoError) {
            qCWarning(dcThingManager()) << "ConfirmPairing failed for" << internalInfo->thingName() << internalInfo->thingClassId();
            externalInfo->finish(internalInfo->status(), internalInfo->displayMessage());
            return;
        }

        // Internal pairing succeeded, set up the thing.
        if (!addNewThing && !m_configuredThings.contains(internalInfo->thingId())) {
            qCWarning(dcThingManager) << "The thing to be reconfigured has disappeared!";
            externalInfo->finish(Thing::ThingErrorThingNotFound);
            return;
        }

        ThingClass thingClass = m_supportedThings.value(internalInfo->thingClassId());
        Thing *thing = nullptr;

        if (addNewThing) {
            thing = new Thing(plugin->pluginId(), thingClass, internalInfo->thingId(), this);
            if (internalInfo->thingName().isEmpty()) {
                thing->setName(thingClass.displayName());
            } else {
                thing->setName(internalInfo->thingName());
            }
        } else {
            thing = m_configuredThings.value(internalInfo->thingId());
            thing->setSetupStatus(Thing::ThingSetupStatusInProgress, Thing::ThingErrorNoError);
            qCDebug(dcThingManager()) << "Reconfiguring thing" << thing;
        }

        thing->setParams(internalInfo->params());
        ParamList settings = buildParams(thingClass.settingsTypes(), ParamList());
        thing->setSettings(settings);

        initThing(thing);

        ThingSetupInfo *info = setupThing(thing);
        connect(info, &ThingSetupInfo::finished, thing, [this, info, externalInfo, addNewThing](){

            externalInfo->finish(info->status(), info->displayMessage());

            if (info->status() != Thing::ThingErrorNoError) {
                if (addNewThing) {
                    qCWarning(dcThingManager()) << "Failed to set up thing" << info->thing()->name()
                                                 << "Not adding thing to the system. Error:"
                                                  << info->status() << info->displayMessage();
                    info->thing()->deleteLater();

                } else {
                    qCWarning(dcThingManager()) << "Failed to reconfigure thing" << info->thing()->name() <<
                                                    "Error:" << info->status() << info->displayMessage();
                    info->thing()->setSetupStatus(Thing::ThingSetupStatusFailed, info->status(), info->displayMessage());
                    // TODO: restore parameters?
                }

                return;
            }

            qCDebug(dcThingManager()) << "Setup complete for thing" << info->thing();
            info->thing()->setSetupStatus(Thing::ThingSetupStatusComplete, Thing::ThingErrorNoError);

            if (addNewThing) {
                qCDebug(dcThingManager()) << "Thing added:" << info->thing();
                registerThing(info->thing());
                emit thingAdded(info->thing());
            } else {
                emit thingChanged(info->thing());
            }
            storeConfiguredThings();

            postSetupThing(info->thing());
        });

    });

    return externalInfo;
}

ThingSetupInfo* ThingManagerImplementation::addConfiguredThingInternal(const ThingClassId &thingClassId, const QString &name, const ParamList &params, const ThingId &parentId)
{
    ThingClass thingClass = findThingClass(thingClassId);
    if (thingClass.id().isNull()) {
        qCWarning(dcThingManager()) << "Cannot add thing. ThingClass" << thingClassId << "not found.";
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        info->finish(Thing::ThingErrorThingClassNotFound);
        return info;
    }

    if (thingClass.setupMethod() != ThingClass::SetupMethodJustAdd) {
        qCWarning(dcThingManager()) << "Cannot add thing. This thing cannot be added this way. (SetupMethodJustAdd)";
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        info->finish(Thing::ThingErrorCreationMethodNotSupported);
        return info;
    }

    ThingId thingId = ThingId::createThingId();
    // Chances are like 0, but...
    while (m_configuredThings.contains(thingId)) {
        thingId = ThingId::createThingId();
    }

    IntegrationPlugin *plugin = m_integrationPlugins.value(thingClass.pluginId());
    if (!plugin) {
        qCWarning(dcThingManager()) << "Cannot add thing. Plugin for thing class" << thingClass.name() << "not found.";
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        info->finish(Thing::ThingErrorPluginNotFound);
        return info;
    }

    // set params
    Thing::ThingError paramsResult = ThingUtils::verifyParams(thingClass.paramTypes(), params);
    if (paramsResult != Thing::ThingErrorNoError) {
        qCWarning(dcThingManager()) << "Cannot add thing. Parameter verification failed.";
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        info->finish(paramsResult);
        return info;
    }

    ParamList effectiveParams = buildParams(thingClass.paramTypes(), params);
    Thing *thing = new Thing(plugin->pluginId(), thingClass, thingId, this);
    thing->setParentId(parentId);
    if (name.isEmpty()) {
        thing->setName(thingClass.name());
    } else {
        thing->setName(name);
    }
    thing->setParams(effectiveParams);

    // set settings (init with defaults)
    ParamList settings = buildParams(thingClass.settingsTypes(), ParamList());
    thing->setSettings(settings);


    initThing(thing);

    ThingSetupInfo *info = setupThing(thing);
    connect(info, &ThingSetupInfo::finished, this, [this, info](){
        if (info->status() != Thing::ThingErrorNoError) {
            qCWarning(dcThingManager) << "Thing setup failed. Not adding thing to system.";
            info->thing()->deleteLater();
            return;
        }

        info->thing()->setSetupStatus(Thing::ThingSetupStatusComplete, Thing::ThingErrorNoError);

        qCDebug(dcThingManager) << "Thing setup complete.";
        registerThing(info->thing());
        storeConfiguredThings();
        emit thingAdded(info->thing());
        postSetupThing(info->thing());
    });

    return info;
}

Thing::ThingError ThingManagerImplementation::removeConfiguredThing(const ThingId &thingId)
{
    // We're checking thingSetupStatus and abort any pending setup here. As setup finished()
    // comes in as a QueuedConnection, make sure to process all events before going on so we
    // don't end up aborting an already finished setup instead of calling thingRemoved() on it.
    qApp->processEvents();

    Thing *thing = m_configuredThings.take(thingId);
    if (!thing) {
        return Thing::ThingErrorThingNotFound;
    }
    IntegrationPlugin *plugin = m_integrationPlugins.value(thing->pluginId());
    if (!plugin) {
        qCWarning(dcThingManager()).nospace() << "Plugin not loaded for thing " << thing->name() << ". Not calling thingRemoved on plugin.";
    } else if (thing->setupStatus() == Thing::ThingSetupStatusInProgress) {
        qCWarning(dcThingManager()).nospace() << "Thing " << thing->name() << " is still being set up. Aborting setup.";
        ThingSetupInfo *setupInfo = m_pendingSetups.value(thingId);
        emit setupInfo->aborted();
    } else if (thing->setupStatus() == Thing::ThingSetupStatusComplete) {
        plugin->thingRemoved(thing);
    }

    thing->deleteLater();

    NymeaSettings settings(NymeaSettings::SettingsRoleThings);
    settings.beginGroup("ThingConfig");
    settings.beginGroup(thingId.toString());
    settings.remove("");
    settings.endGroup();

    QFile::remove(statesCacheFile(thingId));

    foreach (const IOConnectionId &ioConnectionId, m_ioConnections.keys()) {
        IOConnection ioConnection = m_ioConnections.value(ioConnectionId);
        if (ioConnection.inputThingId() == thing->id() || ioConnection.outputThingId() == thing->id()) {
            disconnectIO(ioConnectionId);
        }
    }

    emit thingRemoved(thingId);

    return Thing::ThingErrorNoError;
}

BrowseResult *ThingManagerImplementation::browseThing(const ThingId &thingId, const QString &itemId, const QLocale &locale)
{
    Thing *thing = m_configuredThings.value(thingId);

    BrowseResult *result = new BrowseResult(thing, this, itemId, locale, this, 30000);

    if (!thing) {
        qCWarning(dcThingManager()) << "Cannot browse thing. No such thing:" << thingId.toString();
        result->finish(Thing::ThingErrorThingNotFound);
        return result;
    }

    IntegrationPlugin *plugin = m_integrationPlugins.value(thing->pluginId());
    if (!plugin) {
        qCWarning(dcThingManager()) << "Cannot browse thing. Plugin not found for thing" << thing;
        return result;
    }

    if (!thing->setupComplete()) {
        qCWarning(dcThingManager()) << "Cannot browse thing. Thing did not finish setup" << thing;
        return result;
    }

    if (!thing->thingClass().browsable()) {
        qCWarning(dcThingManager()) << "Cannot browse thing. ThingClass" << thing->thingClass().name() << "is not browsable.";
        result->finish(Thing::ThingErrorUnsupportedFeature);
        return result;
    }

    plugin->browseThing(result);
    connect(result, &BrowseResult::finished, this, [result](){
        if (result->status() != Thing::ThingErrorNoError) {
            qCWarning(dcThingManager()) << "Browse thing failed:" << result->status();
        }
    });
    return result;
}

BrowserItemResult *ThingManagerImplementation::browserItemDetails(const ThingId &thingId, const QString &itemId, const QLocale &locale)
{
    Thing *thing = m_configuredThings.value(thingId);

    BrowserItemResult *result = new BrowserItemResult(thing, this, itemId, locale, this, 30000);

    if (!thing) {
        qCWarning(dcThingManager()) << "Cannot browse thing. No such thing:" << thingId.toString();
        result->finish(Thing::ThingErrorThingNotFound);
        return result;
    }

    IntegrationPlugin *plugin = m_integrationPlugins.value(thing->pluginId());
    if (!plugin) {
        qCWarning(dcThingManager()) << "Cannot browse thing. Plugin not found for thing" << thing;
        return result;
    }

    if (thing->setupStatus() != Thing::ThingSetupStatusComplete) {
        qCWarning(dcThingManager()) << "Cannot browse thing. Thing did not finish setup" << thing;
        return result;
    }

    if (!thing->thingClass().browsable()) {
        qCWarning(dcThingManager()) << "Cannot browse thing. ThingClass" << thing->thingClass().name() << "is not browsable.";
        result->finish(Thing::ThingErrorUnsupportedFeature);
        return result;
    }

    plugin->browserItem(result);
    connect(result, &BrowserItemResult::finished, this, [result](){
        if (result->status() != Thing::ThingErrorNoError) {
            qCWarning(dcThingManager()) << "Browsing thing failed:" << result->status();
        }
    });
    return result;
}

BrowserActionInfo* ThingManagerImplementation::executeBrowserItem(const BrowserAction &browserAction)
{
    Thing *thing = m_configuredThings.value(browserAction.thingId());

    BrowserActionInfo *info = new BrowserActionInfo(thing, this, browserAction, this, 30000);

    if (!thing) {
        info->finish(Thing::ThingErrorThingNotFound);
        return info;
    }

    IntegrationPlugin *plugin = m_integrationPlugins.value(thing->pluginId());
    if (!plugin) {
        qCWarning(dcThingManager()) << "Cannot browse thing. Plugin not found for thing" << thing;
        info->finish(Thing::ThingErrorPluginNotFound);
        return info;
    }

    if (thing->setupStatus() != Thing::ThingSetupStatusComplete) {
        qCWarning(dcThingManager()) << "Cannot browse thing. Thing did not finish setup" << thing;
        info->finish(Thing::ThingErrorSetupFailed);
        return info;
    }

    if (!thing->thingClass().browsable()) {
        info->finish(Thing::ThingErrorUnsupportedFeature);
        return info;
    }
    plugin->executeBrowserItem(info);
    return info;
}

BrowserItemActionInfo* ThingManagerImplementation::executeBrowserItemAction(const BrowserItemAction &browserItemAction)
{
    Thing *thing = m_configuredThings.value(browserItemAction.thingId());

    BrowserItemActionInfo *info = new BrowserItemActionInfo(thing, this, browserItemAction, this, 30000);

    if (!thing) {
        info->finish(Thing::ThingErrorThingNotFound);
        return info;
    }

    IntegrationPlugin *plugin = m_integrationPlugins.value(thing->pluginId());
    if (!plugin) {
        qCWarning(dcThingManager()) << "Cannot execute browser item action. Plugin not found for thing" << thing;
        info->finish(Thing::ThingErrorPluginNotFound);
        return info;
    }

    if (thing->setupStatus() != Thing::ThingSetupStatusComplete) {
        qCWarning(dcThingManager()) << "Cannot execute browser item action. Thing did not finish setup" << thing;
        info->finish(Thing::ThingErrorSetupFailed);
        return info;
    }

    if (!thing->thingClass().browsable()) {
        info->finish(Thing::ThingErrorUnsupportedFeature);
        return info;
    }
    // TODO: check browserItemAction.params with ThingClass

    plugin->executeBrowserItemAction(info);
    return info;
}

IOConnections ThingManagerImplementation::ioConnections(const ThingId &thingId) const
{
    if (thingId.isNull()) {
        return m_ioConnections.values();
    }
    IOConnections ioConnections;
    foreach (const IOConnection &ioConnection, m_ioConnections) {
        if (ioConnection.inputThingId() == thingId || ioConnection.outputThingId() == thingId) {
            ioConnections.append(ioConnection);
        }
    }
    return ioConnections;
}

IOConnectionResult ThingManagerImplementation::connectIO(const IOConnection &connection)
{
    IOConnectionResult result;

    // Do some sanity checks
    Thing *inputThing = m_configuredThings.value(connection.inputThingId());
    if (!inputThing) {
        qCWarning(dcThingManager()) << "Could not find inputThing" << connection.inputThingId() << "in configured things. Not adding IO connection.";
        result.error = Thing::ThingErrorThingNotFound;
        return result;
    }
    if (!inputThing->thingClass().stateTypes().contains(connection.inputStateTypeId())) {
        qCWarning(dcThingManager()) << "Input thing" << inputThing->name() << "does not have a state with id" << connection.inputStateTypeId();
        result.error = Thing::ThingErrorStateTypeNotFound;
        return result;
    }
    StateType inputStateType = inputThing->thingClass().stateTypes().findById(connection.inputStateTypeId());

    // Check if this is actually an input
    if (inputStateType.ioType() != Types::IOTypeDigitalInput && inputStateType.ioType() != Types::IOTypeAnalogInput) {
        qCWarning(dcThingManager()) << "The given input state is neither a digital nor an analog input.";
        result.error = Thing::ThingErrorInvalidParameter;
        return result;
    }

    Thing *outputThing = m_configuredThings.value(connection.outputThingId());
    if (!outputThing) {
        qCWarning(dcThingManager()) << "Could not find outputThing" << connection.outputThingId() << "in configured things. Not adding IO connection.";
        result.error = Thing::ThingErrorThingNotFound;
        return result;
    }
    if (!outputThing->thingClass().stateTypes().contains(connection.outputStateTypeId())) {
        qCWarning(dcThingManager()) << "Output thing" << outputThing->name() << "does not have a state with id" << connection.outputStateTypeId();
        result.error = Thing::ThingErrorStateTypeNotFound;
        return result;
    }
    StateType outputStateType = outputThing->thingClass().stateTypes().findById(connection.outputStateTypeId());

    // Check if this is actually an output
    if (outputStateType.ioType() != Types::IOTypeDigitalOutput && outputStateType.ioType() != Types::IOTypeAnalogOutput) {
        qCWarning(dcThingManager()) << "The given output state is neither a digital nor an analog output.";
        result.error = Thing::ThingErrorInvalidParameter;
        return result;
    }

    // Check if io types are compatible
    if (inputStateType.ioType() == Types::IOTypeDigitalInput && outputStateType.ioType() != Types::IOTypeDigitalOutput) {
        qCWarning(dcThingManager()) << "Cannot connect IOs of different type:" << inputStateType.ioType() << "is not compatible with" << outputStateType.ioType();
        result.error = Thing::ThingErrorInvalidParameter;
        return result;
    }
    if (inputStateType.ioType() == Types::IOTypeAnalogInput && outputStateType.ioType() != Types::IOTypeAnalogOutput) {
        qCWarning(dcThingManager()) << "Cannot connect IOs of different type:" << inputStateType.ioType() << "is not compatible with" << outputStateType.ioType();
        result.error = Thing::ThingErrorInvalidParameter;
        return result;
    }

    // Check if either input or output is already connected
    foreach (const IOConnectionId &id, m_ioConnections.keys()) {
        if (m_ioConnections.value(id).inputThingId() == connection.inputThingId() && m_ioConnections.value(id).inputStateTypeId() == connection.inputStateTypeId()) {
            qCDebug(dcThingManager()).nospace() << "Thing " << inputThing->name() << " already has an IO connection on " << inputStateType.displayName() << ". Replacing old connection.";
            disconnectIO(id);
            continue;
        }
        if (m_ioConnections.value(id).outputThingId() == connection.outputThingId() && m_ioConnections.value(id).outputStateTypeId() == connection.outputStateTypeId()) {
            qCDebug(dcThingManager()).nospace() << "Thing " << inputThing->name() << " already has an IO connection on " << inputStateType.displayName() << ". Replacing old connection.";
            disconnectIO(id);
        }
    }

    // Finally add the connection
    m_ioConnections.insert(connection.id(), connection);

    storeIOConnections();

    emit ioConnectionAdded(connection);

    qCDebug(dcThingManager()) << "IO connected added:" << inputThing << "->" << outputThing;

    // Sync initial state
    syncIOConnection(inputThing, connection.inputStateTypeId());

    result.error = Thing::ThingErrorNoError;
    result.ioConnectionId = connection.id();
    return result;
}

Thing::ThingError ThingManagerImplementation::disconnectIO(const IOConnectionId &ioConnectionId)
{
    if (!m_ioConnections.contains(ioConnectionId)) {
        qCWarning(dcThingManager()) << "IO connection" << ioConnectionId << "not found. Cannot disconnect.";
        return Thing::ThingErrorItemNotFound;
    }
    m_ioConnections.remove(ioConnectionId);

    NymeaSettings settings(NymeaSettings::SettingsRoleIOConnections);
    settings.beginGroup("IOConnections");
    settings.remove(ioConnectionId.toString());
    settings.endGroup();

    qCDebug(dcThingManager()) << "IO connection disconnected:" << ioConnectionId;

    emit ioConnectionRemoved(ioConnectionId);
    return Thing::ThingErrorNoError;
}

QString ThingManagerImplementation::translate(const PluginId &pluginId, const QString &string, const QLocale &locale)
{
    return m_translator->translate(pluginId, string, locale);
}

ParamType ThingManagerImplementation::translateParamType(const PluginId &pluginId, const ParamType &paramType, const QLocale &locale)
{
    ParamType translatedParamType = paramType;
    translatedParamType.setDisplayName(translate(pluginId, paramType.displayName(), locale));
    return translatedParamType;
}

StateType ThingManagerImplementation::translateStateType(const PluginId &pluginId, const StateType &stateType, const QLocale &locale)
{
    StateType translatedStateType = stateType;
    translatedStateType.setDisplayName(translate(pluginId, stateType.displayName(), locale));
    return translatedStateType;
}

EventType ThingManagerImplementation::translateEventType(const PluginId &pluginId, const EventType &eventType, const QLocale &locale)
{
    EventType translatedEventType = eventType;
    translatedEventType.setDisplayName(translate(pluginId, eventType.displayName(), locale));
    return translatedEventType;
}

ActionType ThingManagerImplementation::translateActionType(const PluginId &pluginId, const ActionType &actionType, const QLocale &locale)
{
    ActionType translatedActionType = actionType;
    translatedActionType.setDisplayName(translate(pluginId, actionType.displayName(), locale));
    return translatedActionType;
}

ThingClass ThingManagerImplementation::translateThingClass(const ThingClass &thingClass, const QLocale &locale)
{
    ThingClass translatedThingClass = thingClass;
    translatedThingClass.setDisplayName(translate(thingClass.pluginId(), thingClass.displayName(), locale));

    ParamTypes translatedSettingsTypes;
    foreach (const ParamType &paramType, thingClass.settingsTypes()) {
        translatedSettingsTypes.append(translateParamType(thingClass.pluginId(), paramType, locale));
    }
    translatedThingClass.setSettingsTypes(translatedSettingsTypes);

    ParamTypes translatedParamTypes;
    foreach (const ParamType &paramType, thingClass.paramTypes()) {
        translatedParamTypes.append(translateParamType(thingClass.pluginId(), paramType, locale));
    }
    translatedThingClass.setParamTypes(translatedParamTypes);

    StateTypes translatedStateTypes;
    foreach (const StateType &stateType, thingClass.stateTypes()) {
        translatedStateTypes.append(translateStateType(thingClass.pluginId(), stateType, locale));
    }
    translatedThingClass.setStateTypes(translatedStateTypes);

    EventTypes translatedEventTypes;
    foreach (const EventType &eventType, thingClass.eventTypes()) {
        translatedEventTypes.append(translateEventType(thingClass.pluginId(), eventType, locale));
    }
    translatedThingClass.setEventTypes(translatedEventTypes);

    ActionTypes translatedActionTypes;
    foreach (const ActionType &actionType, thingClass.actionTypes()) {
        translatedActionTypes.append(translateActionType(thingClass.pluginId(), actionType, locale));
    }
    translatedThingClass.setActionTypes(translatedActionTypes);

    return translatedThingClass;
}

Vendor ThingManagerImplementation::translateVendor(const Vendor &vendor, const QLocale &locale)
{
    IntegrationPlugin *plugin = nullptr;
    foreach (IntegrationPlugin *p, m_integrationPlugins) {
        if (p->supportedVendors().contains(vendor)) {
            plugin = p;
        }
    }
    if (!plugin) {
        return vendor;
    }

    Vendor translatedVendor = vendor;
    translatedVendor.setDisplayName(translate(plugin->pluginId(), vendor.displayName(), locale));
    return translatedVendor;
}

Thing *ThingManagerImplementation::findConfiguredThing(const ThingId &id) const
{
    foreach (Thing *thing, m_configuredThings) {
        if (thing->id() == id) {
            return thing;
        }
    }
    return nullptr;
}

Things ThingManagerImplementation::configuredThings() const
{
    return m_configuredThings.values();
}

Things ThingManagerImplementation::findConfiguredThings(const ThingClassId &thingClassId) const
{
    QList<Thing*> ret;
    foreach (Thing *thing, m_configuredThings) {
        if (thing->thingClassId() == thingClassId) {
            ret << thing;
        }
    }
    return ret;
}

Things ThingManagerImplementation::findConfiguredThings(const QString &interface) const
{
    QList<Thing*> ret;
    foreach (Thing *thing, m_configuredThings) {
        ThingClass thingClass = m_supportedThings.value(thing->thingClassId());
        if (thingClass.interfaces().contains(interface)) {
            ret.append(thing);
        }
    }
    return ret;
}

Things ThingManagerImplementation::findChilds(const ThingId &id) const
{
    QList<Thing *> ret;
    foreach (Thing *d, m_configuredThings) {
        if (d->parentId() == id) {
            ret.append(d);
        }
    }
    return ret;
}

ThingClass ThingManagerImplementation::findThingClass(const ThingClassId &thingClassId) const
{
    foreach (const ThingClass &thingClass, m_supportedThings) {
        if (thingClass.id() == thingClassId) {
            return thingClass;
        }
    }
    return ThingClass();
}

ThingActionInfo *ThingManagerImplementation::executeAction(const Action &action)
{
    Action finalAction = action;
    Thing *thing = m_configuredThings.value(action.thingId());
    if (!thing) {
        qCWarning(dcThingManager()) << "Cannot execute action. No such thing:" << action.thingId();
        ThingActionInfo *info = new ThingActionInfo(nullptr, action, this);
        info->finish(Thing::ThingErrorThingNotFound);
        return info;
    }

    if (!thing->setupComplete()) {
        qCWarning(dcThingManager()) << "Cannot execute action. Thing" << thing->name() << "hasn't completed setup.";
        ThingActionInfo *info = new ThingActionInfo(nullptr, action, this);
        info->finish(Thing::ThingErrorSetupFailed);
        return info;
    }

    // Make sure this thing has an action type with this id
    ThingClass thingClass = findThingClass(thing->thingClassId());
    ActionType actionType = thingClass.actionTypes().findById(action.actionTypeId());
    if (actionType.id().isNull()) {
        qCWarning(dcThingManager()) << "Cannot execute action. No such action type" << action.actionTypeId();
        ThingActionInfo *info = new ThingActionInfo(thing, action, this);
        info->finish(Thing::ThingErrorActionTypeNotFound);
        return info;
    }

    // If there's a stateType with the same id, we'll need to take min/max values from the state as
    // they might change at runtime
    ParamTypes paramTypes = actionType.paramTypes();
    StateType stateType = thingClass.stateTypes().findById(action.actionTypeId());
    if (!stateType.id().isNull()) {
        ParamType pt = actionType.paramTypes().at(0);
        pt.setMinValue(thing->state(stateType.id()).minValue());
        pt.setMaxValue(thing->state(stateType.id()).maxValue());
        paramTypes = ParamTypes() << pt;
    }

    Thing::ThingError paramCheck = ThingUtils::verifyParams(paramTypes, action.params());
    if (paramCheck != Thing::ThingErrorNoError) {
        qCWarning(dcThingManager()) << "Cannot execute action. Parameter verification failed.";
        ThingActionInfo *info = new ThingActionInfo(thing, action, this);
        info->finish(paramCheck);
        return info;
    }
    ParamList finalParams = buildParams(actionType.paramTypes(), action.params());
    finalAction.setParams(finalParams);

    ThingActionInfo *info = new ThingActionInfo(thing, finalAction, this, 30000);

    IntegrationPlugin *plugin = m_integrationPlugins.value(thing->pluginId());
    if (!plugin) {
        qCWarning(dcThingManager()) << "Cannot execute action. Plugin not found for device" << thing->name();
        info->finish(Thing::ThingErrorPluginNotFound);
        return info;
    }

    connect(info, &ThingActionInfo::finished, this, [=](){
        emit actionExecuted(action, info->status());
    });

    plugin->executeAction(info);

    return info;
}

void ThingManagerImplementation::loadPlugins()
{    
    QStringList searchDirs;
    // Add first level of subdirectories to the plugin search dirs so we can point to a collection of plugins
    foreach (const QString &path, pluginSearchDirs()) {
        searchDirs.append(path);
        QDir dir(path);
        foreach (const QString &subdir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            searchDirs.append(path + '/' + subdir);
        }
    }

    foreach (const QString &path, searchDirs) {
        QDir dir(path);
        qCDebug(dcThingManager) << "Loading plugins from:" << dir.absolutePath();
        foreach (const QString &entry, dir.entryList({"*.so", "*.js", "*.py"}, QDir::Files)) {

            IntegrationPlugin *plugin = nullptr;

            QFileInfo fi(path + '/' + entry);
            if (entry.startsWith("libnymea_integrationplugin") && entry.endsWith(".so")) {
                plugin = createCppIntegrationPlugin(fi.absoluteFilePath());

            } else if (entry.startsWith("integrationplugin") && entry.endsWith(".js")) {
#if QT_VERSION >= QT_VERSION_CHECK(5,12,0)
                ScriptIntegrationPlugin *p = new ScriptIntegrationPlugin(this);
                bool ok = p->loadScript(fi.absoluteFilePath());
                if (ok) {
                    plugin = p;
                } else {
                    delete p;
                }
#else
                qCWarning(dcThingManager()) << "Not loading JS plugin as JS plugin support is not included in this nymea instance.";
#endif
            } else if (entry.startsWith("integrationplugin") && entry.endsWith(".py")) {
#ifdef WITH_PYTHON
                PythonIntegrationPlugin *p = new PythonIntegrationPlugin(this);
                bool ok = p->loadScript(fi.absoluteFilePath());
                if (ok) {
                    plugin = p;
                } else {
                    delete p;
                }
#else
                qCWarning(dcThingManager()) << "Not loading Python plugin as Python plugin support is not included in this nymea instance.";
#endif
            } else {
                // Not a known plugin type
                continue;
            }

            if (!plugin) {
                qCWarning(dcThingManager()) << "Error loading plugin:" << fi.absoluteFilePath();
                continue;
            }

            if (m_integrationPlugins.contains(plugin->pluginId())) {
                qCWarning(dcThingManager()) << "A plugin with this ID is already loaded. Not loading" << entry << plugin->pluginId();
                delete plugin;
                continue;
            }
            loadPlugin(plugin);
            PluginInfoCache::cachePluginInfo(plugin->metadata().jsonObject());
        }
    }
}

void ThingManagerImplementation::loadPlugin(IntegrationPlugin *pluginIface)
{
    // Populate the API storage for the plugin.
    // NOTE:
    // Right now we grant access to every api key requested in the JSON file. This means, an attacker could just
    // write a plugin requesting a certain key and load it. This is not an actual problem right now as
    // deployments that allow loading random plugins don't ship any high security keys. Once nymea supports
    // a "plugin store" and allows loading 3rd party plugins along with a more sensitive api key provider,
    // the plugins JSON needs to be reviewd by the store owner and signed with a store key. Only signed plugins
    // should be granted access to their requested keys.
    ApiKeyStorage *apiKeyStorage = new ApiKeyStorage(pluginIface);
    QStringList requestedKeys = pluginIface->metadata().apiKeys();
    foreach (const QString &apiKeyName, pluginIface->metadata().apiKeys()) {
        if (m_apiKeysProvidersLoader->allApiKeys().contains(apiKeyName)) {
            ApiKey apiKey = m_apiKeysProvidersLoader->allApiKeys().value(apiKeyName);
            apiKeyStorage->insertKey(apiKeyName, apiKey);
            requestedKeys.removeAll(apiKeyName);
        }
    }
    if (!requestedKeys.isEmpty()) {
        qCWarning(dcThingManager()).nospace() << "Unable to load API keys for plugin " << pluginIface->metadata().pluginName() << ": " << requestedKeys;
    }
    pluginIface->setParent(this);
    pluginIface->initPlugin(this, m_hardwareManager, apiKeyStorage);

    qCDebug(dcThingManager) << "**** Loaded plugin" << pluginIface->pluginName();
    foreach (const Vendor &vendor, pluginIface->supportedVendors()) {
        qCDebug(dcThingManager) << "* Loaded vendor:" << vendor.name() << vendor.id();
        if (m_supportedVendors.contains(vendor.id()))
            continue;

        m_supportedVendors.insert(vendor.id(), vendor);
    }

    foreach (const ThingClass &thingClass, pluginIface->supportedThings()) {
        if (!m_supportedVendors.contains(thingClass.vendorId())) {
            qCWarning(dcThingManager) << "Vendor not found. Ignoring thing. VendorId:" << thingClass.vendorId() << "ThingClass:" << thingClass.name() << thingClass.id();
            continue;
        }
        m_vendorThingMap[thingClass.vendorId()].append(thingClass.id());
        m_supportedThings.insert(thingClass.id(), thingClass);
        qCDebug(dcThingManager) << "* Loaded thing class:" << thingClass.name();
    }

    NymeaSettings settings(NymeaSettings::SettingsRolePlugins);
    settings.beginGroup("PluginConfig");
    ParamList params;

    settings.beginGroup(pluginIface->pluginId().toString());
    foreach (const ParamType &paramType, pluginIface->configurationDescription()) {
        QVariant value = paramType.defaultValue();
        if (settings.contains(paramType.id().toString())) {
            value = settings.value(paramType.id().toString());
        } else if (settings.childGroups().contains(paramType.id().toString())) {
            // 0.12.2 - 0.22 used to store it in subgroups
            settings.beginGroup(paramType.id().toString());
            value = settings.value("value");
            settings.endGroup();
        }
        value.convert(paramType.type());
        Param param(paramType.id(), value);
        params.append(param);
    }
    settings.endGroup(); // pluginId

    settings.endGroup(); // PluginConfig

    if (params.count() > 0) {
        Thing::ThingError status = pluginIface->setConfiguration(params);
        if (status != Thing::ThingErrorNoError) {
            qCWarning(dcThingManager) << "Error setting params to plugin. Broken configuration?";
        }
    }

    // Call the init method of the plugin
    pluginIface->init();

    m_integrationPlugins.insert(pluginIface->pluginId(), pluginIface);

    connect(pluginIface, &IntegrationPlugin::emitEvent, this, &ThingManagerImplementation::onEventTriggered, Qt::QueuedConnection);
    connect(pluginIface, &IntegrationPlugin::autoThingsAppeared, this, &ThingManagerImplementation::onAutoThingsAppeared, Qt::QueuedConnection);
    connect(pluginIface, &IntegrationPlugin::autoThingDisappeared, this, &ThingManagerImplementation::onAutoThingDisappeared, Qt::QueuedConnection);
}

void ThingManagerImplementation::loadConfiguredThings()
{
    bool needsMigration = false;
    NymeaSettings settings(NymeaSettings::SettingsRoleThings);
    if (settings.childGroups().contains("ThingConfig")) {
        settings.beginGroup("ThingConfig");
    } else {
        settings.beginGroup("DeviceConfig");
        needsMigration = true;
    }
    qCDebug(dcThingManager) << "Loading things from" << settings.fileName();
    foreach (const QString &idString, settings.childGroups()) {
        settings.beginGroup(idString);
        QString thingName = settings.value("thingName").toString();
        if (!settings.contains("thingName")) { // nymea < 0.20
            thingName = settings.value("devicename").toString();
        }
        PluginId pluginId = PluginId(settings.value("pluginid").toString());
        IntegrationPlugin *plugin = m_integrationPlugins.value(pluginId);
        if (!plugin) {
            qCWarning(dcThingManager()) << "Plugin for thing" << thingName << idString << "not found. This thing will not be functional until the plugin can be loaded.";
        }
        ThingClassId thingClassId = ThingClassId(settings.value("thingClassId").toString());
        // If ThingClassId isn't found in the config, retry with ThingClassId (nymea < 0.20)
        if (thingClassId.isNull()) {
            thingClassId = ThingClassId(settings.value("deviceClassId").toString());
        }
        ThingClass thingClass = findThingClass(thingClassId);
        if (!thingClass.isValid()) {
            // Try to load the device class from the cache
            QJsonObject pluginInfo = PluginInfoCache::loadPluginInfo(pluginId);
            if (!pluginInfo.empty()) {
                PluginMetadata pluginMetadata(pluginInfo, false, false);
                thingClass = pluginMetadata.thingClasses().findById(thingClassId);
                if (thingClass.isValid()) {
                    m_supportedThings.insert(thingClassId, thingClass);
                    if (!m_supportedVendors.contains(thingClass.vendorId())) {
                        Vendor vendor = pluginMetadata.vendors().findById(thingClass.vendorId());
                        m_supportedVendors.insert(vendor.id(), vendor);
                    }
                }
            }
        }
        if (!thingClass.isValid()) {
            qCWarning(dcThingManager()) << "Not loading thing" << thingName << idString << "because the thing class for this thing could not be found.";
            settings.endGroup(); // ThingId
            continue;
        }

        // Cross-check if this plugin still implements this thing class
        if (plugin && !plugin->supportedThings().contains(thingClass)) {
            qCWarning(dcThingManager()) << "Not loading thing" << thingName << idString << "because plugin" << plugin->pluginName() << "has removed support for it.";
            settings.endGroup(); // ThingId
            continue;
        }
        Thing *thing = new Thing(pluginId, thingClass, ThingId(idString), this);
        thing->m_autoCreated = settings.value("autoCreated").toBool();
        thing->setName(thingName);
        thing->setParentId(ThingId(settings.value("parentid", QUuid()).toString()));

        ParamList params;
        settings.beginGroup("Params");

        foreach (const ParamType &paramType, thingClass.paramTypes()) {
            QVariant value = paramType.defaultValue();
            if (settings.contains(paramType.id().toString())) {
                value = settings.value(paramType.id().toString());
            } else if (settings.childGroups().contains(paramType.id().toString())) {
                // 0.12.2 - 0.22 used to store in subgroups
                settings.beginGroup(paramType.id().toString());
                value = settings.value("value");
                settings.endGroup(); // paramTypeId
            }
            value.convert(paramType.type());
            Param param(paramType.id(), value);
            params.append(param);
        }

        // In order to give plugins a chance to migrate stuff stored in the params (to e.g. pluginStorage()) we'll load
        // params that might have disappeared from the ParamTypes but still have stuff stored in the config
        foreach (const QString paramTypeIdString, settings.childKeys()) {
            ParamTypeId paramTypeId(paramTypeIdString);
            if (!params.hasParam(paramTypeId)) {
                qCDebug(dcThingManager()) << "Loading legacy param" << paramTypeIdString << "for thing" << thing->name();
                Param param(paramTypeId, settings.value(paramTypeIdString));
                params.append(param);
            }
        }
        // 0.12.2 - 0.22 used to store in subgroups
        foreach (const QString &paramTypeIdString, settings.childGroups()) {
            settings.beginGroup(paramTypeIdString);
            ParamTypeId paramTypeId(paramTypeIdString);
            if (!params.hasParam(paramTypeId)) {
                qCDebug(dcThingManager()) << "Loading legacy param" << paramTypeIdString << "for thing" << thing->name();
                Param param(paramTypeId, settings.value("value"));
                params.append(param);
            }
            settings.endGroup(); // paramTypeId
        }

        settings.endGroup(); // Params

        thing->setParams(params);

        ParamList thingSettings;
        settings.beginGroup("Settings");

        foreach (const ParamType &paramType, thingClass.settingsTypes()) {
            QVariant value = paramType.defaultValue();
            if (settings.contains(paramType.id().toString())) {
                value = settings.value(paramType.id().toString());
            } else if (settings.childGroups().contains(paramType.id().toString())) {
                // 0.12.2 - 0.22 used to store in subgroups
                settings.beginGroup(paramType.id().toString());
                value = settings.value("value");
                settings.endGroup(); // paramTypeId
            }
            value.convert(paramType.type());
            Param param(paramType.id(), value);
            thingSettings.append(param);
        }

        settings.endGroup(); // Settings

        thing->setSettings(thingSettings);

        initThing(thing);

        settings.endGroup(); // ThingId

        // We always add the thing to the list in this case. If it's in the stored things
        // it means that it was working at some point so lets still add it as there might
        // be rules associated with this thing.
        registerThing(thing);

        emit thingAdded(thing);
    }
    settings.endGroup();

    if (needsMigration) {
        storeConfiguredThings();
        settings.remove("DeviceConfig");
    }


    QHash<ThingId, Thing*> setupList = m_configuredThings;
    while (!setupList.isEmpty()) {
        Thing *thing = nullptr;
        foreach (Thing *d, setupList) {
            if (d->parentId().isNull() || !setupList.contains(d->parentId())) {
                thing = d;
                setupList.take(d->id());
                break;
            }
        }
        Q_ASSERT(thing != nullptr);

        trySetupThing(thing);
    }

    loadIOConnections();
}

void ThingManagerImplementation::storeConfiguredThings()
{
    NymeaSettings settings(NymeaSettings::SettingsRoleThings);
    settings.beginGroup("ThingConfig");
    foreach (Thing *thing, m_configuredThings) {
        settings.beginGroup(thing->id().toString());
        // Note: clean thing settings before storing it for clean up
        settings.remove("");
        settings.setValue("autoCreated", thing->autoCreated());
        settings.setValue("thingName", thing->name());
        settings.setValue("thingClassId", thing->thingClassId().toString());
        settings.setValue("pluginid", thing->pluginId().toString());
        if (!thing->parentId().isNull())
            settings.setValue("parentid", thing->parentId().toString());

        settings.beginGroup("Params");
        foreach (const Param &param, thing->params()) {
            settings.setValue(param.paramTypeId().toString(), param.value());
        }
        settings.endGroup(); // Params

        settings.beginGroup("Settings");
        foreach (const Param &param, thing->settings()) {
            settings.setValue(param.paramTypeId().toString(), param.value());
        }
        settings.endGroup(); // Settings


        settings.endGroup(); // ThingId
    }
    settings.endGroup(); // ThingConfig
}

void ThingManagerImplementation::startMonitoringAutoThings()
{
    foreach (IntegrationPlugin *plugin, m_integrationPlugins) {
        plugin->startMonitoringAutoThings();
    }
}

void ThingManagerImplementation::onAutoThingsAppeared(const ThingDescriptors &thingDescriptors)
{
    foreach (const ThingDescriptor &thingDescriptor, thingDescriptors) {

        ThingClass thingClass = findThingClass(thingDescriptor.thingClassId());
        if (!thingClass.isValid()) {
            qCWarning(dcThingManager()) << "Ignoring appearing auto thing for an unknown ThingClass" << thingDescriptor.thingClassId();
            return;
        }

        IntegrationPlugin *plugin = m_integrationPlugins.value(thingClass.pluginId());
        if (!plugin) {
            return;
        }

        if (!thingDescriptor.parentId().isNull() && !m_configuredThings.contains(thingDescriptor.parentId())) {
            qCWarning(dcThingManager()) << "Invalid parent thing id. Not adding thing to the system.";
            continue;
        }

        Thing *thing = nullptr;

        // If the appeared auto thing holds a valid thing id, do a reconfiguration for this thing
        if (!thingDescriptor.thingId().isNull()) {
            thing = findConfiguredThing(thingDescriptor.thingId());
            if (!thing) {
                qCWarning(dcThingManager()) << "Could not find thing for auto thing descriptor" << thingDescriptor.thingId();
                continue;
            }
            qCDebug(dcThingManager()) << "Start reconfiguring auto thing" << thing;
            ParamList finalParams = buildParams(thingClass.paramTypes(), thingDescriptor.params());
            reconfigureThingInternal(thing, finalParams);
            continue;
        }

        thing = new Thing(plugin->pluginId(), thingClass, this);
        thing->m_autoCreated = true;
        thing->setName(thingDescriptor.title());
        thing->setParams(thingDescriptor.params());
        ParamList settings = buildParams(thingClass.settingsTypes(), ParamList());
        thing->setSettings(settings);
        thing->setParentId(thingDescriptor.parentId());

        initThing(thing);

        qCDebug(dcThingManager()) << "Setting up auto thing:" << thing->name() << thing->id().toString();

        ThingSetupInfo *info = setupThing(thing);
        connect(info, &ThingSetupInfo::finished, thing, [this, info](){

            if (info->status() != Thing::ThingErrorNoError) {
                qCWarning(dcThingManager) << "Thing setup failed. Not adding auto thing to system.";
                info->thing()->deleteLater();
                return;
            }

            info->thing()->setSetupStatus(Thing::ThingSetupStatusComplete, Thing::ThingErrorNoError);
            registerThing(info->thing());
            storeConfiguredThings();
            emit thingAdded(info->thing());
            postSetupThing(info->thing());
        });
    }
}

void ThingManagerImplementation::onAutoThingDisappeared(const ThingId &thingId)
{
    IntegrationPlugin *plugin = static_cast<IntegrationPlugin*>(sender());
    Thing *thing = m_configuredThings.value(thingId);

    if (!thing) {
        qCWarning(dcThingManager) << "Received an autoThingDisappeared signal but this thing is unknown:" << thingId;
        return;
    }

    ThingClass thingClass = m_supportedThings.value(thing->thingClassId());

    if (thingClass.pluginId() != plugin->pluginId()) {
        qCWarning(dcThingManager) << "Received a autoThingDisappeared signal but emitting plugin does not own the thing";
        return;
    }

    if (!thing->autoCreated()) {
        qCWarning(dcThingManager) << "Received an autoThingDisappeared signal but thing creationMethod is not CreateMothodAuto";
        return;
    }

    emit thingDisappeared(thingId);
}

void ThingManagerImplementation::onLoaded()
{
    qCDebug(dcThingManager()) << "Done loading plugins and things.";
    emit loaded();

    // schedule some housekeeping...
    QTimer::singleShot(0, this, SLOT(cleanupThingStateCache()));
}

void ThingManagerImplementation::cleanupThingStateCache()
{
    QDir dir(NymeaSettings::cachePath() + "/thingstates/");
    foreach (const QFileInfo &entry, dir.entryList()) {
        ThingId thingId(entry.baseName());
        if (!m_configuredThings.contains(thingId)) {
            qCDebug(dcThingManager()) << "Thing ID" << thingId << "not found in configured things. Cleaning up stale thing state cache.";
            QFile::remove(entry.absoluteFilePath());
        }
    }
}

void ThingManagerImplementation::onEventTriggered(Event event)
{
    // Doing some sanity checks here...
    Thing *thing = m_configuredThings.value(event.thingId());
    if (!thing) {
        qCWarning(dcThingManager()) << "Invalid thing id in emitted event. Not forwarding event. Thing setup not complete yet?";
        return;
    }
    EventType eventType = thing->thingClass().eventTypes().findById(event.eventTypeId());
    if (!eventType.isValid()) {
        qCWarning(dcThingManager()) << "The given thing does not have an event type of id " + event.eventTypeId().toString() + ". Not forwarding event.";
        return;
    }
    // configure logging
    if (thing->loggedEventTypeIds().contains(event.eventTypeId())) {
        event.setLogged(true);
    }

    // Forward the event
    emit eventTriggered(event);
}

void ThingManagerImplementation::slotThingStateValueChanged(const StateTypeId &stateTypeId, const QVariant &value, const QVariant &minValue, const QVariant &maxValue)
{
    Thing *thing = qobject_cast<Thing*>(sender());
    if (!thing || !m_configuredThings.contains(thing->id())) {
        qCWarning(dcThingManager()) << "Invalid thing id in state change. Not forwarding event. Thing setup not complete yet?";
        return;
    }
    if (thing->thingClass().getStateType(stateTypeId).cached()) {
        storeThingState(thing, stateTypeId);
    }

    emit thingStateChanged(thing, stateTypeId, value, minValue, maxValue);

    syncIOConnection(thing, stateTypeId);
}

void ThingManagerImplementation::syncIOConnection(Thing *thing, const StateTypeId &stateTypeId)
{

    foreach (const IOConnection &ioConnection, m_ioConnections) {
        // Check if this state is an input to an IO connection.
        if (ioConnection.inputThingId() == thing->id() && ioConnection.inputStateTypeId() == stateTypeId) {
            Thing *inputThing = thing;
            QVariant inputValue = inputThing->stateValue(stateTypeId);

            Thing *outputThing = m_configuredThings.value(ioConnection.outputThingId());
            if (!outputThing) {
                qCWarning(dcThingManager()) << "IO connection contains invalid output thing!";
                continue;
            }
            IntegrationPlugin *plugin = m_integrationPlugins.value(outputThing->pluginId());
            if (!plugin) {
                qCWarning(dcThingManager()) << "Plugin not found for IO connection's output action.";
                continue;
            }
            StateType inputStateType = inputThing->thingClass().getStateType(stateTypeId);

            StateType outputStateType = outputThing->thingClass().getStateType(ioConnection.outputStateTypeId());
            if (outputStateType.id().isNull()) {
                qCWarning(dcThingManager()) << "Could not find output state type for IO connection.";
                continue;
            }
            QVariant outputValue;
            if (outputStateType.ioType() == Types::IOTypeDigitalOutput) {
                // Digital IOs are mapped as-is
                outputValue = ioConnection.inverted() xor inputValue.toBool();

                // We're already in sync! Skipping action.
                if (outputThing->stateValue(outputStateType.id()) == outputValue) {
                    continue;
                }
            } else {
                // Analog IOs are mapped within the according min/max ranges
                outputValue = mapValue(inputValue, inputStateType, outputStateType, ioConnection.inverted());

                // We're already in sync (fuzzy, good enough)! Skipping action.
                if (qFuzzyCompare(1.0 + outputThing->stateValue(outputStateType.id()).toDouble(), 1.0 + outputValue.toDouble())) {
                    continue;
                }
            }
            Action outputAction(ActionTypeId(ioConnection.outputStateTypeId()), ioConnection.outputThingId());

            Param outputParam(ioConnection.outputStateTypeId(), outputValue);
            outputAction.setParams(ParamList() << outputParam);
            qCDebug(dcThingManager()) << "Executing IO connection action on" << outputThing->name() << outputParam;
            ThingActionInfo* info = executeAction(outputAction);
            connect(info, &ThingActionInfo::finished, this, [=](){
                if (info->status() != Thing::ThingErrorNoError) {
                    // An error happened... let's switch the input back to be in sync with the output
                    qCWarning(dcThingManager()) << "Error syncing IO connection state. Reverting input back to old value.";
                    if (inputStateType.ioType() == Types::IOTypeDigitalInput) {
                        inputThing->setStateValue(inputStateType.id(), outputThing->stateValue(outputStateType.id()));
                    } else {
                        inputThing->setStateValue(inputStateType.id(), mapValue(outputThing->stateValue(outputStateType.id()), outputStateType, inputStateType, ioConnection.inverted()));
                    }
                }
            });
        }

        // Now check if this is an output state type and - if possible - update the inputs for bidirectional connections
        if (ioConnection.outputThingId() == thing->id() && ioConnection.outputStateTypeId() == stateTypeId) {
            Thing *outputThing = thing;
            QVariant outputValue = outputThing->stateValue(stateTypeId);

            Thing *inputThing = m_configuredThings.value(ioConnection.inputThingId());
            if (!inputThing) {
                qCWarning(dcThingManager()) << "IO connection contains invalid input thing!";
                continue;
            }
            IntegrationPlugin *plugin = m_integrationPlugins.value(inputThing->pluginId());
            if (!plugin) {
                qCWarning(dcThingManager()) << "Plugin not found for IO connection's input action.";
                continue;
            }
            StateType outputStateType = outputThing->thingClass().getStateType(stateTypeId);

            StateType inputStateType = inputThing->thingClass().getStateType(ioConnection.inputStateTypeId());
            if (inputStateType.id().isNull()) {
                qCWarning(dcThingManager()) << "Could not find input state type for IO connection.";
                continue;
            }

            if (!inputStateType.writable()) {
                qCDebug(dcThingManager()) << "Input state is not writable. This connection is unidirectional.";
                continue;
            }

            QVariant inputValue;
            if (inputStateType.ioType() == Types::IOTypeDigitalInput) {
                // Digital IOs are mapped as-is
                inputValue = ioConnection.inverted() xor outputValue.toBool();

                // Prevent looping
                if (inputThing->stateValue(inputStateType.id()) == inputValue) {
                    continue;
                }
            } else {
                // Analog IOs are mapped within the according min/max ranges
                inputValue = mapValue(outputValue, outputStateType, inputStateType, ioConnection.inverted());

                // Prevent looping even if the above calculation has rounding errors... Just skip this action if we're close enough already
                if (qFuzzyCompare(1.0 + inputThing->stateValue(inputStateType.id()).toDouble(), 1.0 + inputValue.toDouble())) {
                    continue;
                }
            }
            Action inputAction(ActionTypeId(ioConnection.inputStateTypeId()), ioConnection.inputThingId());

            Param inputParam(ioConnection.inputStateTypeId(), inputValue);
            inputAction.setParams(ParamList() << inputParam);
            qCDebug(dcThingManager()) << "Executing reverse IO connection action on" << inputThing->name() << inputParam;
            executeAction(inputAction);
        }
    }
}

void ThingManagerImplementation::slotThingSettingChanged(const ParamTypeId &paramTypeId, const QVariant &value)
{
    Thing *thing = qobject_cast<Thing*>(sender());
    if (!thing) {
        return;
    }
    storeConfiguredThings();
    emit thingSettingChanged(thing->id(), paramTypeId, value);
}

void ThingManagerImplementation::slotThingNameChanged()
{
    Thing *thing = qobject_cast<Thing*>(sender());
    if (!thing) {
        return;
    }
    storeConfiguredThings();
    emit thingChanged(thing);
}

// Merges params from first and second. First has higher priority than second. If neither are given, the default is used - if any
ParamList ThingManagerImplementation::buildParams(const ParamTypes &types, const ParamList &first, const ParamList &second)
{
    ParamList finalParams;
    foreach (const ParamType &paramType, types) {
        QVariant value;
        if (first.hasParam(paramType.id())) {
            value = first.paramValue(paramType.id());
        } else if (second.hasParam(paramType.id())) {
            value = second.paramValue(paramType.id());
        } else if (paramType.defaultValue().isValid()){
            value = paramType.defaultValue();
        }
        if (!value.isNull()) {
            bool success = value.convert(paramType.type());
            if (!success) {
                qCWarning(dcThingManager()) << "Type mismatch in param" << paramType.name() << "for value" << value;
            }
            finalParams.append(Param(paramType.id(), value));
        }
    }
    return finalParams;
}

void ThingManagerImplementation::pairThingInternal(ThingPairingInfo *info)
{
    ThingClass thingClass = m_supportedThings.value(info->thingClassId());
    if (thingClass.id().isNull()) {
        qCWarning(dcThingManager) << "Cannot find a thing class with id" << info->thingClassId();
        info->finish(Thing::ThingErrorThingClassNotFound);
        return;
    }

    if (thingClass.setupMethod() == ThingClass::SetupMethodJustAdd) {
        qCWarning(dcThingManager) << "Cannot setup this thing this way. No need to pair this thing.";
        info->finish(Thing::ThingErrorSetupMethodNotSupported);
        return;
    }

    IntegrationPlugin *plugin = m_integrationPlugins.value(thingClass.pluginId());
    if (!plugin) {
        qCWarning(dcThingManager) << "Cannot pair thing class" << thingClass.name() << "because no plugin for it is loaded.";
        info->finish(Thing::ThingErrorPluginNotFound);
        return;
    }

    plugin->startPairing(info);

    connect(info, &ThingPairingInfo::finished, this, [this, info, thingClass](){
        if (info->status() != Thing::ThingErrorNoError) {
            qCWarning(dcThingManager()) << "PairThing failed for" << thingClass << info->status() << info->displayMessage();
            return;
        }

        qCDebug(dcThingManager()) << "Pairing started for" << thingClass << "using transaction id" << info->transactionId();
        PairingContext context;
        context.thingId = info->thingId();
        context.thingClassId = info->thingClassId();
        context.thingName = info->thingName();
        context.params = info->params();
        context.parentId = info->parentId();
        m_pendingPairings.insert(info->transactionId(), context);

        // TODO: Add a timer to clean up stuff if confirmPairing is never called.
    });
}

ThingSetupInfo* ThingManagerImplementation::setupThing(Thing *thing)
{
    ThingClass thingClass = findThingClass(thing->thingClassId());
    IntegrationPlugin *plugin = m_integrationPlugins.value(thingClass.pluginId());

    ThingSetupInfo *info = new ThingSetupInfo(thing, this, 30000);

    if (!plugin) {
        qCWarning(dcThingManager) << "Can't find a plugin for this thing" << thing;
        info->finish(Thing::ThingErrorPluginNotFound, tr("The plugin for this thing is not loaded."));
        return info;
    }

    plugin->setupThing(info);

    m_pendingSetups.insert(thing->id(), info);
    connect(info, &ThingSetupInfo::destroyed, thing, [=](){
        m_pendingSetups.remove(thing->id());
    });

    return info;
}

void ThingManagerImplementation::initThing(Thing *thing)
{
    ThingClass thingClass = findThingClass(thing->thingClassId());

    QList<State> states;
    foreach (const StateType &stateType, thingClass.stateTypes()) {
        State state(stateType.id(), thing->id());
        states.append(state);
    }
    thing->setStates(states);
    loadThingStates(thing);

    QList<EventTypeId> loggedEventTypeIds;
    foreach (const EventType &eventType, thingClass.eventTypes()) {
        if (eventType.suggestLogging()) {
            loggedEventTypeIds.append(eventType.id());
        }
    }
    thing->setLoggedEventTypeIds(loggedEventTypeIds);
    QList<StateTypeId> loggedStateTypeIds;
    foreach (const StateType &stateType, thingClass.stateTypes()) {
        if (stateType.suggestLogging()) {
            loggedStateTypeIds.append(stateType.id());
        }
    }
    thing->setLoggedStateTypeIds(loggedStateTypeIds);
}

void ThingManagerImplementation::postSetupThing(Thing *thing)
{
    ThingClass thingClass = findThingClass(thing->thingClassId());
    IntegrationPlugin *plugin = m_integrationPlugins.value(thingClass.pluginId());

    plugin->postSetupThing(thing);
}

QString ThingManagerImplementation::statesCacheFile(const ThingId &thingId)
{
    return NymeaSettings::cachePath() + "/thingstates/" + thingId.toString().remove(QRegExp("[{}]")) + ".cache";
}

void ThingManagerImplementation::loadThingStates(Thing *thing)
{
    QSettings *settings = nullptr;
    if (QFile::exists(statesCacheFile(thing->id()))) {
        settings = new QSettings(statesCacheFile(thing->id()), QSettings::IniFormat);
    } else {
        // try legacy (<= 0.30 cache)
        settings = new QSettings(NymeaSettings::settingsPath() + "/thingstates.conf", QSettings::IniFormat);
        settings->beginGroup(thing->id().toString());
    }
    ThingClass thingClass = m_supportedThings.value(thing->thingClassId());
    foreach (const StateType &stateType, thingClass.stateTypes()) {
        QVariant value = stateType.defaultValue();
        QVariant minValue = stateType.minValue();
        QVariant maxValue = stateType.maxValue();

        if (stateType.cached()) {
            if (settings->childGroups().contains(stateType.id().toString())) {
                settings->beginGroup(stateType.id().toString());
                value = settings->value("value");
                minValue = settings->value("minValue");
                maxValue = settings->value("maxValue");
                settings->endGroup();
            } else if (settings->contains(stateType.id().toString())) {
                // Migration from < 0.30
                value = settings->value(stateType.id().toString());
            }
            value.convert(stateType.type());
            minValue.convert(stateType.type());
            maxValue.convert(stateType.type());
        }

        thing->setStateValue(stateType.id(), value);
        thing->setStateMinMaxValues(stateType.id(), minValue, maxValue);
        thing->setStateValueFilter(stateType.id(), stateType.filter());
    }
    delete settings;
}

void ThingManagerImplementation::storeIOConnections()
{
    NymeaSettings connectionSettings(NymeaSettings::SettingsRoleIOConnections);
    connectionSettings.beginGroup("IOConnections");
    foreach (const IOConnection &ioConnection, m_ioConnections) {
        connectionSettings.beginGroup(ioConnection.id().toString());

        connectionSettings.setValue("inputThingId", ioConnection.inputThingId().toString());
        connectionSettings.setValue("inputStateTypeId", ioConnection.inputStateTypeId().toString());
        connectionSettings.setValue("outputThingId", ioConnection.outputThingId().toString());
        connectionSettings.setValue("outputStateTypeId", ioConnection.outputStateTypeId().toString());
        connectionSettings.setValue("inverted", ioConnection.inverted());

        connectionSettings.endGroup();
    }
    connectionSettings.endGroup();
}

void ThingManagerImplementation::loadIOConnections()
{
    NymeaSettings connectionSettings(NymeaSettings::SettingsRoleIOConnections);
    connectionSettings.beginGroup("IOConnections");
    foreach (const QString &idString, connectionSettings.childGroups()) {
        connectionSettings.beginGroup(idString);
        IOConnectionId id(idString);
        ThingId inputThingId = connectionSettings.value("inputThingId").toUuid();
        StateTypeId inputStateTypeId = connectionSettings.value("inputStateTypeId").toUuid();
        ThingId outputThingId = connectionSettings.value("outputThingId").toUuid();
        StateTypeId outputStateTypeId = connectionSettings.value("outputStateTypeId").toUuid();
        bool inverted = connectionSettings.value("inverted").toBool();
        IOConnection ioConnection(id, inputThingId, inputStateTypeId, outputThingId, outputStateTypeId, inverted);
        m_ioConnections.insert(id, ioConnection);
        connectionSettings.endGroup();

        Thing *inputThing = m_configuredThings.value(inputThingId);
        if (!inputThing) {
            continue;
        }
        syncIOConnection(inputThing, inputStateTypeId);
    }
    connectionSettings.endGroup();
}

QVariant ThingManagerImplementation::mapValue(const QVariant &value, const StateType &fromStateType, const StateType &toStateType, bool inverted) const
{
    double fromMin = fromStateType.minValue().toDouble();
    double fromMax = fromStateType.maxValue().toDouble();
    double toMin = toStateType.minValue().toDouble();
    double toMax = toStateType.maxValue().toDouble();
    double fromValue = value.toDouble();
    double fromPercent = (fromValue - fromMin) / (fromMax - fromMin);
    fromPercent = inverted ? 1 - fromPercent : fromPercent;
    double toValue = toMin + (toMax - toMin) * fromPercent;
    return toValue;
}

void ThingManagerImplementation::trySetupThing(Thing *thing)
{
    thing->setSetupStatus(Thing::ThingSetupStatusInProgress, Thing::ThingErrorNoError);
    ThingSetupInfo *info = setupThing(thing);
    // Set receiving object to "thing" because at startup we load it in any case, knowing that it worked at
    // some point. However, it'll be marked as non-working until the setup succeeds so the user might delete
    // it in the meantime... In that case we don't want to call postsetup on it.
    connect(info, &ThingSetupInfo::finished, thing, [this, info, thing](){

        if (info->status() != Thing::ThingErrorNoError) {
            qCWarning(dcThingManager()) << "Error setting up thing" << info->thing()->name() << info->thing()->id().toString() << info->status() << info->displayMessage();
            info->thing()->setSetupStatus(Thing::ThingSetupStatusFailed, info->status(), info->displayMessage());
            emit thingChanged(info->thing());

            // We know this used to work at some point... try again in a bit unless we don't have a plugin for it...
            if (info->status() != Thing::ThingErrorPluginNotFound) {
                QTimer::singleShot(10000, thing, [this, thing]() {
                    trySetupThing(thing);
                });
            }

            return;
        }

        qCDebug(dcThingManager()) << "Setup complete for thing" << info->thing();
        info->thing()->setSetupStatus(Thing::ThingSetupStatusComplete, Thing::ThingErrorNoError);
        emit thingChanged(info->thing());
        postSetupThing(info->thing());
    });
}

void ThingManagerImplementation::registerThing(Thing *thing)
{
    m_configuredThings.insert(thing->id(), thing);
    connect(thing, &Thing::eventTriggered, this, &ThingManagerImplementation::onEventTriggered);
    connect(thing, &Thing::stateValueChanged, this, &ThingManagerImplementation::slotThingStateValueChanged);
    connect(thing, &Thing::settingChanged, this, &ThingManagerImplementation::slotThingSettingChanged);
    connect(thing, &Thing::nameChanged, this, &ThingManagerImplementation::slotThingNameChanged);
}

IntegrationPlugin *ThingManagerImplementation::createCppIntegrationPlugin(const QString &absoluteFilePath)
{
    // Check plugin API version compatibility
    QLibrary lib(absoluteFilePath);
    if (!lib.load()) {
        qCWarning(dcThingManager()).nospace() << "Error loading plugin " << absoluteFilePath << ": " << lib.errorString();
        return nullptr;
    }

    QFunctionPointer versionFunc = lib.resolve("libnymea_api_version");
    if (!versionFunc) {
        qCWarning(dcThingManager()).nospace() << "Unable to resolve version in plugin " << absoluteFilePath << ". Not loading plugin.";
        lib.unload();
        return nullptr;
    }

    QString version = reinterpret_cast<QString(*)()>(versionFunc)();
    lib.unload();
    QStringList parts = version.split('.');
    QStringList coreParts = QString(LIBNYMEA_API_VERSION).split('.');
    if (parts.length() != 3 || parts.at(0).toInt() != coreParts.at(0).toInt() || parts.at(1).toInt() > coreParts.at(1).toInt()) {
        qCWarning(dcThingManager()).nospace() << "Libnymea API mismatch for " << absoluteFilePath << ". Core API: " << LIBNYMEA_API_VERSION << ", Plugin API: " << version;
        return nullptr;
    }

    // Version is ok. Now load the plugin
    QPluginLoader loader;
    loader.setFileName(absoluteFilePath);
    loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);

    qCDebug(dcThingManager()) << "Loading plugin from:" << absoluteFilePath;
    if (!loader.load()) {
        qCWarning(dcThingManager) << "Could not load plugin data of" << absoluteFilePath << "\n" << loader.errorString();
        return nullptr;
    }

    QJsonObject pluginInfo = loader.metaData().value("MetaData").toObject();
    PluginMetadata metaData(pluginInfo, false, false);
    if (!metaData.isValid()) {
        foreach (const QString &error, metaData.validationErrors()) {
            qCWarning(dcThingManager()) << error;
        }
        loader.unload();
        return nullptr;
    }

    QObject *p = loader.instance();
    if (!p) {
        qCWarning(dcThingManager()) << "Error loading plugin:" << loader.errorString();
        return nullptr;
    }
    IntegrationPlugin *pluginIface = qobject_cast<IntegrationPlugin *>(p);
    if (!pluginIface) {
        qCWarning(dcThingManager) << "Could not get plugin instance of" << absoluteFilePath;
        return nullptr;
    }

    pluginIface->setMetaData(metaData);

    return pluginIface;
}

void ThingManagerImplementation::storeThingStates(Thing *thing)
{
    ThingClass thingClass = m_supportedThings.value(thing->thingClassId());
    foreach (const StateType &stateType, thingClass.stateTypes()) {
        if (stateType.cached()) {
            storeThingState(thing,  stateType.id());
        }
    }
}

void ThingManagerImplementation::storeThingState(Thing *thing, const StateTypeId &stateTypeId)
{
    QSettings settings(statesCacheFile(thing->id()), QSettings::IniFormat);
    qCDebug(dcThingManager()) << "Caching state:" << thing->name() << thing->thingClass().stateTypes().findById(stateTypeId).name();
    settings.beginGroup(stateTypeId.toString());
    settings.setValue("value", thing->stateValue(stateTypeId));
    settings.setValue("minValue", thing->state(stateTypeId).minValue());
    settings.setValue("maxValue", thing->state(stateTypeId).maxValue());
    settings.endGroup();
}

