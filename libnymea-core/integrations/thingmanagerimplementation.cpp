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

ThingManagerImplementation::ThingManagerImplementation(HardwareManager *hardwareManager, const QLocale &locale, QObject *parent) :
    ThingManager(parent),
    m_hardwareManager(hardwareManager),
    m_locale(locale),
    m_translator(new Translator(this))
{
    qRegisterMetaType<ThingClassId>();
    qRegisterMetaType<ThingDescriptor>();

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

    // Give hardware a chance to start up before loading plugins etc.
    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "loadConfiguredThings", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "startMonitoringAutoThings", Qt::QueuedConnection);

    // Make sure this is always emitted after plugins and things are loaded
    QMetaObject::invokeMethod(this, "onLoaded", Qt::QueuedConnection);
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
    foreach (const QString &path, pluginSearchDirs()) {
        QDir dir(path);
        foreach (const QString &entry, dir.entryList()) {
            QFileInfo fi;
            if (entry.startsWith("libnymea_integrationplugin") && entry.endsWith(".so")) {
                fi.setFile(path + "/" + entry);
            } else {
                fi.setFile(path + "/" + entry + "/libnymea_integrationplugin" + entry + ".so");
            }
            if (!fi.exists()) {
                continue;
            }
            QPluginLoader loader(fi.absoluteFilePath());
            pluginList.append(loader.metaData().value("MetaData").toObject());
        }
    }
    return pluginList;
}

