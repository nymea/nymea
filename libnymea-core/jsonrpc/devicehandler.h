/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include "jsonrpc/jsonhandler.h"
#include "devices/devicemanager.h"

namespace nymeaserver {

class DeviceHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit DeviceHandler(QObject *parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply *GetSupportedVendors(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetSupportedDevices(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetDiscoveredDevices(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetPlugins(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetPluginConfiguration(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetPluginConfiguration(const QVariantMap &params);

    Q_INVOKABLE JsonReply *AddConfiguredDevice(const QVariantMap &params);
    Q_INVOKABLE JsonReply *PairDevice(const QVariantMap &params);
    Q_INVOKABLE JsonReply *ConfirmPairing(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetConfiguredDevices(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *ReconfigureDevice(const QVariantMap &params);
    Q_INVOKABLE JsonReply *EditDevice(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveConfiguredDevice(const QVariantMap &params);
    Q_INVOKABLE JsonReply *SetDeviceSettings(const QVariantMap &params);

    Q_INVOKABLE JsonReply *GetEventTypes(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetActionTypes(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetStateTypes(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetStateValue(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetStateValues(const QVariantMap &params) const;

    Q_INVOKABLE JsonReply *BrowseDevice(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *GetBrowserItem(const QVariantMap &params) const;

    Q_INVOKABLE JsonReply *ExecuteAction(const QVariantMap &params);
    Q_INVOKABLE JsonReply *ExecuteBrowserItem(const QVariantMap &params);
    Q_INVOKABLE JsonReply *ExecuteBrowserItemAction(const QVariantMap &params);

    static QVariantMap packBrowserItem(const BrowserItem &item);

signals:
    void PluginConfigurationChanged(const QVariantMap &params);
    void StateChanged(const QVariantMap &params);
    void DeviceRemoved(const QVariantMap &params);
    void DeviceAdded(const QVariantMap &params);
    void DeviceChanged(const QVariantMap &params);
    void DeviceSettingChanged(const QVariantMap &params);
    void EventTriggered(const QVariantMap &params);

private slots:
    void pluginConfigChanged(const PluginId &id, const ParamList &config);

    void deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value);

    void deviceRemovedNotification(const QUuid &deviceId);

    void deviceAddedNotification(Device *device);

    void deviceChangedNotification(Device *device);

    void deviceSettingChangedNotification(const DeviceId deviceId, const ParamTypeId &paramTypeId, const QVariant &value);

private:
    QVariantMap statusToReply(Device::DeviceError status) const;
};

}

#endif // DEVICEHANDLER_H
