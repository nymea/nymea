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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef THINGMANAGER_H
#define THINGMANAGER_H

#include <QObject>

#include "thing.h"
#include "integrationplugin.h"
#include "ioconnection.h"
#include "types/interface.h"
#include "types/vendor.h"
#include "types/browseritem.h"
#include "types/browseraction.h"
#include "types/browseritemaction.h"

class ThingManager : public QObject
{
    Q_OBJECT
public:
    explicit ThingManager(QObject *parent = nullptr);
    virtual ~ThingManager() = default;

    virtual IntegrationPlugins plugins() const = 0;
    virtual IntegrationPlugin* plugin(const PluginId &pluginId) const = 0;
    virtual Thing::ThingError setPluginConfig(const PluginId &pluginId, const ParamList &pluginConfig) = 0;

    virtual Vendors supportedVendors() const = 0;
    virtual Interfaces supportedInterfaces() const = 0;
    virtual ThingClasses supportedThings(const VendorId &vendorId = VendorId()) const = 0;

    virtual ThingClass findThingClass(const ThingClassId &thingClassId) const = 0;

    virtual Things configuredThings() const = 0;
    virtual Thing* findConfiguredThing(const ThingId &id) const = 0;
    virtual Things findConfiguredThings(const ThingClassId &thingClassId) const = 0;
    virtual Things findConfiguredThings(const QString &interface) const = 0;
    virtual Things findChilds(const ThingId &id) const = 0;

    virtual ThingDiscoveryInfo* discoverThings(const ThingClassId &thingClassId, const ParamList &params) = 0;

    virtual ThingSetupInfo* addConfiguredThing(const ThingClassId &thingClassId, const ParamList &params, const QString &name = QString()) = 0;
    virtual ThingSetupInfo* addConfiguredThing(const ThingDescriptorId &thingDescriptorId, const ParamList &params = ParamList(), const QString &name = QString()) = 0;

    virtual ThingSetupInfo* reconfigureThing(const ThingId &thingId, const ParamList &params, const QString &name = QString()) = 0;
    virtual ThingSetupInfo* reconfigureThing(const ThingDescriptorId &thingDescriptorId, const ParamList &params = ParamList(), const QString &name = QString()) = 0;

    virtual ThingPairingInfo* pairThing(const ThingClassId &thingClassId, const ParamList &params, const QString &name = QString()) = 0;
    virtual ThingPairingInfo* pairThing(const ThingDescriptorId &thingDescriptorId, const ParamList &params = ParamList(), const QString &name = QString()) = 0;
    virtual ThingPairingInfo* pairThing(const ThingId &thingId, const ParamList &params, const QString &name = QString()) = 0;
    virtual ThingPairingInfo* confirmPairing(const PairingTransactionId &pairingTransactionId, const QString &username = QString(), const QString &secret = QString()) = 0;

    virtual Thing::ThingError editThing(const ThingId &thingId, const QString &name) = 0;
    virtual Thing::ThingError setThingSettings(const ThingId &thingId, const ParamList &settings) = 0;

    virtual Thing::ThingError setStateLogging(const ThingId &thingId, const StateTypeId &stateTypeId, bool enabled) = 0;
    virtual Thing::ThingError setEventLogging(const ThingId &thingId, const EventTypeId &eventTypeId, bool enabled) = 0;
    virtual Thing::ThingError setActionLogging(const ThingId &thingId, const ActionTypeId &actionTypeId, bool enabled) = 0;
    virtual Thing::ThingError setStateFilter(const ThingId &thingId, const StateTypeId &stateTypeId, Types::StateValueFilter filter) = 0;

    virtual Thing::ThingError removeConfiguredThing(const ThingId &thingId) = 0;

    virtual ThingActionInfo* executeAction(const Action &action) = 0;

    virtual BrowseResult* browseThing(const ThingId &thingId, const QString &itemId, const QLocale &locale) = 0;
    virtual BrowserItemResult* browserItemDetails(const ThingId &thingId, const QString &itemId, const QLocale &locale) = 0;
    virtual BrowserActionInfo* executeBrowserItem(const BrowserAction &browserAction) = 0;
    virtual BrowserItemActionInfo* executeBrowserItemAction(const BrowserItemAction &browserItemAction) = 0;

    virtual IOConnections ioConnections(const ThingId &thingId = ThingId()) const = 0;
    IOConnectionResult connectIO(const ThingId &inputThing, const StateTypeId &inputState, const ThingId &outputThing, const StateTypeId &outputState, bool inverted = false);
    virtual Thing::ThingError disconnectIO(const IOConnectionId &ioConnectionId) = 0;

    virtual QString translate(const PluginId &pluginId, const QString &string, const QLocale &locale) = 0;
    virtual ParamType translateParamType(const PluginId &pluginId, const ParamType &paramType, const QLocale &locale) = 0;
    virtual StateType translateStateType(const PluginId &pluginId, const StateType &stateType, const QLocale &locale) = 0;
    virtual EventType translateEventType(const PluginId &pluginId, const EventType &eventType, const QLocale &locale) = 0;
    virtual ActionType translateActionType(const PluginId &pluginId, const ActionType &actionType, const QLocale &locale) = 0;
    virtual ThingClass translateThingClass(const ThingClass &thingClass, const QLocale &locale) = 0;
    virtual Vendor translateVendor(const Vendor &vendor, const QLocale &locale) = 0;

protected:
    virtual IOConnectionResult connectIO(const IOConnection &connection) = 0;

signals:
    void loaded();
    void pluginConfigChanged(const PluginId &id, const ParamList &config);
    void eventTriggered(const Event &event);
    void thingStateChanged(Thing *thing, const StateTypeId &stateTypeId, const QVariant &value, const QVariant &minValue, const QVariant &maxValue);
    void thingRemoved(const ThingId &thingId);
    void thingAdded(Thing *thing);
    void thingChanged(Thing *thing);
    void thingSettingChanged(const ThingId &thingId, const ParamTypeId &settingParamTypeId, const QVariant &value);
    void ioConnectionAdded(const IOConnection &ioConnection);
    void ioConnectionRemoved(const IOConnectionId &ioConnectionId);
    void actionExecuted(const Action &action, Thing::ThingError status);
};

#endif // THINGMANAGER_H
