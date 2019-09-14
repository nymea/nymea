/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICEPLUGINMOCK_H
#define DEVICEPLUGINMOCK_H

#include "devices/deviceplugin.h"

#include <QProcess>

class HttpDaemon;

class DevicePluginMock : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "io.nymea.DevicePlugin" FILE "devicepluginmock.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginMock();
    ~DevicePluginMock();

    DeviceDiscoveryInfo discoverDevices(DeviceDiscoveryInfo deviceDiscoveryInfo, const ParamList &params) override;

    DeviceSetupInfo setupDevice(Device *device) override;
    void postSetupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;

    void startMonitoringAutoDevices() override;

    DevicePairingInfo pairDevice(DevicePairingInfo &devicePairingInfo) override;
    DevicePairingInfo confirmPairing(DevicePairingInfo &devicePairingInfo, const QString &username, const QString &secret) override;

    Device::BrowseResult browseDevice(Device *device, Device::BrowseResult result, const QString &itemId, const QLocale &locale) override;
    Device::BrowserItemResult browserItem(Device *device, Device::BrowserItemResult result, const QString &itemId, const QLocale &locale) override;

public slots:
    Device::DeviceError executeAction(Device *device, const Action &action) override;
    Device::DeviceError executeBrowserItem(Device *device, const BrowserAction &browserAction) override;
    Device::DeviceError executeBrowserItemAction(Device *device, const BrowserItemAction &browserItemAction) override;

private slots:
    void setState(const StateTypeId &stateTypeId, const QVariant &value);
    void triggerEvent(const EventTypeId &id);
    void onDisappear();
    void onReconfigureAutoDevice();
    void emitDevicesDiscovered(DeviceDiscoveryInfo info);
    void emitPushButtonDevicesDiscovered(DeviceDiscoveryInfo deviceDiscoveryInfo);
    void emitDisplayPinDevicesDiscovered(DeviceDiscoveryInfo deviceDiscoveryInfo);
    void emitDeviceSetupFinished();
    void emitActionExecuted();

    void onPushButtonPressed();
    void onPushButtonPairingFinished();
    void onDisplayPinPairingFinished();
    void onChildDeviceDiscovered(const DeviceId &parentId);
    void onPluginConfigChanged();

private:
    void generateBrowseItems();

private:
    class VirtualFsNode {
    public:
        VirtualFsNode(const BrowserItem &item):item(item) {}
        BrowserItem item;
        QList<VirtualFsNode*> childs;
        void addChild(VirtualFsNode* child) {childs.append(child); }
        VirtualFsNode *findNode(const QString &id) {
            if (item.id() == id) return this;
            foreach (VirtualFsNode *child, childs) {
                VirtualFsNode *node = child->findNode(id);
                if (node) return node;
            }
            return nullptr;
        }
    };

    QHash<Device*, HttpDaemon*> m_daemons;
    QList<Device*> m_asyncSetupDevices;
    QList<QPair<Action, Device*> > m_asyncActions;

    DevicePairingInfo m_pairingInfo;

    int m_discoveredDeviceCount;
    bool m_pushbuttonPressed;

    VirtualFsNode* m_virtualFs = nullptr;
};

#endif // DEVICEPLUGINMOCK_H
