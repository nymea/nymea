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

#ifndef THINGMANAGERIMPLEMENTATION_H
#define THINGMANAGERIMPLEMENTATION_H

#include "libnymea.h"

#include "integrations/thing.h"
#include "integrations/thingdescriptor.h"
#include "integrations/pluginmetadata.h"
#include "integrations/ioconnection.h"

#include "types/thingclass.h"
#include "types/interface.h"
#include "types/event.h"
#include "types/action.h"
#include "types/vendor.h"

#include <QObject>
#include <QTimer>
#include <QLocale>
#include <QPluginLoader>
#include <QTranslator>

#include "hardwaremanager.h"

#include "integrations/thingmanager.h"

class Thing;
class IntegrationPlugin;
class ThingPairingInfo;
class HardwareManager;
class Translator;

class ThingManagerImplementation: public ThingManager
{
    Q_OBJECT

    friend class IntegrationPlugin;

public:
    explicit ThingManagerImplementation(HardwareManager *hardwareManager, const QLocale &locale, QObject *parent = nullptr);
    ~ThingManagerImplementation() override;

    static QStringList pluginSearchDirs();
    static QList<QJsonObject> pluginsMetadata();
    void registerStaticPlugin(IntegrationPlugin* plugin, const PluginMetadata &metaData);

    IntegrationPlugins plugins() const override;
    IntegrationPlugin *plugin(const PluginId &pluginId) const override;
    Thing::ThingError setPluginConfig(const PluginId &pluginId, const ParamList &pluginConfig) override;

    Vendors supportedVendors() const override;
    Interfaces supportedInterfaces() const override;
    ThingClasses supportedThings(const VendorId &vendorId = VendorId()) const override;

    Things configuredThings() const override;
    Thing* findConfiguredThing(const ThingId &id) const override;
    Things findConfiguredThings(const ThingClassId &thingClassId) const override;
    Things findConfiguredThings(const QString &interface) const override;
    Things findChilds(const ThingId &id) const override;
    ThingClass findThingClass(const ThingClassId &thingClassId) const override;

    ThingDiscoveryInfo* discoverThings(const ThingClassId &thingClassId, const ParamList &params) override;

    ThingSetupInfo* addConfiguredThing(const ThingClassId &thingClassId, const ParamList &params, const QString &name = QString()) override;
    ThingSetupInfo* addConfiguredThing(const ThingDescriptorId &thingDescriptorId, const ParamList &params = ParamList(), const QString &name = QString()) override;

    ThingSetupInfo* reconfigureThing(const ThingId &thingId, const ParamList &params, const QString &name = QString()) override;
    ThingSetupInfo* reconfigureThing(const ThingDescriptorId &thingDescriptorId, const ParamList &params = ParamList(), const QString &name = QString()) override;

    ThingPairingInfo* pairThing(const ThingClassId &thingClassId, const ParamList &params, const QString &name = QString()) override;
    ThingPairingInfo* pairThing(const ThingDescriptorId &thingDescriptorId, const ParamList &params = ParamList(), const QString &name = QString()) override;
    ThingPairingInfo* pairThing(const ThingId &thingId, const ParamList &params, const QString &name = QString()) override;
    ThingPairingInfo* confirmPairing(const PairingTransactionId &pairingTransactionId, const QString &username, const QString &secret) override;

    Thing::ThingError editThing(const ThingId &thingId, const QString &name) override;
    Thing::ThingError setThingSettings(const ThingId &thingId, const ParamList &settings) override;

    Thing::ThingError removeConfiguredThing(const ThingId &thingId) override;

    ThingActionInfo* executeAction(const Action &action) override;

    BrowseResult* browseThing(const ThingId &thingId, const QString &itemId, const QLocale &locale) override;
    BrowserItemResult* browserItemDetails(const ThingId &thingId, const QString &itemId, const QLocale &locale) override;
    BrowserActionInfo *executeBrowserItem(const BrowserAction &browserAction) override;
    BrowserItemActionInfo *executeBrowserItemAction(const BrowserItemAction &browserItemAction) override;

