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
    ~DevicePluginMock() override;

    void discoverDevices(DeviceDiscoveryInfo *info) override;

    void setupDevice(DeviceSetupInfo *info) override;
    void postSetupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;

    void startMonitoringAutoDevices() override;

    void startPairing(DevicePairingInfo *info) override;
    void confirmPairing(DevicePairingInfo *info, const QString &username, const QString &secret) override;

    void browseDevice(BrowseResult *result) override;
    void browserItem(BrowserItemResult *result) override;

public slots:
    void executeAction(DeviceActionInfo *info) override;
    void executeBrowserItem(BrowserActionInfo *info) override;
    void executeBrowserItemAction(BrowserItemActionInfo *info) override;

private slots:
    void setState(const StateTypeId &stateTypeId, const QVariant &value);
    void triggerEvent(const EventTypeId &id);
    void onDisappear();
    void onReconfigureAutoDevice();
    void generateDiscoveredDevices(DeviceDiscoveryInfo *info);
    void generateDiscoveredPushButtonDevices(DeviceDiscoveryInfo *info);
    void generateDiscoveredDisplayPinDevices(DeviceDiscoveryInfo *info);

    void onPushButtonPressed();
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
    QList<QPair<Action, Device*> > m_asyncActions;

    int m_discoveredDeviceCount;
    bool m_pushbuttonPressed;

    VirtualFsNode* m_virtualFs = nullptr;
};

#endif // DEVICEPLUGINMOCK_H
