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

#ifndef INTEGRATIONSHANDLER_H
#define INTEGRATIONSHANDLER_H

#include "jsonrpc/jsonhandler.h"
#include "usermanager/userinfo.h"
#include "integrations/thingmanager.h"

namespace nymeaserver {

class IntegrationsHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit IntegrationsHandler(ThingManager *thingManager, QObject *parent = nullptr);

    QString name() const override;
    QHash<QString, QString> cacheHashes() const override;

    Q_INVOKABLE JsonReply *GetVendors(const QVariantMap &params, const JsonContext &context) const;
    Q_INVOKABLE JsonReply *GetThingClasses(const QVariantMap &params, const JsonContext &context) const;
    Q_INVOKABLE JsonReply *DiscoverThings(const QVariantMap &params, const JsonContext &context) const;
    Q_INVOKABLE JsonReply *GetPlugins(const QVariantMap &params, const JsonContext &context) const;
    Q_INVOKABLE JsonReply *GetPluginConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetPluginConfiguration(const QVariantMap &params);
    Q_INVOKABLE JsonReply *AddThing(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *PairThing(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *ConfirmPairing(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetThings(const QVariantMap &params, const JsonContext &context) const;
    Q_INVOKABLE JsonReply *ReconfigureThing(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *EditThing(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveThing(const QVariantMap &params);
    Q_INVOKABLE JsonReply *SetThingSettings(const QVariantMap &params);
    Q_INVOKABLE JsonReply *SetStateLogging(const QVariantMap &params);
    Q_INVOKABLE JsonReply *SetEventLogging(const QVariantMap &params);
    Q_INVOKABLE JsonReply *SetActionLogging(const QVariantMap &params);
    Q_INVOKABLE JsonReply *SetStateFilter(const QVariantMap &params);

    Q_INVOKABLE JsonReply *GetEventTypes(const QVariantMap &params, const JsonContext &context) const;
    Q_INVOKABLE JsonReply *GetActionTypes(const QVariantMap &params, const JsonContext &context) const;
    Q_INVOKABLE JsonReply *GetStateTypes(const QVariantMap &params, const JsonContext &context) const;

    Q_INVOKABLE JsonReply *GetStateValue(const QVariantMap &params, const JsonContext &context) const;
    Q_INVOKABLE JsonReply *GetStateValues(const QVariantMap &params, const JsonContext &context) const;

    Q_INVOKABLE JsonReply *BrowseThing(const QVariantMap &params, const JsonContext &context) const;
    Q_INVOKABLE JsonReply *GetBrowserItem(const QVariantMap &params, const JsonContext &context) const;

    Q_INVOKABLE JsonReply *ExecuteAction(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *ExecuteBrowserItem(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *ExecuteBrowserItemAction(const QVariantMap &params, const JsonContext &context);

    Q_INVOKABLE JsonReply *GetIOConnections(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *ConnectIO(const QVariantMap &params);
    Q_INVOKABLE JsonReply *DisconnectIO(const QVariantMap &params);

    static QVariantMap packBrowserItem(const BrowserItem &item);

signals:
    void PluginConfigurationChanged(const QVariantMap &params);
    // Thing permission relevant notifications
    void StateChanged(const QVariantMap &params, const ThingId &thingId);
    void ThingRemoved(const QVariantMap &params, const ThingId &thingId);
    void ThingAdded(const QVariantMap &params, const ThingId &thingId);
    void ThingChanged(const QVariantMap &params, const ThingId &thingId);
    void ThingSettingChanged(const QVariantMap &params, const ThingId &thingId);
    void EventTriggered(const QVariantMap &params, const ThingId &thingId);
    void IOConnectionAdded(const QVariantMap &params);
    void IOConnectionRemoved(const QVariantMap &params);

    // User specific notifications depending on the thing based permissions
    void ThingRemoved(const QVariantMap &params, const nymeaserver::UserInfo &userInfo);
    void ThingAdded(const QVariantMap &params, const nymeaserver::UserInfo &userInfo);

private slots:
    void pluginConfigChanged(const PluginId &id, const ParamList &config);

    void thingStateChanged(Thing *thing, const QUuid &stateTypeId, const QVariant &value, const QVariant &minValue, const QVariant &maxValue, const QVariantList &possibleValues);

    void thingRemovedNotification(const ThingId &thingId);

    void thingAddedNotification(Thing *thing);

    void thingChangedNotification(Thing *thing);

    void thingSettingChangedNotification(const ThingId &thingId, const ParamTypeId &paramTypeId, const QVariant &value);

private:
    ThingManager *m_thingManager = nullptr;
    QVariantMap statusToReply(Thing::ThingError status) const;

    QHash<QString, QString> m_cacheHashes;
};

}

#endif // INTEGRATIONSHANDLER_H