    IOConnections ioConnections(const ThingId &thingId = ThingId()) const override;
    IOConnectionResult connectIO(const IOConnection &connection) override;
    Thing::ThingError disconnectIO(const IOConnectionId &ioConnectionId) override;

    QString translate(const PluginId &pluginId, const QString &string, const QLocale &locale) override;
    ParamType translateParamType(const PluginId &pluginId, const ParamType &paramType, const QLocale &locale) override;
    StateType translateStateType(const PluginId &pluginId, const StateType &stateType, const QLocale &locale) override;
    EventType translateEventType(const PluginId &pluginId, const EventType &eventType, const QLocale &locale) override;
    ActionType translateActionType(const PluginId &pluginId, const ActionType &actionType, const QLocale &locale) override;
    ThingClass translateThingClass(const ThingClass &thingClass, const QLocale &locale) override;
    Vendor translateVendor(const Vendor &vendor, const QLocale &locale) override;

signals:
    void loaded();

private slots:
    void loadPlugins();
    void loadPlugin(IntegrationPlugin *pluginIface, const PluginMetadata &metaData);
    void loadConfiguredThings();
    void storeConfiguredThings();
    void startMonitoringAutoThings();
    void onAutoThingsAppeared(const ThingDescriptors &thingDescriptors);
    void onAutoThingDisappeared(const ThingId &thingId);
    void onLoaded();
    void cleanupThingStateCache();
    void onEventTriggered(const Event &event);

    // Only connect this to Things. It will query the sender()
    void slotThingStateValueChanged(const StateTypeId &stateTypeId, const QVariant &value);
    void slotThingSettingChanged(const ParamTypeId &paramTypeId, const QVariant &value);
    void slotThingNameChanged();

private:
    // Builds a list of params ready to create a thing.
    // Template is thingClass.paramtypes, "first" has highest priority. If a param is not found neither in first nor in second, defaults apply.
    ParamList buildParams(const ParamTypes &types, const ParamList &first, const ParamList &second = ParamList());
    void pairThingInternal(ThingPairingInfo *info);
    ThingSetupInfo *addConfiguredThingInternal(const ThingClassId &thingClassId, const QString &name, const ParamList &params, const ThingId &parentId = ThingId());
    ThingSetupInfo *reconfigureThingInternal(Thing *thing, const ParamList &params, const QString &name = QString());
    ThingSetupInfo *setupThing(Thing *thing);
    void postSetupThing(Thing *thing);
    void trySetupThing(Thing *thing);
    void storeThingStates(Thing *thing);
    void loadThingStates(Thing *thing);
    void storeIOConnections();
    void loadIOConnections();
    void syncIOConnection(Thing *inputThing, const StateTypeId &stateTypeId);
    QVariant mapValue(const QVariant &value, const StateType &fromStateType, const StateType &toStateType, bool inverted) const;

private:
    HardwareManager *m_hardwareManager;

    QLocale m_locale;
    Translator *m_translator = nullptr;
    QHash<VendorId, Vendor> m_supportedVendors;
    QHash<QString, Interface> m_supportedInterfaces;
    QHash<VendorId, QList<ThingClassId> > m_vendorThingMap;
    QHash<ThingClassId, ThingClass> m_supportedThings;
    QHash<ThingId, Thing*> m_configuredThings;
    QHash<ThingDescriptorId, ThingDescriptor> m_discoveredThings;

    QHash<PluginId, IntegrationPlugin*> m_integrationPlugins;

    class PairingContext {
    public:
        ThingId thingId;
        ThingClassId thingClassId;
        ThingId parentId;
        ParamList params;
        QString thingName;
    };
    QHash<PairingTransactionId, PairingContext> m_pendingPairings;

    QHash<IOConnectionId, IOConnection> m_ioConnections;
};

#endif // THINGMANAGERIMPLEMENTATION_H
