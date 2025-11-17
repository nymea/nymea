// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
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

#ifndef INTEGRATIONPLUGIN_H
#define INTEGRATIONPLUGIN_H

#include "libnymea.h"
#include "typeutils.h"

#include "thing.h"
#include "thingdescriptor.h"
#include "pluginmetadata.h"
#include "servicedata.h"

#include "types/thingclass.h"
#include "types/event.h"
#include "types/action.h"
#include "types/vendor.h"
#include "types/param.h"
#include "types/interface.h"
#include "types/browseraction.h"
#include "types/browseritemaction.h"

#include "hardwaremanager.h"

#include "network/apikeys/apikeystorage.h"

#include "thingdiscoveryinfo.h"
#include "thingpairinginfo.h"
#include "thingsetupinfo.h"
#include "thingactioninfo.h"
#include "browseresult.h"
#include "browseritemresult.h"
#include "browseractioninfo.h"
#include "browseritemactioninfo.h"

#include <QObject>
#include <QTranslator>
#include <QPair>
#include <QSettings>
#include <QMetaType>

Q_DECLARE_LOGGING_CATEGORY(dcIntegrations);

class ThingManager;

class LIBNYMEA_EXPORT IntegrationPlugin: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QUuid id READ pluginId)
    Q_PROPERTY(QString name READ pluginName)
    Q_PROPERTY(QString displayName READ pluginDisplayName)
    Q_PROPERTY(ParamTypes paramTypes READ configurationDescription)

public:
    IntegrationPlugin(QObject *parent = nullptr);
    virtual ~IntegrationPlugin();

    PluginMetadata metadata();

    virtual void init() {}

    PluginId pluginId() const;
    QString pluginName() const;
    QString pluginDisplayName() const;
    Vendors supportedVendors() const;
    ThingClasses supportedThings() const;
    ThingClass thingClass(const ThingClassId &thingClassId) const;

    virtual void startMonitoringAutoThings();
    virtual void discoverThings(ThingDiscoveryInfo *info);

    virtual void setupThing(ThingSetupInfo *info);
    virtual void postSetupThing(Thing *thing);
    virtual void thingRemoved(Thing *thing);

    virtual void startPairing(ThingPairingInfo *info);
    virtual void confirmPairing(ThingPairingInfo *info, const QString &username, const QString &secret);

    virtual void executeAction(ThingActionInfo *info);

    virtual void browseThing(BrowseResult *result);
    virtual void browserItem(BrowserItemResult *result);
    virtual void executeBrowserItem(BrowserActionInfo *info);
    virtual void executeBrowserItemAction(BrowserItemActionInfo *info);

    virtual QList<ServiceData> serviceInformation() const;

    // Configuration
    Q_INVOKABLE ParamTypes configurationDescription() const;
    Q_INVOKABLE Thing::ThingError setConfiguration(const ParamList &configuration);
    Q_INVOKABLE ParamList configuration() const;
    Q_INVOKABLE QVariant configValue(const ParamTypeId &paramTypeId) const;
    Q_INVOKABLE Thing::ThingError setConfigValue(const ParamTypeId &paramTypeId, const QVariant &value);

    bool isBuiltIn() const;

signals:
    void emitEvent(const Event &event);
    void configValueChanged(const ParamTypeId &paramTypeId, const QVariant &value);
    void autoThingsAppeared(const ThingDescriptors &thingDescriptors);
    void autoThingDisappeared(const ThingId &thingId);

protected:
    Things myThings() const;
    HardwareManager *hardwareManager() const;
    QSettings *pluginStorage() const;
    ApiKeyStorage *apiKeyStorage() const;

    void setMetaData(const PluginMetadata &metaData);

private:
    friend class ThingManager;
    friend class ThingManagerImplementation;

    void initPlugin(ThingManager *thingManager, HardwareManager *hardwareManager, ApiKeyStorage *apiKeyStorage);

    PluginMetadata m_metaData;
    ThingManager *m_thingManager = nullptr;
    HardwareManager *m_hardwareManager = nullptr;
    QSettings *m_storage = nullptr;
    ApiKeyStorage *m_apiKeyStorage;

    ParamList m_config;
};
Q_DECLARE_INTERFACE(IntegrationPlugin, "io.nymea.IntegrationPlugin")
Q_DECLARE_METATYPE(IntegrationPlugin*)

QDebug operator<<(QDebug debug, IntegrationPlugin *plugin);

class LIBNYMEA_EXPORT IntegrationPlugins: public QList<IntegrationPlugin*>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    IntegrationPlugins();
    IntegrationPlugins(const QList<IntegrationPlugin*> &other);
    IntegrationPlugin* findById(const PluginId &id) const;
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
Q_DECLARE_METATYPE(IntegrationPlugins)

#endif // INTEGRATIONPLUGIN_H