void ThingManagerImplementation::registerStaticPlugin(IntegrationPlugin *plugin, const PluginMetadata &metaData)
{
    if (!metaData.isValid()) {
        qCWarning(dcThingManager()) << "Plugin metadata not valid. Not loading static plugin:" << plugin->pluginName();
        return;
    }
    loadPlugin(plugin, metaData);
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

    ParamList params = buildParams(plugin->configurationDescription(), pluginConfig);
    Thing::ThingError verify = ThingUtils::verifyParams(plugin->configurationDescription(), params);
    if (verify != Thing::ThingErrorNoError)
        return verify;

    Thing::ThingError result = plugin->setConfiguration(params);
    if (result != Thing::ThingErrorNoError)
        return result;

    NymeaSettings settings(NymeaSettings::SettingsRolePlugins);
    settings.beginGroup("PluginConfig");
    settings.beginGroup(plugin->pluginId().toString());

    foreach (const Param &param, pluginConfig) {
        settings.beginGroup(param.paramTypeId().toString());
        settings.setValue("type", static_cast<int>(param.value().type()));
        settings.setValue("value", param.value());
        settings.endGroup();
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

    ParamList effectiveParams = buildParams(thingClass.discoveryParamTypes(), params);
    Thing::ThingError result = ThingUtils::verifyParams(thingClass.discoveryParamTypes(), effectiveParams);
    if (result != Thing::ThingErrorNoError) {
        qCWarning(dcThingManager) << "Thing discovery failed. Parameter verification failed.";
        ThingDiscoveryInfo *discoveryInfo = new ThingDiscoveryInfo(thingClassId, params, this);
        discoveryInfo->finish(result);
        return discoveryInfo;
    }

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

    ParamList finalParams = buildParams(thing->thingClass().paramTypes(), params);
    Thing::ThingError result = ThingUtils::verifyParams(thing->thingClass().paramTypes(), finalParams);
    if (result != Thing::ThingErrorNoError) {
        qCWarning(dcThingManager()) << "Cannot reconfigure thing. Params failed validation.";
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        info->finish(result);
        return info;
    }

    // first remove the thing in the plugin
    plugin->thingRemoved(thing);

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
    // build a list of settings using: a) provided new settings b) previous settings and c) default values
    ParamList effectiveSettings = buildParams(thing->thingClass().settingsTypes(), settings, thing->settings());
    Thing::ThingError status = ThingUtils::verifyParams(thing->thingClass().settingsTypes(), effectiveSettings);
    if (status != Thing::ThingErrorNoError) {
        qCWarning(dcThingManager()) << "Error setting thing settings for" << thing->name() << thing->id().toString();
        return status;
    }
    thing->setSettings(effectiveSettings);
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
                m_configuredThings.insert(info->thing()->id(), info->thing());
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
    ParamList effectiveParams = buildParams(thingClass.paramTypes(), params);
    Thing::ThingError paramsResult = ThingUtils::verifyParams(thingClass.paramTypes(), effectiveParams);
    if (paramsResult != Thing::ThingErrorNoError) {
        qCWarning(dcThingManager()) << "Cannot add thing. Parameter verification failed.";
        ThingSetupInfo *info = new ThingSetupInfo(nullptr, this);
        info->finish(paramsResult);
        return info;
    }

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
    qCDebug(dcThingManager()) << "Adding thing settings" << settings << thingId;
    thing->setSettings(settings);

    ThingSetupInfo *info = setupThing(thing);
    connect(info, &ThingSetupInfo::finished, this, [this, info](){
        if (info->status() != Thing::ThingErrorNoError) {
            qCWarning(dcThingManager) << "Thing setup failed. Not adding thing to system.";
            info->thing()->deleteLater();
            return;
        }

        info->thing()->setSetupStatus(Thing::ThingSetupStatusComplete, Thing::ThingErrorNoError);

        qCDebug(dcThingManager) << "Thing setup complete.";
        m_configuredThings.insert(info->thing()->id(), info->thing());
        storeConfiguredThings();
        postSetupThing(info->thing());

        emit thingAdded(info->thing());
    });

    return info;
}

Thing::ThingError ThingManagerImplementation::removeConfiguredThing(const ThingId &thingId)
{
    Thing *thing = m_configuredThings.take(thingId);
    if (!thing) {
        return Thing::ThingErrorThingNotFound;
    }
    IntegrationPlugin *plugin = m_integrationPlugins.value(thing->pluginId());
    if (!plugin) {
        qCWarning(dcThingManager()).nospace() << "Plugin not loaded for thing " << thing->name() << ". Not calling thingRemoved on plugin.";
    } else {
        plugin->thingRemoved(thing);
    }

    thing->deleteLater();

    NymeaSettings settings(NymeaSettings::SettingsRoleThings);
    settings.beginGroup("ThingConfig");
    settings.beginGroup(thingId.toString());
    settings.remove("");
    settings.endGroup();

    NymeaSettings stateCache(NymeaSettings::SettingsRoleThingStates);
    stateCache.remove(thingId.toString());

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

ThingClass ThingManagerImplementation::translateThingClass(const ThingClass &thingClass, const QLocale &locale)
{
    ThingClass translatedThingClass = thingClass;
    translatedThingClass.setDisplayName(translate(thingClass.pluginId(), thingClass.displayName(), locale));

    ParamTypes translatedSettingsTypes;
    foreach (const ParamType &paramType, thingClass.settingsTypes()) {
        translatedSettingsTypes.append(translateParamType(thingClass.pluginId(), paramType, locale));
    }
    translatedThingClass.setSettingsTypes(translatedSettingsTypes);
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

    ParamList finalParams = buildParams(actionType.paramTypes(), action.params());
    Thing::ThingError paramCheck = ThingUtils::verifyParams(actionType.paramTypes(), finalParams);
    if (paramCheck != Thing::ThingErrorNoError) {
        qCWarning(dcThingManager()) << "Cannot execute action. Parameter verification failed.";
        ThingActionInfo *info = new ThingActionInfo(thing, action, this);
        info->finish(paramCheck);
        return info;
    }
    finalAction.setParams(finalParams);

    ThingActionInfo *info = new ThingActionInfo(thing, finalAction, this, 30000);

    IntegrationPlugin *plugin = m_integrationPlugins.value(thing->pluginId());
    if (!plugin) {
        qCWarning(dcThingManager()) << "Cannot execute action. Plugin not found for device" << thing->name();
        info->finish(Thing::ThingErrorPluginNotFound);
        return info;
    }

    plugin->executeAction(info);

    return info;
}

void ThingManagerImplementation::loadPlugins()
{
    foreach (const QString &path, pluginSearchDirs()) {
        QDir dir(path);
        qCDebug(dcThingManager) << "Loading plugins from:" << dir.absolutePath();
        foreach (const QString &entry, dir.entryList()) {
            QFileInfo fi;
            if (entry.startsWith("libnymea_integrationplugin") && entry.endsWith(".so")) {
                fi.setFile(path + "/" + entry);
            } else {
                fi.setFile(path + "/" + entry + "/libnymea_integrationplugin" + entry + ".so");
            }

            if (!fi.exists())
                continue;

            // Check plugin API version compatibility
            QLibrary lib(fi.absoluteFilePath());
            QFunctionPointer versionFunc = lib.resolve("libnymea_api_version");
            if (!versionFunc) {
                qCWarning(dcThingManager()).nospace() << "Unable to resolve version in plugin " << entry << ". Not loading plugin.";
                lib.unload();
                continue;
            }

            QString version = reinterpret_cast<QString(*)()>(versionFunc)();
            lib.unload();
            QStringList parts = version.split('.');
            QStringList coreParts = QString(LIBNYMEA_API_VERSION).split('.');
            if (parts.length() != 3 || parts.at(0).toInt() != coreParts.at(0).toInt() || parts.at(1).toInt() > coreParts.at(1).toInt()) {
                qCWarning(dcThingManager()).nospace() << "Libnymea API mismatch for " << entry << ". Core API: " << LIBNYMEA_API_VERSION << ", Plugin API: " << version;
                continue;
            }

            // Version is ok. Now load the plugin
            QPluginLoader loader;
            loader.setFileName(fi.absoluteFilePath());
            loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);

            qCDebug(dcThingManager()) << "Loading plugin from:" << fi.absoluteFilePath();
            if (!loader.load()) {
                qCWarning(dcThingManager) << "Could not load plugin data of" << entry << "\n" << loader.errorString();
                continue;
            }

            QJsonObject pluginInfo = loader.metaData().value("MetaData").toObject();
            PluginMetadata metaData(pluginInfo);
            if (!metaData.isValid()) {
                foreach (const QString &error, metaData.validationErrors()) {
                    qCWarning(dcThingManager()) << error;
                }
                loader.unload();
                continue;
            }

            IntegrationPlugin *pluginIface = qobject_cast<IntegrationPlugin *>(loader.instance());
            if (!pluginIface) {
                qCWarning(dcThingManager) << "Could not get plugin instance of" << entry;
                loader.unload();
                continue;
            }
            if (m_integrationPlugins.contains(pluginIface->pluginId())) {
                qCWarning(dcThingManager()) << "A plugin with this ID is already loaded. Not loading" << entry;
                continue;
            }
            loadPlugin(pluginIface, metaData);
            PluginInfoCache::cachePluginInfo(pluginInfo);
        }
    }

#if QT_VERSION >= QT_VERSION_CHECK(5,12,0)
    foreach (const QString &path, pluginSearchDirs()) {
        QDir dir(path);
        qCDebug(dcThingManager) << "Loading JS plugins from:" << dir.absolutePath();
        foreach (const QString &entry, dir.entryList()) {
            QFileInfo jsFi;
            QFileInfo jsonFi;

            if (entry.endsWith(".js")) {
                jsFi.setFile(path + "/" + entry);
            } else {
                jsFi.setFile(path + "/" + entry + "/" + entry + ".js");
            }

            if (!jsFi.exists()) {
                continue;
            }

            ScriptIntegrationPlugin *plugin = new ScriptIntegrationPlugin(this);
            bool ret = plugin->loadScript(jsFi.absoluteFilePath());
            if (!ret) {
                delete plugin;
                qCWarning(dcThingManager()) << "JS plugin failed to load";
                continue;
            }
            PluginMetadata metaData(plugin->metaData());
            if (!metaData.isValid()) {
                qCWarning(dcThingManager()) << "Not loading JS plugin. Invalid metadata.";
                foreach (const QString &error, metaData.validationErrors()) {
                    qCWarning(dcThingManager()) << error;
                }
            }
            loadPlugin(plugin, metaData);
        }
    }
#endif
}

void ThingManagerImplementation::loadPlugin(IntegrationPlugin *pluginIface, const PluginMetadata &metaData)
{
    pluginIface->setParent(this);
    pluginIface->initPlugin(metaData, this, m_hardwareManager);

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
    if (settings.childGroups().contains(pluginIface->pluginId().toString())) {
        settings.beginGroup(pluginIface->pluginId().toString());

        if (!settings.childGroups().isEmpty()) {
            // Note: since nymea 0.12.2 the param type gets saved too for better data converting
            foreach (const QString &paramTypeIdString, settings.childGroups()) {
                ParamTypeId paramTypeId(paramTypeIdString);
                ParamType paramType = pluginIface->configurationDescription().findById(paramTypeId);
                if (!paramType.isValid()) {
                    qCWarning(dcThingManager()) << "Not loading Param for plugin" << pluginIface->pluginName() << "because the ParamType for the saved Param" << ParamTypeId(paramTypeIdString).toString() << "could not be found.";
                    continue;
                }

                QVariant paramValue;
                settings.beginGroup(paramTypeIdString);
                paramValue = settings.value("value", paramType.defaultValue());
                paramValue.convert(settings.value("type").toInt());
                params.append(Param(paramTypeId, paramValue));
                settings.endGroup();
            }
        } else {
            // Note: < nymea 0.12.2
            foreach (const QString &paramTypeIdString, settings.allKeys()) {
                params.append(Param(ParamTypeId(paramTypeIdString), settings.value(paramTypeIdString)));
            }
        }

        settings.endGroup();
    } else if (!pluginIface->configurationDescription().isEmpty()){
        // plugin requires config but none stored. Init with defaults
        foreach (const ParamType &paramType, pluginIface->configurationDescription()) {
            Param param(paramType.id(), paramType.defaultValue());
            params.append(param);
        }
    }
    settings.endGroup();

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
                PluginMetadata pluginMetadata(pluginInfo);
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
        if (!settings.childGroups().isEmpty()) {
            foreach (const QString &paramTypeIdString, settings.childGroups()) {
                ParamTypeId paramTypeId(paramTypeIdString);
                ParamType paramType = thingClass.paramTypes().findById(paramTypeId);
                QVariant defaultValue;
                if (!paramType.isValid()) {
                    // NOTE: We're not skipping unknown parameters to give plugins a chance to still access old values if they change their config and migrate things over.
                    qCWarning(dcThingManager()) << "Unknown param" << paramTypeIdString << "for" << thing << ". ParamType could not be found in device class.";
                }

                // Note: since nymea 0.12.2
                QVariant paramValue;
                settings.beginGroup(paramTypeIdString);
                paramValue = settings.value("value", paramType.defaultValue());
                paramValue.convert(settings.value("type").toInt());
                params.append(Param(paramTypeId, paramValue));
                settings.endGroup(); // ParamId
            }
        } else {
            foreach (const QString &paramTypeIdString, settings.allKeys()) {
                params.append(Param(ParamTypeId(paramTypeIdString), settings.value(paramTypeIdString)));
            }
        }
        // Make sure all params are around. if they aren't initialize with default values
        foreach (const ParamType &paramType, thingClass.paramTypes()) {
            if (!params.hasParam(paramType.id())) {
                params.append(Param(paramType.id(), paramType.defaultValue()));
            }
        }
        thing->setParams(params);
        settings.endGroup(); // Params

        ParamList thingSettings;
        settings.beginGroup("Settings");
        if (!settings.childGroups().isEmpty()) {
            foreach (const QString &paramTypeIdString, settings.childGroups()) {
                ParamTypeId paramTypeId(paramTypeIdString);
                ParamType paramType = thingClass.settingsTypes().findById(paramTypeId);
                if (!paramType.isValid()) {
                    qCWarning(dcThingManager()) << "Not loading Setting for thing" << thing << "because the ParamType for the saved Setting" << ParamTypeId(paramTypeIdString).toString() << "could not be found.";
                    continue;
                }

                // Note: since nymea 0.12.2
                QVariant paramValue;
                settings.beginGroup(paramTypeIdString);
                paramValue = settings.value("value", paramType.defaultValue());
                paramValue.convert(settings.value("type").toInt());
                thingSettings.append(Param(paramTypeId, paramValue));
                settings.endGroup(); // ParamId
            }
        } else {
            foreach (const QString &paramTypeIdString, settings.allKeys()) {
                params.append(Param(ParamTypeId(paramTypeIdString), settings.value(paramTypeIdString)));
            }
        }

        // Fill in any missing params with defaults
        thingSettings = buildParams(thingClass.settingsTypes(), thingSettings);

        thing->setSettings(thingSettings);

        settings.endGroup(); // Settings
        settings.endGroup(); // ThingId

        // We always add the thing to the list in this case. If it's in the stored things
        // it means that it was working at some point so lets still add it as there might
        // be rules associated with this thing.
        m_configuredThings.insert(thing->id(), thing);

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

        thing->setSetupStatus(Thing::ThingSetupStatusInProgress, Thing::ThingErrorNoError);
        ThingSetupInfo *info = setupThing(thing);
        // Set receiving object to "thing" because at startup we load it in any case, knowing that it worked at
        // some point. However, it'll be marked as non-working until the setup succeeds so the user might delete
        // it in the meantime... In that case we don't want to call postsetup on it.
        connect(info, &ThingSetupInfo::finished, thing, [this, info](){

            if (info->status() != Thing::ThingErrorNoError) {
                qCWarning(dcThingManager()) << "Error setting up thing" << info->thing()->name() << info->thing()->id().toString() << info->status() << info->displayMessage();
                info->thing()->setSetupStatus(Thing::ThingSetupStatusFailed, info->status(), info->displayMessage());
                emit thingChanged(info->thing());
                return;
            }

            qCDebug(dcThingManager()) << "Setup complete for thing" << info->thing();
            info->thing()->setSetupStatus(Thing::ThingSetupStatusComplete, Thing::ThingErrorNoError);
            emit thingChanged(info->thing());
            postSetupThing(info->thing());
        });
    }
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
            settings.beginGroup(param.paramTypeId().toString());
            settings.setValue("type", static_cast<int>(param.value().type()));
            settings.setValue("value", param.value());
            settings.endGroup(); // ParamTypeId
        }
        settings.endGroup(); // Params

        settings.beginGroup("Settings");
        foreach (const Param &param, thing->settings()) {
            settings.beginGroup(param.paramTypeId().toString());
            settings.setValue("type", static_cast<int>(param.value().type()));
            settings.setValue("value", param.value());
            settings.endGroup(); // ParamTypeId
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

        qCDebug(dcThingManager()) << "Setting up auto thing:" << thing->name() << thing->id().toString();

        ThingSetupInfo *info = setupThing(thing);
        connect(info, &ThingSetupInfo::finished, thing, [this, info](){

            if (info->status() != Thing::ThingErrorNoError) {
                qCWarning(dcThingManager) << "Thing setup failed. Not adding auto thing to system.";
                info->thing()->deleteLater();
                return;
            }

            info->thing()->setSetupStatus(Thing::ThingSetupStatusComplete, Thing::ThingErrorNoError);
            m_configuredThings.insert(info->thing()->id(), info->thing());
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
        qWarning(dcThingManager) << "Received an autoThingDisappeared signal but this thing is unknown:" << thingId;
        return;
    }

    ThingClass thingClass = m_supportedThings.value(thing->thingClassId());

    if (thingClass.pluginId() != plugin->pluginId()) {
        qWarning(dcThingManager) << "Received a autoThingDisappeared signal but emitting plugin does not own the thing";
        return;
    }

    if (!thing->autoCreated()) {
        qWarning(dcThingManager) << "Received an autoThingDisappeared signal but thing creationMethod is not CreateMothodAuto";
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
    NymeaSettings settings(NymeaSettings::SettingsRoleThingStates);
    foreach (const QString &entry, settings.childGroups()) {
        ThingId thingId(entry);
        if (!m_configuredThings.contains(thingId)) {
            qCDebug(dcThingManager()) << "Thing ID" << thingId << "not found in configured things. Cleaning up stale thing state cache.";
            settings.remove(entry);
        }
    }
}

void ThingManagerImplementation::onEventTriggered(const Event &event)
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
    // All good, forward the event
    emit eventTriggered(event);
}

void ThingManagerImplementation::slotThingStateValueChanged(const StateTypeId &stateTypeId, const QVariant &value)
{
    Thing *thing = qobject_cast<Thing*>(sender());
    if (!thing || !m_configuredThings.contains(thing->id())) {
        qCWarning(dcThingManager()) << "Invalid thing id in state change. Not forwarding event. Thing setup not complete yet?";
        return;
    }
    emit thingStateChanged(thing, stateTypeId, value);

    Param valueParam(ParamTypeId(stateTypeId.toString()), value);
    Event event(EventTypeId(stateTypeId.toString()), thing->id(), ParamList() << valueParam, true);
    emit eventTriggered(event);
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

ParamList ThingManagerImplementation::buildParams(const ParamTypes &types, const ParamList &first, const ParamList &second)
{
    // Merge params from discovered descriptor and additional overrides provided on API call. User provided params have higher priority than discovery params.
    ParamList finalParams;
    foreach (const ParamType &paramType, types) {
        if (first.hasParam(paramType.id())) {
            finalParams.append(Param(paramType.id(), first.paramValue(paramType.id())));
        } else if (second.hasParam(paramType.id())) {
            finalParams.append(Param(paramType.id(), second.paramValue(paramType.id())));
        } else if (paramType.defaultValue().isValid()){
            finalParams.append(Param(paramType.id(), paramType.defaultValue()));
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

    if (!plugin) {
        qCWarning(dcThingManager) << "Can't find a plugin for this thing" << thing;
        ThingSetupInfo *info = new ThingSetupInfo(thing, this);
        info->finish(Thing::ThingErrorPluginNotFound, tr("The plugin for this thing is not loaded."));
        return info;
    }

    QList<State> states;
    foreach (const StateType &stateType, thingClass.stateTypes()) {
        State state(stateType.id(), thing->id());
        states.append(state);
    }
    thing->setStates(states);
    loadThingStates(thing);

    connect(thing, &Thing::stateValueChanged, this, &ThingManagerImplementation::slotThingStateValueChanged);
    connect(thing, &Thing::settingChanged, this, &ThingManagerImplementation::slotThingSettingChanged);
    connect(thing, &Thing::nameChanged, this, &ThingManagerImplementation::slotThingNameChanged);


    ThingSetupInfo *info = new ThingSetupInfo(thing, this, 30000);
    plugin->setupThing(info);

    return info;
}

void ThingManagerImplementation::postSetupThing(Thing *thing)
{
    ThingClass thingClass = findThingClass(thing->thingClassId());
    IntegrationPlugin *plugin = m_integrationPlugins.value(thingClass.pluginId());

    plugin->postSetupThing(thing);
}

void ThingManagerImplementation::loadThingStates(Thing *thing)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleThingStates);
    settings.beginGroup(thing->id().toString());
    ThingClass thingClass = m_supportedThings.value(thing->thingClassId());
    foreach (const StateType &stateType, thingClass.stateTypes()) {
        if (stateType.cached()) {
            QVariant value;
            // First try to load new style
            if (settings.childGroups().contains(stateType.id().toString())) {
                settings.beginGroup(stateType.id().toString());
                value = settings.value("value", stateType.defaultValue());
                value.convert(settings.value("type").toInt());
                settings.endGroup();
            } else { // Try to fall back to the pre 0.9.0 way of storing states
                value = settings.value(stateType.id().toString(), stateType.defaultValue());
            }
            thing->setStateValue(stateType.id(), value);
        } else {
            thing->setStateValue(stateType.id(), stateType.defaultValue());
        }
    }
    settings.endGroup();
}

void ThingManagerImplementation::storeThingStates(Thing *thing)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleThingStates);
    settings.beginGroup(thing->id().toString());
    ThingClass thingClass = m_supportedThings.value(thing->thingClassId());
    foreach (const StateType &stateType, thingClass.stateTypes()) {
        if (stateType.cached()) {
            settings.beginGroup(stateType.id().toString());
            settings.setValue("type", static_cast<int>(thing->stateValue(stateType.id()).type()));
            settings.setValue("value", thing->stateValue(stateType.id()));
            settings.endGroup();
        }
    }
    settings.endGroup();
}

